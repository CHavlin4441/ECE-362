#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/xosc.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "hardware/structs/io_bank0.h"
#include "pico/stdlib.h"

void autotest();
const char keymap[16] = "DCBA#9630852*741";
char key = '\0';
int col = 0;

//////////////////////////////////////////////////////////////////////////////

const char* username = "havlinc";

//////////////////////////////////////////////////////////////////////////////

// Only uncomment ONE step at a time.

// When testing init_gpio_irq
// #define STEP1
// When testing init_keypad_irq
// #define STEP2
// When testing init_fifo_irq
#define STEP3

/****************************************************************/
// Copy in the three functions from lab 1 below.

void init_outputs() 
{
    gpio_init_mask(0xf << 22);
    gpio_set_dir_out_masked64(0xf << 22);
    
    for(int pin = 22; pin <= 25; pin++) 
    {
        io_bank0_hw->io[pin].ctrl = GPIO_FUNC_SIO;
        sio_hw->gpio_clr = (1 << pin);
        sio_hw->gpio_oe_set = (1 << pin);
    }

}

void init_inputs() 
{
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

/****************************************************************/
// Lab 2 functions

void gpio_isr() 
{
    uint32_t event21 = gpio_get_irq_event_mask(21);
    uint32_t event26 = gpio_get_irq_event_mask(26);

    if(event21 & GPIO_IRQ_EDGE_RISE)
    {
        io_bank0_hw->intr[21 / 8] = GPIO_IRQ_EDGE_RISE << (4 * (21 % 8));

        for(int pin = 22; pin <= 25; pin++)
        {
            gpio_put(pin, 0);
        }

        xosc_dormant();
    }

    if(event26 & GPIO_IRQ_EDGE_RISE)
    {
        io_bank0_hw->intr[26 / 8] = GPIO_IRQ_EDGE_RISE << (4 * (26 % 8));

        for(int pin = 22; pin <= 25; pin++)
        {
            gpio_put(pin, 1);
        }

    }

}

void init_gpio_irq() 
{
    for(int pin = 22; pin <= 25; pin++)
    {
        gpio_put(pin, 1);
    }

    uint32_t mask = (1 << 21) | (1 << 26);
    gpio_add_raw_irq_handler_masked(mask, gpio_isr);
    
    io_bank0_irq_ctrl_hw_t *irq_ctrl = (get_core_num() == 0) ?
        &io_bank0_hw->proc0_irq_ctrl : &io_bank0_hw->proc1_irq_ctrl;
    
    hw_set_bits(&irq_ctrl->inte[21 / 8],
            GPIO_IRQ_EDGE_RISE << (4 * (21 % 8)));

    hw_set_bits(&irq_ctrl->inte[26 / 8],
            GPIO_IRQ_EDGE_RISE << (4 * (26 % 8)));

    hw_set_bits(
        &io_bank0_hw->dormant_wake_irq_ctrl.inte[26 / 8],
        GPIO_IRQ_EDGE_RISE << (4 * (26 % 8))
    );

    irq_set_enabled(IO_IRQ_BANK0, true);
}

void drive_column() 
{
    for(int i = 6; i <= 9; i++)
    {
        gpio_put(i, 0);
    }

    gpio_put(6 + col, 1);
    sleep_ms(25);

    col = (col + 1) % 4;
}

void keypad_isr() 
{
    for(int row = 0; row < 4; row++)
    {
        uint gpio = 2 + row;
        uint32_t event = gpio_get_irq_event_mask(gpio);

        if(event & GPIO_IRQ_EDGE_RISE)
        {
            io_bank0_hw->intr[gpio / 8] = GPIO_IRQ_EDGE_RISE << (4 * (gpio % 8));

            int index = col * 4 + row;
            key = keymap[index];

            #ifdef STEP3
                multicore_fifo_push_blocking((uint32_t)key);
            #else
                printf("Pressed: %c\n", key);
            #endif
        }

    }

}

void init_keypad_irq() 
{
    io_bank0_hw->intr[0] = 0xFu << (2 * 4) | 0xFu << (3 * 4) | 0xFu << (4 * 4) | 0xFu << (5 * 4);

    uint32_t mask_row = (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
    gpio_add_raw_irq_handler_masked(mask_row, keypad_isr);

    io_bank0_irq_ctrl_hw_t *irq_ctrl = (get_core_num() == 0) ?
        &io_bank0_hw->proc0_irq_ctrl : &io_bank0_hw->proc1_irq_ctrl;

    for(int pin = 2; pin <= 5; pin++)
    {
        uint32_t shift = 4 * (pin % 8);
        hw_set_bits(&irq_ctrl->inte[pin / 8],
                    GPIO_IRQ_EDGE_RISE << shift);
    }

    irq_set_enabled(IO_IRQ_BANK0, true);
}

void init_fifo_irq() 
{
    init_keypad();
    init_keypad_irq();

    while(true)
    {
        drive_column();
    }

}

/****************************************************************/
// Main

int main() {
    // Configures our microcontroller to 
    // communicate over UART through the TX/RX pins
    stdio_init_all();

    // Uncomment when you need to run autotest.
    // Keep this commented out until you need it
    // since it adds a lot of time to the upload process.
    autotest();

    ///////////////////////
    // From lab 1

    init_inputs();
    init_outputs();
    init_keypad();

    ///////////////////////
    // For lab 2
    
    #ifdef STEP1
        init_gpio_irq();
        for(;;) 
        {
            printf("Hello world\n");
            sleep_ms(1000);
        }
    #endif
    #ifdef STEP2
        init_keypad_irq();
        for (;;) 
        {
            drive_column();
        }
    #endif
    #ifdef STEP3
        // TODO: launch the function init_fifo_irq on core 1.
        multicore_reset_core1();
        multicore_launch_core1(init_fifo_irq);
        
        for(;;) 
        {
            // TODO: pop a value from the FIFO and store it in global variable `key`.
            // TODO: print the value of `key` to the console.
            uint32_t received = multicore_fifo_pop_blocking();
            key = (char) received;
            printf("Key received: %c\n", key);
        }
    #endif
    
    // Never reached.
    for(;;);
    return 0;
}
