# PicoSDKCPP Arduino Serial Console Example

This example demonstrates how to use the `serial_console` system from C code while providing Arduino serial I/O through a C++ bridge.

## Purpose

- Show how the C-based console core (`serial_console.c`, `serial_console.h`) can interact with an object-based Arduino API.
- Provide a working example for Arduino serial input/output using `Serial`.
- Demonstrate a practical bridge when your main skill and core logic are in C, but the target platform provides a C++ serial object.

## Key Notes

- This example is intentionally included to show how the system-object interaction is done.
- The design is a workaround to let a C module call Arduino `Serial` without converting the whole console system to C++.
- My skills are in C for embedded systems, not C++ in embedded systems. There may be a smoother or more idiomatic C++ solution, but this approach has worked for an Arduino project.
- This is an Arduino example and uses the Arduino serial interface (`Serial`).

## Required changes

### `serial_console.h`

- The public header must expose only the true public API.
- Internal platform functions such as `console_print_impl()`, `console_getchar_impl()`, and `device_specific_init()` must not be declared as `static` in the header.
- They must be declared with external linkage so the C translation unit can call the implementation provided by the C++ file.

### `serial_console.c`

- The C file should declare those platform hooks with normal external linkage.
- It should not define them as `static` in the C file if they are implemented elsewhere.
- The C file should simply call `device_specific_init()` and `console_print_impl()` through those external declarations.

### `serial-cpp.cpp`

- This file provides the Arduino implementation using `Serial`.
- It uses `extern "C"` and non-static function definitions so the C code can link to them.
- `device_specific_init()` initializes `Serial`.
- `console_print_impl()` sends text through `Serial.print()`.
- `console_getchar_impl()` reads from `Serial.available()` and returns `Serial.read()` or `-1`.

## What this shows

- A C core module can remain C-based while still using Arduino's C++ serial object.
- The bridge file is the only Arduino-specific piece in this example.
- The rest of the serial console module stays portable and can be reused on other platforms by replacing just the bridge file.

## Build and use

- Open this example in the Arduino environment or your preferred Arduino-compatible build system.
- Ensure `serial-cpp.cpp` is compiled along with `serial_console.c` and your sketch.
- Use `Serial.begin(115200)` in the bridge and open the serial monitor at the same baud rate.

## Note
This example is a practical solution, I used to on a school project. It is not meant to be the cleanest modern C++ design, but it is a workable bridge for a C-based embedded console system and Arduino serial.
