#include "serial_console.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// #define SERIAL_CONSOLE_ENABLE_TAB_COMPLETION


static void console_print_impl(const char *text) {
    // Default platform text output should be put here.
}

static int console_getchar_impl(void) {
    // Default platform char input should be put here.
}

void serial_console_init(serial_console_t *console, const command_t *commands, uint8_t num_commands) {
    console->buffer_index = 0;
    console->commands = commands;
    console->num_commands = num_commands;
    memset(console->input_buffer, 0, sizeof(console->input_buffer));
}

static void trim_string(char *str) {
    char *start = str;
    char *end = str + strlen(str) - 1;
    
    while (*start && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    *(end + 1) = 0;
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

static void cmd_help(serial_console_t *console) {
    console_print_impl("=== Available Commands ===\n");
    for (uint8_t i = 0; i < console->num_commands; i++) {
        char line[256];
        snprintf(line, sizeof(line), "Command: %s\n    %s\n    Usage: %s\n", console->commands[i].name,
                 console->commands[i].help_text,
                 console->commands[i].usage_text ? console->commands[i].usage_text : "n/a");
        console_print_impl(line);
    }
}

#ifdef SERIAL_CONSOLE_ENABLE_TAB_COMPLETION
static void handle_tab_completion(serial_console_t *console) {
    // find token start at beginning + leading spaces
    size_t len = console->buffer_index;
    size_t start = 0;
    while (start < len && isspace((unsigned char)console->input_buffer[start])) {
        start++;
    }

    size_t token_len = len - start;
    if (token_len == 0) {
        return;
    }

    char *token = console->input_buffer + start;
    command_t *match = NULL;
    uint8_t matches = 0;
    bool help_matches = (strncmp("help", token, token_len) == 0);
    
    // Check user-defined commands
    for (uint8_t i = 0; i < console->num_commands; i++) {
        if (strncmp(console->commands[i].name, token, token_len) == 0) {
            matches++;
            if (matches == 1) {
                match = (command_t *)&console->commands[i];
            }
        }
    }
    
    // Include built-in help command
    if (help_matches) {
        matches++;
        if (matches == 1) {
            static command_t help_cmd = {"help", "Show available commands", "help", NULL};
            match = &help_cmd;
        }
    }

    if (matches == 0) {
        return;
    }

    if (matches == 1 && match != NULL) {
        size_t cmd_len = strlen(match->name);
        if (token_len == cmd_len) {
            char line[256];
            snprintf(line, sizeof(line), "\nUsage: %s\n", match->usage_text ? match->usage_text : match->name);
            console_print_impl(line);
            console_print_impl("> ");
            console_print_impl(console->input_buffer);
            return;
        }

        const char *rest = match->name + token_len;
        while (*rest && console->buffer_index < sizeof(console->input_buffer) - 1) {
            console->input_buffer[console->buffer_index++] = *rest;
            rest++;
        }
        console->input_buffer[console->buffer_index] = '\0';

        console_print_impl(match->name + token_len);
        return;
    }

    // Multiple matches: show candidates
    console_print_impl("\nPossible matches:\n");
    
    if (help_matches) {
        console_print_impl("  help\n");
    }
    
    // Show user-defined commands that match
    for (uint8_t i = 0; i < console->num_commands; i++) {
        if (strncmp(console->commands[i].name, token, token_len) == 0) {
            char line[80];
            snprintf(line, sizeof(line), "  %s\n", console->commands[i].name);
            console_print_impl(line);
        }
    }
    console_print_impl("> ");
    console_print_impl(console->input_buffer);
}
#endif

void serial_console_update(serial_console_t *console) {
    int c = console_getchar_impl();
    
    if (c == -1) {
        return;
    }


    #ifdef SERIAL_CONSOLE_ENABLE_TAB_COMPLETION
    if (c == '\t') {
        handle_tab_completion(console);
        return;
    }
    #endif
    
    if (c == '\r' || c == '\n') {
        if (console->buffer_index > 0) {
            console->input_buffer[console->buffer_index] = '\0';
            
            trim_string(console->input_buffer);
            
            console_print_impl("\n");
            
            char *command = console->input_buffer;
            char *args = strstr(console->input_buffer, " ");
            if (args) {
                *args = '\0';
                args++;
                trim_string(args);
            }
            
            if (strcmp(command, "help") == 0) {
                cmd_help(console);
            } else {
                bool found = false;
                for (uint8_t i = 0; i < console->num_commands; i++) {
                    if (strcmp(command, console->commands[i].name) == 0) {
                        if (console->commands[i].callback) {
                            console->commands[i].callback(args);
                        }
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    console_print_impl("Unknown command. Type 'help' for available commands.\n");
                }
            }
            
            console->buffer_index = 0;
            memset(console->input_buffer, 0, sizeof(console->input_buffer));
            console_print_impl("> ");
        }
        return;
    }
    
    if (c == '\b' || c == 127) {
        if (console->buffer_index > 0) {
            console->buffer_index--;
            console->input_buffer[console->buffer_index] = '\0';
            console_print_impl("\b \b");  // Backspace, space, backspace
        }
        return;
    }
    
    if (console->buffer_index < sizeof(console->input_buffer) - 1 && isprint((unsigned char)c)) {
        console->input_buffer[console->buffer_index++] = c;
        char ch[2] = {(char)c, '\0'};
        console_print_impl(ch);  // Echo character
    }
}

void console_print(const char *message) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s\n", message);
    console_print_impl(buf);
}

void console_print_raw(const char *message) {
    console_print_impl(message);
}
