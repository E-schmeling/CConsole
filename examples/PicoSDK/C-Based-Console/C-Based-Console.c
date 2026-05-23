#include <stdio.h>
#include "pico/stdlib.h"
#include "serial_console.h"

serial_console_t sm;

void cmd_hello(const char *args)
{
    console_print("Hello, world!\n");
}

void cmd_echo(const char *args)
{
    if (args && *args) {
        console_print(args);
        console_print("\n");
    }
}

const command_t commands[] =
{
    {"hello", "prints hello world", "help", cmd_hello},
    {"echo", "echos input", "echo <text>",cmd_echo}
};

int main()
{
    stdio_init_all();
    serial_console_init(&sm, commands, sizeof(commands) / sizeof(commands[0]));
    while (true) {
        serial_console_update(&sm);
        sleep_ms(1);
    }
}
