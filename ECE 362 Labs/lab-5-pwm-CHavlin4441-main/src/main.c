#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "queue.h"
#include "support.h"
#include "hardware/irq.h"

//////////////////////////////////////////////////////////////////////////////

const char* username = "havlinc";

//////////////////////////////////////////////////////////////////////////////

static int duty_cycle = 0;
static int dir = 0;
static int color = 0;


void display_init_pins();
void display_init_timer();
void display_char_print(const char message[]);
void keypad_init_pins();
void keypad_init_timer();
void init_wavetable(void);
void set_freq(int chan, float f);
extern KeyEvents kev;
void drum_machine();

//////////////////////////////////////////////////////////////////////////////

// When testing static duty-cycle PWM
// #define STEP2
// When testing variable duty-cycle PWM
// #define STEP3
// When testing 8-bit audio synthesis
#define STEP4
// When trying out drum machine
// #define DRUM_MACHINE

//////////////////////////////////////////////////////////////////////////////

void init_pwm_static(uint32_t period, uint32_t duty_cycle) 
{
    uint pins[3] = {37, 38, 39};
    bool slice_config[8] = {false};

    if(period ==0) //void divide by zero in breathing handler when period is used as a divisor
    {
        period = 1;
    }

    if(duty_cycle > period) //cap duty cycle to period to avoid wraparound in breathing handler when duty_cycle is used as a divisor
    {
        duty_cycle = period;
    }

    for(int i = 0; i < 3; i++)
    {
        uint pin = pins[i];

        gpio_set_function(pin, GPIO_FUNC_PWM); //GPIO_FUNC_PWM = 4

        //set up the slice for this pin, if we haven't already for a previous pin sharing the same slice
        uint slice = pwm_gpio_to_slice_num(pin);
        uint channel = pwm_gpio_to_channel(pin);

        //if the slice for this pin hasn't been configured yet, 
        //set it up with the desired period and a default clock divider. 
        if(!slice_config[slice])
        {
            pwm_set_clkdiv(slice, 150);
            pwm_set_wrap(slice, period -1);
            slice_config[slice] = true;
        }

        pwm_set_chan_level(slice, channel, duty_cycle);

        if(channel == PWM_CHAN_A)
        {
            pwm_set_output_polarity(slice, true, false); //set channel A to active high, channel B to active low
        }
        else
        {
            pwm_set_output_polarity(slice, false, true); //set channel A to active low, channel B to active high
        }

        pwm_set_enabled(slice, true);
    }
    
}

void pwm_breathing() 
{
    uint slice_num = pwm_gpio_to_slice_num(37); 
    pwm_clear_irq(slice_num); //acknowledge the interrupt right away to avoid missing interrupts while we are in the handler

    uint32_t top = pwm_hw->slice[slice_num].top; //get the period of the PWM signal

    //handle direction and color changes at the limits
    if(dir == 0 && duty_cycle == 100)
    {
        dir = 1;
        color = (color + 1) % 3;
    }
    else if(dir == 1 && duty_cycle == 0)
    {
        dir = 0;
    }

    //update duty cycle
    if(dir == 0)
    {
        duty_cycle++;
    }
    else
    {
        duty_cycle--;
    }

    //convert % to PWM level
    uint32_t level = ((uint32_t)duty_cycle * top) / 100;

    //apply new duty cycle to the correct selected color channel
    pwm_set_gpio_level(37, (color == 0) ? level : 0);
    pwm_set_gpio_level(38, (color == 1) ? level : 0);
    pwm_set_gpio_level(39, (color == 2) ? level : 0);
}

void init_pwm_irq() 
{
    //set up the PWM slice and channels for the breathing handler
    uint slice_37 = pwm_gpio_to_slice_num(37);

    //clear any pending interrupts and enable the interrupt for the slice we are using for breathing
    pwm_set_irq_enabled(slice_37, true);

    //set the handler for the PWM interrupt to be our breathing handler, and enable the interrupt
    irq_set_exclusive_handler(PWM_IRQ_WRAP_0, pwm_breathing);
    irq_set_enabled(PWM_IRQ_WRAP_0, true);

    pwm_clear_irq(slice_37); //acknowledge any pending interrupts before we start

    //initialize breathing pattern variables
    duty_cycle = 100;
    dir = 1;
    color = 0;

    uint32_t top = pwm_hw->slice[slice_37].top; 

    //start with red at 100% duty cycle, green and blue off
    pwm_set_gpio_level(37, top);
    pwm_set_gpio_level(38, 0);
    pwm_set_gpio_level(39, 0);
    
}

