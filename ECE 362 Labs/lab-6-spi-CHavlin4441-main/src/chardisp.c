#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "chardisp.h"

// Make sure to set these in main.c
extern const int SPI_DISP_SCK; extern const int SPI_DISP_CSn; extern const int SPI_DISP_TX;

/***************************************************************** */

// "chardisp" stands for character display, which can be an LCD or OLED
void init_chardisp_pins() 
{
    //initialize SPI pins for character display
    spi_init(spi0, 10000); //initialize SPI pins for character display
    
    gpio_set_function(SPI_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_TX, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_CSn, GPIO_FUNC_SPI);

    spi_set_format(spi0, 9, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); //configure SPI format

}

void send_spi_cmd(spi_inst_t* spi, uint16_t value) 
{
    gpio_put(SPI_DISP_CSn, 0); //pull CS low to start transmission

    spi_write16_blocking(spi, &value, 1); //send the value over SPI

    gpio_put(SPI_DISP_CSn, 1); //pull CS high to end transmission
}

void send_spi_data(spi_inst_t* spi, uint16_t value) 
{
    //set the command bit to 0 to indicate this is data, and send the value over SPI
    gpio_put(SPI_DISP_CSn, 0); //pull CS low to start transmission

    uint16_t val = value | 0x0100; //mask the value to ensure the command bit is 0
    send_spi_cmd(spi, val); //send the value over SPI using the send_spi_cmd function

    gpio_put(SPI_DISP_CSn, 1); //pull CS high to end transmission
}

void cd_init() 
{
    sleep_ms(1);

    //initialize the character display using a sequence of SPI commands
    send_spi_cmd(spi0, 0x0038);
    sleep_us(40);

    //diplay on, cursor off, blink off
    send_spi_cmd(spi0, 0x000C);
    sleep_us(40);

    //clear display
    send_spi_cmd(spi0, 0x0001);
    sleep_ms(2);

    //increment cursor, no display shift
    send_spi_cmd(spi0, 0x0006);
    sleep_us(40);
}

void cd_display1(const char *str) 
{
    send_spi_cmd(spi0, 0x0080); //set DDRAM address to 0

    for(int i = 0; i < 16 && str[i] != '\0'; i++) 
    {
        send_spi_data(spi0, str[i]); //send each character of the string as data over SPI
    }

}
void cd_display2(const char *str) 
{
    send_spi_cmd(spi0, 0x00C0); //set DDRAM address to 0x40 (start of second line)

    for(int i = 0; i < 16 && str[i] != '\0'; i++) 
    {
        send_spi_data(spi0, str[i]); //send each character of the string as data over SPI
    }
}

/***************************************************************** */