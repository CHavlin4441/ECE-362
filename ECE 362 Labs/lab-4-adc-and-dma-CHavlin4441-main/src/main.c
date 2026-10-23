#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"


//////////////////////////////////////////////////////////////////////////////

const char* username = "havlinc";

//////////////////////////////////////////////////////////////////////////////

uint32_t adc_fifo_out = 0;
void display_init_pins();
void display_init_timer();
void display_char_print(const char* buffer);
void autotest();

//////////////////////////////////////////////////////////////////////////////

// Only uncomment ONE step at a time.

// When testing manual ADC single-shot conversion
#define STEP2
// When testing manual ADC free-running conversion
// #define STEP3
// When testing automated ADC sampling with DMA
// #define STEP4

//////////////////////////////////////////////////////////////////////////////

// If ADC/DMA SDK functions and macros aren't being highlighted by VScode 
// and you're not sure why, make sure to read the tip in step 2!

void init_adc() 
{
    //enable ADC CLK and RST
    adc_init(); //sets CLK and brings ADC out of reset, so we can use it
    adc_gpio_init(45); //sets up GPIO 45 for ADC use 
    adc_select_input(5); //select ADC input 5, which is GPIO 45
    
}

uint16_t read_adc() 
{
    //sets the ADC_CS_START_ONCE bit to begin a single conversation
    return adc_read(); //waits for conversion to complete and returns the result
    
}

void init_adc_freerun() 
{
    //enable ADC CLK and RST
    adc_init(); //sets CLK and brings ADC out of reset, so we can use it
    adc_gpio_init(45); //sets up GPIO 45 for ADC use 
    adc_select_input(5); //select ADC input 5, which is GPIO 45

    adc_run(true); //sets the ADC_CS_START_MANY bit to begin free-running conversion
}

void init_dma() 
{
    //write the necessary code to set up the DMA channel to transfer data from the
    //ADC result register to the variable adc_fifo_out whenever a new sample is ready in the ADC FIFO
    dma_hw->ch[0].read_addr = (uintptr_t) &adc_hw->fifo; //set the read address to the ADC result register
    dma_hw->ch[0].write_addr = (uintptr_t) &adc_fifo_out; //set the write address to the variable adc_fifo_out

    dma_hw->ch[0].transfer_count = (1 << DMA_CH0_TRANS_COUNT_MODE_LSB) | 1; //set the transfer count to 1 and enable the
                                                                            //transfer count mode bit to allow for infinite transfers
    
    uint32_t temp = 0;

    temp |= (1 << DMA_CH0_CTRL_TRIG_DATA_SIZE_LSB); //specify 32 bit transfers data size
    
    temp |= (DREQ_ADC << DMA_CH0_CTRL_TRIG_TREQ_SEL_LSB); //set the transfer request to be triggered by the ADC

    temp |= DMA_CH0_CTRL_TRIG_EN_BITS; //set the EN bit

    dma_hw->ch[0].ctrl_trig = temp; //write the control register for channel 0
    
}

void init_adc_dma() 
{
    //add necessary initialization code from previous steps
    init_dma();
    init_adc_freerun();

    adc_fifo_setup(true, true, 1, false, false); //set up the ADC FIFO 

}

int main()
{
    // Configures our microcontroller to 
    // communicate over UART through the TX/RX pins
    stdio_init_all();

    // Uncomment when you need to run autotest.
    // Keep this commented out until you need it
    // since it adds a lot of time to the upload process.
    // autotest();

    // Step 2 - singleshot
    #ifdef STEP2
    init_adc();
    for(;;) {
        printf("ADC Result: %d     \r", read_adc());
        // We've found that when we do NOT send a newline character,
        // the output is not flushed immediately, which can cause
        // the output to be delayed or not appear at all in some cases.
        // The fflush function forces a flush.
        fflush(stdout);
        sleep_ms(250);
    }
    #endif

    // Step 3 - freerun
    #ifdef STEP3
    init_adc_freerun();

    int i = 0;
    for(;;) {
        printf("ADC Result: %d     \r", adc_hw->result);
        fflush(stdout);
        sleep_ms(250);
    }
    #endif

    // Step 4 - adc_dma
    #ifdef STEP4

    // Don't forget to copy in display.c from lab 3.
    display_init_pins();
    display_init_timer();

    init_adc_dma();
    char buffer[10];
    for(;;) {
        float f = (adc_fifo_out * 3.3) / 4095.0;
        snprintf(buffer, sizeof(buffer), "%1.7f", f);
        display_char_print(buffer);

        // If you need to debug without the display, 
        // you can uncomment the following lines.

        // printf("ADC Result: %s     \r", buffer);
        // fflush(stdout);
        
        sleep_ms(250);
    }
    #endif

    for(;;);
    return 0;
}

