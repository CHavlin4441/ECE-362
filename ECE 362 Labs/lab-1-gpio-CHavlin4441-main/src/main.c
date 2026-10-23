#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/structs/io_bank0.h"
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"

//////////////////////////////////////////////////////////////////////////////

const char* username = "havlinc";

//////////////////////////////////////////////////////////////////////////////

void autotest();

void init_outputs() 
{
    // fill in
    for(int pin = 22; pin <= 25; pin++)
    {
        io_bank0_hw->io[pin].ctrl = GPIO_FUNC_SIO;
        sio_hw->gpio_clr = (1 << pin);
        sio_hw->gpio_oe_set = (1 << pin);
    }
}

void init_inputs() 
{

    // fill in
    sio_hw->gpio_oe_clr = (1 << 21);
    sio_hw->gpio_oe_clr = (1 << 26);

    hw_write_masked(&pads_bank0_hw->io[21],
                   PADS_BANK0_GPIO0_IE_BITS,
                   PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
    );

    hw_write_masked(&pads_bank0_hw->io[26],
                   PADS_BANK0_GPIO0_IE_BITS,
                   PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
    );

    io_bank0_hw->io[21].ctrl = 5 << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    io_bank0_hw->io[26].ctrl = 5 << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    
    hw_clear_bits(&pads_bank0_hw->io[21], PADS_BANK0_GPIO0_ISO_BITS);
    hw_clear_bits(&pads_bank0_hw->io[26], PADS_BANK0_GPIO0_ISO_BITS);

}

void init_keypad() 
{
    // fill in
    const int row[] = {2, 3, 4, 5}; 
    const int col[] = {6, 7, 8, 9};

    for(int i = 0; i < 4; i++)
    {
        io_bank0_hw->io[col[i]].ctrl = GPIO_FUNC_SIO;
        sio_hw->gpio_oe_set = (1 << col[i]);
        sio_hw->gpio_clr = (1 << col[i]);
        
        hw_clear_bits(&pads_bank0_hw->io[col[i]], PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS);

        io_bank0_hw->io[row[i]].ctrl = GPIO_FUNC_SIO;
        sio_hw->gpio_oe_clr = (1 << row[i]);

        hw_set_bits(&pads_bank0_hw->io[row[i]], PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_PDE_BITS);
        hw_clear_bits(&pads_bank0_hw->io[row[i]], PADS_BANK0_GPIO0_OD_BITS | PADS_BANK0_GPIO0_PUE_BITS);
        
        hw_clear_bits(&pads_bank0_hw->io[row[i]], PADS_BANK0_GPIO0_ISO_BITS);
        hw_clear_bits(&pads_bank0_hw->io[col[i]], PADS_BANK0_GPIO0_ISO_BITS);
    }

}

int main() {
    // Configures our microcontroller to 
    // communicate over UART through the TX/RX pins
    stdio_init_all();

    // Leave commented until you actually need it.
    // Can take significantly longer to upload code when uncommented.
    // autotest();
    
    ////////////////////////////////////
    // All your code goes below.
    gpio_init_mask(0xf << 22);
    gpio_set_dir_out_masked64(0xf << 22);

    init_outputs();
    init_inputs();
    init_keypad();
    
    //testing step 2 - blink leds on pins 22-25
    // while(1)
    // {
    
    //     for(int pin = 22; pin <= 25; pin++)
    //     {
    //         sio_hw->gpio_set = (1u << pin);  // turn LED on
    //         sleep_ms(500);
            
    //     }

    //     for(int pin = 22; pin <= 25; pin++)
    //     {
    //         sio_hw->gpio_clr = (1u << pin);  // turn LED off
    //         sleep_ms(500);
    //     }

    // }
    
    //testing step 3 - read inputs on pins 21 and 26       
    // while (1) 
    // {
    //     uint32_t inputs = sio_hw->gpio_in;

    //     if((inputs & (1u << 21))) 
    //     {
    //         sio_hw->gpio_set = (0xf << 22);  // Turn on all LEDs
    
    //     } 

    //     else if((inputs & (1u << 26))) 
    //     {
    //         sio_hw->gpio_clr = (0xf << 22);  // Turn off all LEDs
    //     }

    //     sleep_ms(10);
    // }

    //testing step 4 - keypad scanning
    const uint COLS[] = {6, 7, 8, 9};
    const uint ROWS[] = {2, 3, 4, 5};

    while (1)
    {

        for(int i = 0; i < 4; i++)
        {
            sio_hw->gpio_set = (1 << COLS[i]);
            
            sleep_ms(10);   
            

            int pin = (sio_hw->gpio_in >> ROWS[i]) & 1;
            int led = 25 - i;

            if(pin == 0)
            {
                sio_hw->gpio_clr = (1 << led);
            }  

            else
            {
                sio_hw->gpio_set = (1 << led); 
            }

            sio_hw->gpio_clr = (1 << COLS[i]);
        }

    }

    // All your code goes above.
    ////////////////////////////////////

    // An infinite loop is necessary to 
    // ensure control flow remains with user.
    for(;;);

    // Never reached.
    return 0;
}