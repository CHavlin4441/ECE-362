# ECE 36200 Labs – Proton Development Board

My lab work for **ECE 36200: Microprocessor Systems and Interfacing** at Purdue University. The labs are completed on the Purdue **Proton** development board, which is built around the Raspberry Pi RP2350 microcontroller.

## Labs

| Lab | Topic | What it covers |
| --- | ----- | -------------- |
| [Lab 0](./lab-0-intro-CHavlin4441-main) | Introduction | Toolchain setup, building and flashing the board, serial debugging |
| [Lab 1](./lab-1-gpio-CHavlin4441-main) | GPIO | Configuring pins as inputs and outputs, reading buttons, driving LEDs |
| [Lab 2](./lab-2-interrupts-CHavlin4441-main) | Interrupts | Handling GPIO events with interrupt service routines instead of polling |
| [Lab 3](./lab-3-timers-CHavlin4441-main) | Timers | Hardware timers and alarms for periodic and time-based events |
| [Lab 4](./lab-4-adc-and-dma-CHavlin4441-main) | ADC and DMA | Sampling analog signals and moving data with DMA without CPU involvement |
| [Lab 5](./lab-5-pwm-CHavlin4441-main) | PWM | Generating pulse-width-modulated signals for brightness and audio output |
| [Lab 6](./lab-6-spi-CHavlin4441-main) | SPI | Communicating with peripherals and displays over SPI |
| [Lab 7](./lab-7-uart-CHavlin4441-main) | UART | Serial communication and building a simple text interface |
| [Lab 8](./lab-8-intro-CHavlin4441-main) | Intro to RISC-V Assembly | Writing and debugging basic RISC-V assembly on the RP2350's Hazard3 cores |
| [Lab 9](./lab-9-control-flow-CHavlin4441-main) | Control Flow | Branches, loops, and conditionals in assembly |
| [Lab 10](./lab-10-functions-CHavlin4441-main) | Functions | Calling conventions, the stack, and writing reusable assembly functions |

Labs 0–7 cover embedded programming in C. Labs 8–10 cover RISC-V assembly.

The full instructions for each lab are in the official course repository linked below.

## Building

Each lab folder is a standalone project based on the course code template. To build one:

1. Install the Raspberry Pi Pico SDK and toolchain as described in Lab 0.
2. Open the lab folder in VS Code (or your preferred editor).
3. Build the project and flash it to the Proton board over USB.

## Credits

The lab experiments, instructions, and starter code templates were created by **Niraj Menon** and **Purdue University** for ECE 36200. The original material is available here:

- Course lab repository: [ece362-purdue/proton-labs](https://github.com/ece362-purdue/proton-labs)
- Proton board resources: [ece362-purdue.github.io/proton-labs](https://ece362-purdue.github.io/proton-labs/)

My contribution is the code I wrote to complete each lab.

## License

The original lab material, *ECE 36200 Lab Experiments (Proton Development Board)* © 2026 by Purdue University and Niraj Menon, is licensed under [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)](https://creativecommons.org/licenses/by-nc-sa/4.0/).

Because this repository builds on that material, it is shared under the same CC BY-NC-SA 4.0 license.