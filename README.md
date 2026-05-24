# CConsole

A minimal serial console framework for embedded C systems.

This repository provides a small command parsing console core written in C, which can include support for tab completion and a '!' history command. There are multiple example integrations for the Pi Pico specifically because I am lazy, and it should demonstrate the easy changes that need to be made between environments. 

The core console engine lives in `serial_console.c` and `serial_console.h`. It handles command parsing, input buffering, help listing, and command dispatch. The platform-specific parts are intentionally isolated so the same console logic can be reused across boards.

## Structure

- `serial_console.h`: public API for the console system.
- `serial_console.c`: core console implementation and command handling.

- `examples/`: example projects showing how to connect the core to different serial transports.

## How to use

This console system is designed to work with any embedded board as long as the transport-specific hooks are implemented.

The key integration points are:

- `device_specific_init()`: initialise the board's serial transport or USB CDC peripheral.
- `console_print_impl()`: send a nul-terminated string out over the chosen transport.
- `console_getchar_impl()`: read a single character if available and return it, or return `-1` when no character is ready.

> NOTE: Using this with a C++ object-based transport such as Arduino `Serial` requires a small bridge layer. See the `examples/PicoSDKCPP/C-Based-Console` example for a working pattern that keeps the console core in C while implementing the transport in C++.

These three functions are the only things that should change between boards if the console core remains the same.

The console also supports two optional compile-time features:

- `SERIAL_CONSOLE_ENABLE_TAB_COMPLETION`: enable simple command completion on `\t`.
- `SERIAL_CONSOLE_INCLUDE_BANG_COMMAND`: enable `!` command history / replay support.

> NOTE: Only 1 layer of command history is supported and replayed using '!' this is to minimise memory use. If it ever becomes helpful for me I will possibly add a second layer to '!!' but no one will ever use this besides me so who am I kidding, this is just hopefully to impress people whomst look at my github in course of job applications.

> If you are looking at this outside of looking at my job application please let me know, I am working on making embedded libraries for different things this is the first.

## Examples

### `examples/PicoCPP/C-Based-Console`

- Arduino-style example using a C++ bridge file.
- Keeps the console core in C while implementing the serial port with Arduino `Serial`.
- Demonstrates the object interaction model needed when the board's serial API is C++.

### `examples/PicoSDKUART/C-Based-Console`

- Pico SDK example using raw UART APIs.
- Shows how the console core can be wired directly to a UART peripheral.
- Best when the board exposes a C UART API and no object-based bridge is needed.

### `examples/PicoSDKCDC/C-Based-Console`

- Pico SDK example using USB CDC for virtual serial over USB.
- Shows how the same console core works over USB serial instead of UART.
- Useful for boards that provide USB serial natively.

## Notes

- The console core itself is C, but the transport implementation can be either C or C++ depending on the board.
- The required board-specific changes are not limited to the Arduino example; any port must update the initialisation, input, and output hooks.
- Do not make the transport hooks `static` if they are implemented in a different translation unit or language boundary.
- If a board offers a direct C serial API, use that to keep the integration simple. The C++ bridge is only necessary when the transport API is object-based.
- This project is released under the MIT license; but if anyone actually uses this, I would love to know.