void pwm_audio_handler() 
{
    const uint gpio = 36;
    uint slice_num = pwm_gpio_to_slice_num(gpio);

    pwm_clear_irq(slice_num); //acknowledge the interrupt right away to avoid missing interrupts while we are in the handler

    uint32_t top = pwm_hw->slice[slice_num].top; //get the period of the PWM signal

    offset0 += step0;
    offset1 += step1;

    uint32_t limit = (N << 16); // N is a constant defined elsewhere in the code

    //wrap around the offsets if they exceed the wavetable length to create a repeating waveform
    if(offset0 >= limit)
    {
        offset0 -= limit;
    }
    if(offset1 >= limit)
    {
        offset1 -= limit;
    }

    uint16_t sample0 = wavetable[offset0 >> 16]; //get the current sample for channel 0
    uint16_t sample1 = wavetable[offset1 >> 16]; //get the current sample for channel 1
    uint32_t samp = (sample0 + sample1) / 2; //mix the two channels together, simple average

    uint32_t level = (samp * top) / (1 << 16); //convert the 15-bit mixed sample to a PWM level based on the current top value
    
    pwm_set_gpio_level(gpio, level);
}

void init_pwm_audio() 
{
    const uint gpio = 36;

    gpio_set_function(gpio, GPIO_FUNC_PWM); //GPIO_FUNC_PWM = 4

    uint slice = pwm_gpio_to_slice_num(gpio);

    pwm_set_clkdiv(slice, 150);

    uint32_t top = (1000000 / RATE);

    //set the period for the audio PWM signal based on the desired sample rate
    pwm_set_wrap(slice, top - 1);

    //start with 0% duty cycle so we don't get any pops until the handler starts updating the duty cycle based on the audio samples
    pwm_set_gpio_level(gpio, 0);

    //initialize the wavetable with the desired waveform samples
    init_wavetable();

    //clear any pending interrupts and enable the interrupt for the slice we are using for audio
    pwm_set_irq_enabled(slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP_0, pwm_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP_0, true);

    pwm_clear_irq(slice); //acknowledge any pending interrupts before we start

    pwm_set_enabled(slice, true);
}

//////////////////////////////////////////////////////////////////////////////

