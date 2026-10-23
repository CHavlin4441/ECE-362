#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////////

// No autotest, but you might want to set this anyway.
const char* username = "havlinc";

// When testing basic TX/RX
// #define STEP2
// When connecting UART to printf(), fgets()
#define STEP3
// When testing UART IRQ for buffering
// #define STEP4
// When testing PCS
// #define STEP5

//////////////////////////////////////////////////////////////////////////////

void init_uart() 
{
    //initialize UART0 with the specified baud rate and settings.
    gpio_set_function(0, UART_FUNCSEL_NUM(uart0, 0));
    gpio_set_function(1, UART_FUNCSEL_NUM(uart0, 1));

    //set the UART parameters: baud rate, data bits, stop bits, and parity.
    uart_init(uart0, 115200);
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, 8, 1, 0);
}

#ifdef STEP2
int main() {
    init_uart();
    for (;;) {
        char buf[2];
        uart_read_blocking(uart0, (uint8_t*)buf, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0'; // Ensure null-termination
        uart_puts(uart0, "You said: ");
        uart_puts(uart0, buf);
        uart_puts(uart0, "\n");
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef STEP3

// 3.3
int _read(__unused int handle, char *buffer, int length) 
{
    // Your code here to read from the UART and fill the buffer.
    // DO NOT USE THE STDIO_* FUNCTIONS FROM ABOVE.  Only UART ones.

    // The argument "handle" is unused.  This is meant for use with 
    // files, which are not very different from text streams.  However, 
    // we read from the UART, not the file specified by the handle.

    // handle is irrelevant since these functions will only ever be called 
    // by the correct functions.  No need for an if statement.

    // Instructions: Given the buffer and a specific length to read, read 1 
    // character at a time from the UART until the buffer is 
    // filled or the length is reached. 
    int byte = 0;

    //if the user types backspace, remove the last character 
    //from the buffer and move the cursor back in the terminal.  
    //If the user types a character, add it to the buffer and echo it back to the terminal.  
    //Continue until the buffer is filled or the length is reached.        
    while(byte < length) 
    {
        while(!uart_is_readable(uart0)) 
        {
            tight_loop_contents();
        }

        char c = uart_getc(uart0);

        if(c == 8)
        {
            uart_putc_raw(uart0, 8); //backspace
            uart_putc_raw(uart0, 32); //space to erase character
            uart_putc_raw(uart0, 8); //backspace again to move cursor back

            byte--;
            continue;
        }
        else
        {
            uart_putc_raw(uart0, c); //echo character back to terminal
        }

        buffer[byte] = c;
        byte++;
    }

    return(byte);
}

int _write(__unused int handle, char *buffer, int length) 
{
    // Your code here to write to the UART from the buffer.
    // DO NOT USE THE STDIO_* FUNCTIONS FROM ABOVE.  Only UART ones.

    // The argument "handle" is unused.  This is meant for use with 
    // files, which are not very different from text streams.  However, 
    // we write to the UART, not the file specified by the handle.

    // handle is irrelevant since these functions will only ever be called 
    // by the correct functions.  No need for an if statement.

    // Instructions: Given the buffer and a specific length to write, write 1
    // character at a time to the UART until the length is reached. 
    int byte = 0;

    //if the user types backspace, remove the last character 
    //from the buffer and move the cursor back in the terminal.
    for(byte = 0; byte < length; byte++) 
    {
        while(!uart_is_writable(uart0)) 
        {
            tight_loop_contents();
        }

        uart_putc(uart0, buffer[byte]);
    }

    return(byte);
}

int main()
{
    init_uart();

    // insert any setbuf lines below...
    setbuf(stdout, NULL);  // Disable buffering for stdout
    setbuf(stdin, NULL);   // Disable buffering for stdin
    char name[8];
    int age = 0;
    for(;;) {
        printf("Enter your name and age: ");
        scanf("%s %d", name, &age);
        printf("Hello, %s! You are %d years old.\n", name, age);
        sleep_ms(100);  // in case the output loops and is too fast 
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef STEP4

#define BUFSIZE 32
char serbuf[BUFSIZE];
int seridx = 0;
int newline_seen = 0;

//add this here so that compiler does not complain about implicit function
void uart_rx_handler();

//initialize UART interrupts to trigger on every received character and call uart_rx_handler when an interrupt occurs.
void init_uart_irq() 
{
    uart0_hw->lcr_h &= ~UART_UARTLCR_H_FEN_BITS; //disable FIFOs to trigger interrupt on every character
    uart0_hw->imsc |= UART_UARTIMSC_RXIM_BITS; //enable RX interrupt

    irq_set_exclusive_handler(UART0_IRQ, uart_rx_handler);
    irq_set_enabled(UART0_IRQ, true);
}

//handle UART RX interrupts by reading the received character and storing it in a buffer until a newline is seen.
void uart_rx_handler() 
{
    uart0_hw->icr = UART_UARTICR_RXIC_BITS; //clear RX interrupt

    if(seridx >= BUFSIZE)
    {
        (void)uart_getc(uart0); //discard character if buffer is full
        return;
    }

    char c = (char)(uart0_hw->dr & 0xFF); //read character from UART data register

    //if the character is a newline, set the newline flag to indicate that a full line has been received.
    if(c == '\n' || c == '\r')
    {
        newline_seen = 1;
    }

    else if(c == 8 || c == 0x7F)
    {
        if(seridx > 0) 
        {
            uart_putc_raw(uart0, 8); //backspace
            uart_putc_raw(uart0, 32); //space to erase character
            uart_putc_raw(uart0, 8); //backspace again to move cursor back

            seridx--; //move back in buffer
            serbuf[seridx] = '\0'; //null-terminate buffer after backspace
        }

    }

    //if the character is a backspace, remove the last character from the buffer and move the cursor back in the terminal.  
    //If the character is a regular character, add it to the buffer and echo it back to the terminal.  
    //Continue until the buffer is filled or a newline is seen.
    else
    {
        uart_putc_raw(uart0, c); //echo character back to terminal
        serbuf[seridx] = c; //add character to buffer
        seridx++;
    }

}

int _read(__unused int handle, char *buffer, int length) 
{
    while(!newline_seen) 
    {
        sleep_ms(5); //wait for newline to be seen
    }

    newline_seen = 0; //reset newline flag

    int copy = (length < seridx) ? length : seridx; //number of characters to copy to buffer
    
    memcpy(buffer, serbuf, copy); //copy characters to buffer

    if(copy < length) 
    {
        buffer[copy] = '\0'; //null-terminate buffer if there is space
    }
    else
    {
        buffer[length - 1] = '\0'; //null-terminate buffer at max length
    }
    
    seridx = 0; //reset buffer index

    return(length); //return number of characters read 
}

int _write(__unused int handle, char *buffer, int length) 
{
    int i;
    for(i = 0; i < length; i++) 
    {
        while(!uart_is_writable(uart0)) 
        {
            tight_loop_contents();
        }

        uart_putc(uart0, buffer[i]);
    }

    return(i); //return number of characters written
}

int main() {
init_uart();
init_uart_irq();

setbuf(stdout, NULL); // Disable buffering for stdout

char name[8];
int age = 0;
for(;;) {
    printf("Enter your name and age: ");
    scanf("%s %d", name, &age);
    // THIS IS IMPORTANT.
    fflush(stdin);
    printf("\nHello, %s! You are %d years old.\r\n", name, age);
    sleep_ms(100);  // in case the output loops and is too fast
}
}

#endif

//////////////////////////////////////////////////////////////////////////////

#ifdef STEP5

// Copy global variables, init_uart_irq, uart_rx_handler, _read, and _write from STEP4.
#define BUFSIZE 32
char serbuf[BUFSIZE];
int seridx = 0;
int newline_seen = 0;

void uart_rx_handler();

void init_uart_irq() 
{
    uart0_hw->lcr_h &= ~UART_UARTLCR_H_FEN_BITS; //disable FIFOs to trigger interrupt on every character
    uart0_hw->imsc |= UART_UARTIMSC_RXIM_BITS; //enable RX interrupt

    irq_set_exclusive_handler(UART0_IRQ, uart_rx_handler);
    irq_set_enabled(UART0_IRQ, true);
}

void uart_rx_handler() 
{
    uart0_hw->icr = UART_UARTICR_RXIC_BITS; //clear RX interrupt

    if(seridx >= BUFSIZE)
    {
        (void)uart_getc(uart0); //discard character if buffer is full
        return;
    }

    char c = (char)(uart0_hw->dr & 0xFF); //read character from UART data register

    if(c == '\n' || c == '\r')
    {
        newline_seen = 1;
    }

    else if(c == 8 || c == 0x7F)
    {
        if(seridx > 0) 
        {
            uart_putc_raw(uart0, 8); //backspace
            uart_putc_raw(uart0, 32); //space to erase character
            uart_putc_raw(uart0, 8); //backspace again to move cursor back

            seridx--; //move back in buffer
            serbuf[seridx] = '\0'; //null-terminate buffer after backspace
        }

    }

    else
    {
        uart_putc_raw(uart0, c); //echo character back to terminal
        serbuf[seridx] = c; //add character to buffer
        seridx++;
    }

}

int _read(__unused int handle, char *buffer, int length) 
{
    while(!newline_seen) 
    {
        sleep_ms(5); //wait for newline to be seen
    }

    newline_seen = 0; //reset newline flag

    int copy = (length < seridx) ? length : seridx; //number of characters to copy to buffer
    
    memcpy(buffer, serbuf, copy); //copy characters to buffer

    if(copy < length) 
    {
        buffer[copy] = '\0'; //null-terminate buffer if there is space
    }
    else
    {
        buffer[length - 1] = '\0'; //null-terminate buffer at max length
    }
    
    seridx = 0; //reset buffer index

    return(length); //return number of characters read 
}

int _write(__unused int handle, char *buffer, int length) 
{
    int i;
    for(i = 0; i < length; i++) 
    {
        while(!uart_is_writable(uart0)) 
        {
            tight_loop_contents();
        }

        uart_putc(uart0, buffer[i]);
    }

    return(i); //return number of characters written
}

void cmd_gpio(int argc, char **argv) {
    // This is the main command handler for the "gpio" command.
    // It will call either cmd_gpio_out or cmd_gpio_set based on the arguments.
    
    // Ensure that argc is at least 2, otherwise print an example use case and return.
    // If the second argument is "out":
    //      Ensure that argc is exactly 3, otherwise print an example use case and return.
    //      Convert the third argument to an integer pin number using atoi.
    //      Check if the pin number is valid (0-47), otherwise print an error and return.
    //      Set the pin to output using gpio_init and gpio_set_dir.
    //      Print a success message.
    // If the second argument is "set":
    //      Ensure that argc is exactly 4, otherwise print an example use case and return.
    //      Convert the third argument to an integer pin number using atoi.
    //      Check if the pin number is valid (0-47), otherwise print an error and return.
    //      Check if the pin has been initialized as a GPIO output, if not, return.
    //      Convert the fourth argument to an integer value (0 or 1) using atoi.
    //      Check if the value is valid (0 or 1), otherwise print an error and return.
    //      Set the pin to the specified value using gpio_put.
    //      Print a success message.
    
    // Else, print an unknown command error.
    if(argc < 2) 
    {
        printf("\r\nExample usage: gpio out 22\r\n");
        printf("\r\nExample usage: gpio set 22 1\r\n");
        return;
    }

    if(strcmp(argv[1], "out") == 0) 
    {
        if(argc != 3) 
        {
            printf("Example usage: gpio out 22\r\n");
            return;
        }

        int pin = atoi(argv[2]);

        if(pin < 0 || pin > 47) 
        {
            printf("\r\nInvalid pin number %d. Must be between 0 and 47.\r\n", pin);
            return;
        }

        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        printf("\r\nInitialized pin %d as output.\r\n", pin);
    }
    
    else if(strcmp(argv[1], "set") == 0) 
    {
        if(argc != 4) 
        {
            printf("\r\nExample usage: gpio set 22 1\r\n");
            return;
        }

        int pin = atoi(argv[2]);

        if(pin < 0 || pin > 47) 
        {
            printf("\r\nInvalid pin number %d. Must be between 0 and 47.\r\n", pin);
            return;
        }

        if(!gpio_get_dir(pin)) 
        {
            printf("\r\nPin %d is not initialized as an output.\r\n", pin);
            return;
        }

        int value = atoi(argv[3]);

        if(value != 0 && value != 1) 
        {
            printf("\r\nInvalid value. Must be 0 or 1.\r\n", value);
            return;
        }

        gpio_put(pin, value);
        printf("\r\nSet pin %d to %d.\r\n", pin, value);
    }
    else 
    {
        printf("\r\nUnknown command '%s'.\r\n", argv[1]);
    }

}

int main() 
{
    // See lab for instructions.

    init_uart();
    init_uart_irq();

    setbuf(stdout, NULL); //disable buffering for stdout

    printf("\r\n%s's Peripheral Command Shell (PCS)\r\n", username);
    printf("\r\nEnter a command below.\r\n\r\n");

    int argc; //number of arguments
    char *argv[10]; //array of argument strings
    char input[100]; //buffer for user input

    while(1) 
    {
        argc = 0; //reset argument count

        printf("\r\n> "); //prompt for input
        fgets(input, sizeof(input), stdin); //read user input from terminal
        
        fflush(stdin); //read user input

        input[strcspn(input, "\n")] = '\0'; //remove newline characters from input

        char *token = strtok(input, " "); //tokenize input by spaces
        while(token != NULL && argc < 10) 
        {
            argv[argc] = token; //store token in argv
            argc++; //increment argument count
            token = strtok(NULL, " "); //get next token
        }

        if(argc == 0) 
        {
            continue; //no command entered, prompt again
        }

        if(strcmp(argv[0], "gpio") == 0) 
        {
            cmd_gpio(argc, argv); //handle gpio command
        }
        else 
        {
            printf("\r\nUnknown command '%s'.\r\n", argv[0]);
        }

    }

    return(0);
}

#endif

//////////////////////////////////////////////////////////////////////////////
