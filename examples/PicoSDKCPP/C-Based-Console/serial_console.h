#ifndef SERIAL_CONSOLE_H
#define SERIAL_CONSOLE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Callback invoked when a console command is executed.
 *
 * @param args  Pointer to the argument string passed to the command (may be
 *              an empty string). The implementation must not modify this
 *              pointer or assume it is nul-terminated beyond the presented
 *              string.
 */
typedef void (*command_callback_t)(const char *args);

/**
 * @brief Represents a single console command.
 *
 * - `name` is the literal command name to match (case-sensitive).
 * - `help_text` is a short description shown in command listings.
 * - `usage_text` is an optional usage string describing arguments.
 * - `callback` is called when the command is invoked.
 */
typedef struct {
    const char *name;
    const char *help_text;
    const char *usage_text;
    command_callback_t callback;
} command_t;

/**
 * @brief Serial console runtime state.
 *
 * - `input_buffer` stores the currently entered line, including a terminating
 *   nul when a full command is available.
 * - `buffer_index` is the current write index into `input_buffer`.
 * - `commands` points to an array of `command_t` definitions.
 * - `num_commands` is the number of entries in the `commands` array.
 */
typedef struct {
    char input_buffer[128];
    uint8_t buffer_index;
    const command_t *commands;
    uint8_t num_commands;
} serial_console_t;

/**
 * @brief Initialize a serial console instance.
 *
 * Prepares the console state to accept input and associates it with the
 * provided command table.
 *
 * @param console       Pointer to the `serial_console_t` instance to init.
 * @param commands      Pointer to a contiguous array of `command_t` entries.
 * @param num_commands  Number of entries in the `commands` array.
 */
void serial_console_init(serial_console_t *console, const command_t *commands, uint8_t num_commands);

/**
 * @brief Poll/update the serial console state.
 *
 * This should be called periodically from the main loop. The function will
 * read available serial input (platform-specific) and update the internal
 * buffer. When a full line/command is detected it will be parsed and the
 * matching command's callback will be invoked.
 *
 * @param console  Pointer to the `serial_console_t` instance to update.
 */
void serial_console_update(serial_console_t *console);

/**
 * @brief Print a nul-terminated message to the console with formatting.
 *
 * This higher-level print function may add line endings or other platform
 * friendly formatting. Use `console_print_raw` for minimal/raw output.
 *
 * @param message  Nul-terminated string to send to the console.
 */
void console_print(const char *message);

/**
 * @brief Print a nul-terminated message to the console with no additional
 *        formatting.
 *
 * Use this when the caller requires exact byte output (for example when
 * emitting binary-safe data or already-formatted strings).
 *
 * @param message  Nul-terminated string to send to the console.
 */
void console_print_raw(const char *message);

#endif // SERIAL_CONSOLE_H
