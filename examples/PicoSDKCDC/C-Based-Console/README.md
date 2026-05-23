# C-Based-Console (Pico USB Serial)

Minimal example showing a simple serial console on the Raspberry Pi Pico using the Pico SDK over USB CDC (virtual COM port).

This was built using VSC's "Pi Pico extension"

- Build: ensure `serial_console.c` is added to the target and enable USB stdio in `CMakeLists.txt`:
	```cmake
	pico_enable_stdio_usb(C-Based-Console 1)
	add_executable(C-Based-Console C-Based-Console.c serial_console.c)
	```
- Usage: call `stdio_init_all();`, `serial_console_init(...)`, then repeatedly call `serial_console_update()` in your main loop.

- Tab completion is disabled by default. To enable, define `SERIAL_CONSOLE_ENABLE_TAB_COMPLETION` in `serial_console.c` or add a compile definition:
	```cmake
	target_compile_definitions(C-Based-Console PRIVATE SERIAL_CONSOLE_ENABLE_TAB_COMPLETION)
	```
- As of now, this does not have ! command implementation, this is an easy fix that I am just being lazy.
- Nor does this have quite the documentationt that the external files has, I suggest using those, and adding your own implementation, this is just an example of its use. 