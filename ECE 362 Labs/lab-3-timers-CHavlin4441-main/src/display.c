#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

// 7-segment display message buffer
// Declared as static to limit scope to this file only.
static char msg[8] = {
    0x3F, // seven-segment value of 0
    0x06, // seven-segment value of 1
    0x5B, // seven-segment value of 2
    0x4F, // seven-segment value of 3
    0x66, // seven-segment value of 4
    0x6D, // seven-segment value of 5
    0x7D, // seven-segment value of 6
    0x07, // seven-segment value of 7
};

extern char font[]; // Font mapping for 7-segment display
static int index = 0; // Current index in the message buffer

// We provide you with this function for directly displaying characters.
// However, it can't use the decimal point, which display_print does.
void display_char_print(const char message[]) 
{
    for (int i = 0; i < 8; i++) {
        msg[i] = font[message[i] & 0xFF];
    }
}

/********************************************************* */
// Implement the functions below.


void display_init_pins() 
{
    for (int pin = 10; pin <= 17; pin++)
    {
        io_bank0_hw->io[pin].ctrl = GPIO_FUNC_SIO;

        hw_clear_bits(&pads_bank0_hw->io[pin], PADS_BANK0_GPIO0_IE_BITS);
        hw_clear_bits(&pads_bank0_hw->io[pin], PADS_BANK0_GPIO0_PUE_BITS | PADS_BANK0_GPIO0_PDE_BITS);
        
        sio_hw->gpio_clr = (1 << pin);
        sio_hw->gpio_oe_set = (1 << pin);

        hw_clear_bits(&pads_bank0_hw->io[pin], PADS_BANK0_GPIO0_ISO_BITS);
    }
    
    for (int pin = 18; pin <= 20; pin++)
    {
        io_bank0_hw->io[pin].ctrl = GPIO_FUNC_SIO;
        
        hw_clear_bits(&pads_bank0_hw->io[pin], PADS_BANK0_GPIO0_IE_BITS);
        hw_clear_bits(&pads_bank0_hw->io[pin], PADS_BANK0_GPIO0_PUE_BITS | PADS_BANK0_GPIO0_PDE_BITS);
        
        sio_hw->gpio_clr = (1 << pin);
        sio_hw->gpio_oe_set = (1 << pin);

        hw_clear_bits(&pads_bank0_hw->io[pin], PADS_BANK0_GPIO0_ISO_BITS);
    }
    
}

void display_isr() 
{
    hw_clear_bits(&timer1_hw->intr, 1 << 0);

    sio_hw->gpio_clr = 0x7FF << 10;
    sio_hw->gpio_set = (index) << 18 | (msg[index] << 10);
    
    index = (index + 1) % 8;
    
    timer1_hw->alarm[0] = (uint32_t)(time_us_64() + 3000);
}

void display_init_timer() 
{
    hw_set_bits(&timer1_hw->inte, 1 << 0);

    irq_set_exclusive_handler(TIMER1_IRQ_0, display_isr);
    irq_set_enabled(TIMER1_IRQ_0, true);

    uint64_t now = time_us_64();
    timer1_hw->alarm[0] = (uint32_t)(now + 3000); 
}

void display_print(const uint16_t message[]) 
{

    for(int i = 0; i < 8; i++) 
    {
        uint16_t event = message[i];

        msg[i] = font[message[i] & 0xFF];

        if(event & 0x0100)
        {
            msg[i] |= 0x80;
        }

    }

}