int main()
{
    // Configures our microcontroller to 
    // communicate over UART through the TX/RX pins
    stdio_init_all();

    // Uncomment when you need to run autotest.
    // Keep this commented out until you need it
    // since it adds a lot of time to the upload process.
    // autotest();

    // Make sure to copy in the latest display.c and keypad.c from your previous labs.
    keypad_init_pins();
    keypad_init_timer();
    display_init_pins();
    display_init_timer();

    /*
    *******************************************************
    * Make sure to go through the code in the steps below.  
    * A lot of it can be very useful for your projects.
    *******************************************************
    */

    #ifdef STEP2
    init_pwm_static(100, 50); // Start out with 500/1000, 50%
    display_char_print("      50");
    uint16_t percent = 50; // Set initial percentage for duty cycle, displayed 
    uint16_t disp_buffer = 0;
    char buf[9];

    // Display initial duty cycle
    snprintf(buf, sizeof(buf), "      50");
    display_char_print(buf);

    bool new_entry = true;  // Flag to track if we're starting a new entry
    
    for (;;) {
        uint16_t keyevent = key_pop(); // Pop a key event from the queue
        if (keyevent & 0x100) {
            char key = keyevent & 0xFF;
            if (key >= '0' && key <= '9') {
                // If the key is a digit, check if we need to clear the buffer first
                if (new_entry) {
                    disp_buffer = 0;  // Clear the buffer for new entry
                    new_entry = false;  // No longer a new entry
                }
                // Shift into buffer
                disp_buffer = (disp_buffer * 10) + (key - '0');
                snprintf(buf, sizeof(buf), "%8d", disp_buffer);
                display_char_print(buf); // Display the new value
            } else if (key == '#') {
                // If the key is '#', set the duty cycle
                percent = disp_buffer;
                if (percent > 100) {
                    percent = 100; // Cap at 100%
                }
                init_pwm_static(100, percent); // Update PWM with new duty cycle
                snprintf(buf, sizeof(buf), "%8d", percent);
                display_char_print(buf); // Display the new duty cycle
                new_entry = true;  // Ready for new entry
            }
            else if (key == '*') {
                // If the key is '*', reset the buffer
                disp_buffer = 50;
                percent = 50;
                init_pwm_static(100, percent); // Reset PWM to 50% duty cycle
                snprintf(buf, sizeof(buf), "      50");
                display_char_print(buf); // Display reset
                new_entry = true;  // Ready for new entry
            }
            else {
                // Any other key also starts a new entry
                new_entry = true;
            }
        }
    }
    #endif

    #ifdef STEP3
    init_pwm_static(10000, 5000); // Start out with 500/1000, 50%
    init_pwm_irq(); // Initialize PWM IRQ for variable duty cycle

    for(;;) {
        // The handler manages everything from now on.
        // Use the CPU to do something else!
        tight_loop_contents();
    }
    #endif
    
    #ifdef STEP4
    char freq_buf[9] = {0};
    int pos = 0;
    bool decimal_entered = false;
    int decimal_pos = 0;
    int current_channel = 0;

    keypad_init_pins();
    keypad_init_timer();
    display_init_pins();
    display_init_timer();

    init_pwm_audio(); 

    // set_freq(0, 440.0f); // Set initial frequency to 440 Hz (A4 note)
    // set_freq(1, 0.0f); // Turn off channel 1 initially
    // set_freq(0, 261.626f);
    // set_freq(1, 329.628f);

    set_freq(0, 440.0f); // Set initial frequency for channel 0
    display_char_print(" 440.000 ");

    for(;;) {
        uint16_t keyevent = key_pop();

        if (keyevent & 0x100) {
            char key = keyevent & 0xFF;
            if (key == 'A') {
                current_channel = 0;
                pos = 0;
                freq_buf[0] = '\0';
                decimal_entered = false;
                decimal_pos = 0;
                display_char_print("         ");
            } else if (key == 'B') {
                current_channel = 1;
                pos = 0;
                freq_buf[0] = '\0';
                decimal_entered = false;
                decimal_pos = 0;
                display_char_print("         ");
            } else if (key >= '0' && key <= '9') {
                if (pos == 0) {
                    snprintf(freq_buf, sizeof(freq_buf), "        "); // Clear buffer on first digit
                    display_char_print(freq_buf);
                }
                if (pos < 8) {
                    freq_buf[pos++] = key;
                    freq_buf[pos] = '\0';
                    display_char_print(freq_buf);
                    if (decimal_entered) decimal_pos++;
                }
                } else if (key == '*') {
                if (!decimal_entered && pos < 7) {
                    freq_buf[pos++] = '.';
                    freq_buf[pos] = '\0';
                    display_char_print(freq_buf);
                    decimal_entered = true;
                    decimal_pos = 0;
                }
                } else if (key == '#') {
                float freq = 0.0f;
                if (decimal_entered) {
                    freq = strtof(freq_buf, NULL);
                } else {
                    freq = (float)atoi(freq_buf);
                }
                set_freq(current_channel, freq);
                snprintf(freq_buf, sizeof(freq_buf), "%8.3f", freq);
                display_char_print(freq_buf);
                pos = 0;
                freq_buf[0] = '\0';
                decimal_entered = false;
                decimal_pos = 0;
            } else {
                // Reset on any other key
                pos = 0;
                freq_buf[0] = '\0';
                decimal_entered = false;
                decimal_pos = 0;
                display_char_print("        ");
            }
        }
    }
    #endif

    #ifdef DRUM_MACHINE
        drum_machine();
    #endif

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }

    for(;;);
    return 0;
}
