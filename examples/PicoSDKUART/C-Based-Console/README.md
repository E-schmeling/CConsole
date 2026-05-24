# C-Based-Console (Pico USB Serial)

Minimal example showing a simple serial console on the Raspberry Pi Pico using the Pico SDK using UART(pins 4&5).

This was built using VSC's "Pi Pico extension"

- Build: ensure `serial_console.c` is added to the target:
	```cmake
	add_executable(C-Based-Console C-Based-Console.c serial_console.c)
	```
 Due to Not using any builtin console/pring via UART or USB, disable them using:
    ```cmake
    pico_enable_stdio_uart(C-Based-Console 0)
    pico_enable_stdio_usb(C-Based-Console 0)
    ```


- Usage: call `stdio_init_all();`, `serial_console_init(...)`, then repeatedly call `serial_console_update()` in your main loop.

- Tab completion is disabled by default. To enable, define `SERIAL_CONSOLE_ENABLE_TAB_COMPLETION` in `serial_console.c` or add a compile definition:
	```cmake
	target_compile_definitions(C-Based-Console PRIVATE SERIAL_CONSOLE_ENABLE_TAB_COMPLETION)
	```
- Repeat last command(bang `!`) is disabled by default. To enable, define `SERIAL_CONSOLE_INCLUDE_BANG_COMMAND` in `serial_console.c` or see above for adding a compile definition.