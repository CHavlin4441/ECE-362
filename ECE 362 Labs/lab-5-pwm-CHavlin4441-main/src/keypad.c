#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>
#include "queue.h"

// Global column variable
int col = -1;

// Global key state
static bool state[16]; // Are keys pressed/released

// Keymap for the keypad
const char keymap[17] = "DCBA#9630852*741";

void keypad_drive_column();
void keypad_isr();

/********************************************************* */
// Implement the functions below.

void keypad_init_pins() 
{
    for(int i = 2; i <= 5; i++)
    {
        io_bank0_hw->io[i].ctrl = GPIO_FUNC_SIO;
        hw_set_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_PDE_BITS);
        hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_PUE_BITS | PADS_BANK0_GPIO0_OD_BITS);
        sio_hw->gpio_oe_clr = (1 << i);
        hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);     
    }
    
    for(int i = 6; i <= 9; i++)
    {
        io_bank0_hw->io[i].ctrl = GPIO_FUNC_SIO;
        hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_PUE_BITS | PADS_BANK0_GPIO0_PDE_BITS);
        sio_hw->gpio_oe_set = (1 << i);
        hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);     
    }

    sio_hw->gpio_clr = (15 << 6);
}

void keypad_init_timer() 
{
    hw_set_bits(&timer0_hw->inte, (1 << 0) | (1 << 1));

    irq_set_exclusive_handler(TIMER0_IRQ_0, keypad_drive_column);
    irq_set_enabled(TIMER0_IRQ_0, true);

    irq_set_exclusive_handler(TIMER0_IRQ_1, keypad_isr);
    irq_set_enabled(TIMER0_IRQ_1, true);

    uint64_t now = time_us_64();
    timer0_hw->alarm[0] = (uint32_t)(now + 50000);
    timer0_hw->alarm[1] = (uint32_t)(now + 80000);
}

void keypad_drive_column() 
{
    hw_clear_bits(&timer0_hw->intr, (1 << 0));
    sio_hw->gpio_clr = (15 << 6); 

    col = (col + 1) & 3;  
    
    sio_hw->gpio_set = (1 << (6 + col));

    uint64_t targ = time_us_64() + 100000;
    timer0_hw->alarm[0] = (uint32_t)targ;
}

uint8_t keypad_read_rows() 
{
    uint32_t raw = sio_hw->gpio_in;
    
    return (uint8_t)((raw >> 2) & 0xF); 
}

void keypad_isr() 
{
    hw_clear_bits(&timer0_hw->intr, (1 << 1));

    int curr_col = col;
    uint8_t rows = keypad_read_rows();
    
    for(int row = 0; row < 4; row++)
    {
        int idx = curr_col * 4 + row;
        bool now_pressed = (rows & (1 << row)) != 0;

        if(now_pressed && !state[idx])
        {
            state[idx] = true;

            char key = keymap[idx];

            uint16_t event = ((uint16_t)key) | 0x0100;

            key_push(event);
        }

        if(!now_pressed && state[idx])
        {
            state[idx] = false;

            char key = keymap[idx];

            uint16_t event = ((uint16_t)key);

            key_push(event);
            
        }

    }
   
    timer0_hw->alarm[1] = (uint32_t)(time_us_64() + 30000);
}
