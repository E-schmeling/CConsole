#ifndef SERIAL_CONSOLE_H
#define SERIAL_CONSOLE_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*command_callback_t)(const char *args);

typedef struct {
    const char *name;
    const char *help_text;
    const char *usage_text;
    command_callback_t callback;
} command_t;

typedef struct {
    char input_buffer[128];
    uint8_t buffer_index;
    const command_t *commands;
    uint8_t num_commands;
} serial_console_t;

void serial_console_init(serial_console_t *console, const command_t *commands, uint8_t num_commands);

void serial_console_update(serial_console_t *console);

void console_print(const char *message);

void console_print_raw(const char *message);

#endif // SERIAL_CONSOLE_H
