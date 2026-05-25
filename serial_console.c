/**
 * @file serial_console.c
 * @brief Simple serial console implementation in C.
 * @author ERS
 *
 * This module provides a basic framework for a serial console with command parsing,
 * built-in help, and optional features like tab-completion and command history.
 */

/** =======================================================================
 *  Routine Defines
 *  =======================================================================
 */
#include "serial_console.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// Optional features
// #define SERIAL_CONSOLE_ENABLE_TAB_COMPLETION
// #define SERIAL_CONSOLE_INCLUDE_BANG_COMMAND


/** =======================================================================
 *  Device-specific Defines
 *  =======================================================================
 */


 
/** =======================================================================
 *  Function prototypes for internal routines
 *  =======================================================================
 */
static void console_print_impl(const char *text);

static int16_t console_getchar_impl(void);

static void device_specific_init(void);

void serial_console_init(serial_console_t *console, const command_t *commands, uint8_t num_commands);

static void trim_string(char *str);

static void cmd_help(serial_console_t *console);

#ifdef SERIAL_CONSOLE_INCLUDE_BANG_COMMAND
static void save_last_command(const char *command, const char *args);

static bool execute_saved_command(serial_console_t *console, const char *command, const char *args);

static void cmd_bang(serial_console_t *console, const char *args);
#endif

#ifdef SERIAL_CONSOLE_ENABLE_TAB_COMPLETION
static void handle_tab_completion(serial_console_t *console);
#endif


/** =======================================================================
 *  Global Variables
 *  =======================================================================
 */
#ifdef SERIAL_CONSOLE_INCLUDE_BANG_COMMAND
static char last_command_name[128];
static char last_command_args[128];
static bool has_last_command = false;
#endif

/** =======================================================================
 *  Device-specific implementations (to be customized)
 *  =======================================================================
 */

/**
 * @brief Low-level output routine (platform-specific).
 *
 * Implement this function to actually send `text` to the target console or
 * serial peripheral. This function is intentionally minimal and is used by
 * higher-level helpers in this module. Keep it non-blocking if possible.
 *
 * @param text Nul-terminated string to send.
 */
static void console_print_impl(const char *text)
{
    // Implement this function to send the text to your output.
}

/**
 * @brief Non-blocking input character provider.
 *
 * Called repeatedly by `serial_console_update()` to obtain incoming input.
 * Return the next available character as an `int16_t`
 * or `-1` when no character is currently available. Returning `-1` allows
 * the caller to continue without stalling.
 *
 * @return Next input character, or -1 if none available.
 */
static int16_t console_getchar_impl(void)
{
    // Implement this function to read a character from your input.
    return -1; // Return -1 if no character is available
}

/**
 * @brief Device-specific initialization hook for the console.
 *
 * Perform any board/peripheral setup required for `console_print_impl`
 * and `console_getchar_impl` to operate correctly (UART config, GPIOs,
 * buffering, interrupts, etc.). Keep this function small and idempotent.
 */
static void device_specific_init(void)
{
    // Implement any necessary initialization for your specific hardware here.
}

/** =======================================================================
 *  Console implementation
 *  =======================================================================
 */

void serial_console_init(serial_console_t *console, const command_t *commands, uint8_t num_commands)
{
    device_specific_init();
    console->buffer_index = 0;
    console->commands = commands;
    console->num_commands = num_commands;
    memset(console->input_buffer, 0, sizeof(console->input_buffer));
}

/**
 * @internal
 * @brief Trim leading and trailing ASCII whitespace in-place.
 *
 * Modifies the provided NUL-terminated string. After this call the string
 * will have no leading or trailing whitespace and will remain NUL-terminated.
 *
 * @param str Nul-terminated string to trim (modified in-place).
 */
static void trim_string(char *str)
{
    char *start = str;
    char *end = str + strlen(str) - 1;

    while (*start && isspace((unsigned char)*start))
        start++;
    while (end > start && isspace((unsigned char)*end))
        end--;

    *(end + 1) = 0;
    if (start != str)
    {
        memmove(str, start, strlen(start) + 1);
    }
}

/**
 * @internal
 * @brief Built-in `help` command implementation.
 *
 * Iterates the registered command table and prints a short description and
 * usage string for each entry. This is a convenience command available even
 * when user code does not register a `help` command explicitly.
 *
 * @param console Pointer to the active `serial_console_t` instance.
 */
static void cmd_help(serial_console_t *console)
{
    console_print_impl("=== Available Commands ===\n");
    for (uint8_t i = 0; i < console->num_commands; i++)
    {
        char line[256];
        snprintf(line, sizeof(line), "Command: %s\n    %s\n    Usage: %s\n", console->commands[i].name,
                 console->commands[i].help_text,
                 console->commands[i].usage_text ? console->commands[i].usage_text : "n/a");
        console_print_impl(line);
    }
}

#ifdef SERIAL_CONSOLE_INCLUDE_BANG_COMMAND
/**
 * @internal
 * @brief Save the last executed command for later replay with `!`.
 *
 * This function is called after successfully executing a command (except `!` itself)
 * to store the command name and arguments.
 *
 * @param command The command name that was executed.
 * @param args The arguments that were passed to the command (may be NULL or empty).
 */
static void save_last_command(const char *command, const char *args)
{
    if (!command || command[0] == '\0' || strcmp(command, "!") == 0)
    {
        return;
    }

    strncpy(last_command_name, command, sizeof(last_command_name));
    last_command_name[sizeof(last_command_name) - 1] = '\0';

    if (args && args[0] != '\0')
    {
        strncpy(last_command_args, args, sizeof(last_command_args));
        last_command_args[sizeof(last_command_args) - 1] = '\0';
    }
    else
    {
        last_command_args[0] = '\0';
    }

    has_last_command = true;
}

/**
 * @internal
 * @brief Execute a previously saved command.
 *
 * @param console Pointer to the active `serial_console_t` instance.
 * @param command The command name to execute.
 * @param args The arguments for the command (may be NULL or empty).
 * @return true if the command was found and executed, false otherwise.
 */
static bool execute_saved_command(serial_console_t *console, const char *command, const char *args)
{
    if (strcmp(command, "help") == 0)
    {
        cmd_help(console);
        return true;
    }

    for (uint8_t i = 0; i < console->num_commands; i++)
    {
        if (strcmp(command, console->commands[i].name) == 0)
        {
            if (console->commands[i].callback)
            {
                console->commands[i].callback(args);
            }
            return true;
        }
    }

    return false;
}

/**
 * @internal
 * @brief Handle the `!` command for replaying the last executed command.
 *
 * @param console Pointer to the active `serial_console_t` instance.
 * @param args Will be ignored, included for consistency.
 */
static void cmd_bang(serial_console_t *console, const char *args)
{
    (void)args;

    if (!has_last_command)
    {
        console_print_impl("No previous command to repeat.\n");
        return;
    }

    console_print_impl("Repeating last command: ");
    console_print_impl(last_command_name);
    if (last_command_args[0] != '\0')
    {
        console_print_impl(" ");
        console_print_impl(last_command_args);
    }
    console_print_impl("\n");

    const char *replay_args = last_command_args[0] ? last_command_args : NULL;
    if (!execute_saved_command(console, last_command_name, replay_args))
    {
        console_print_impl("Previous command is no longer available.\n");
    }
}
#endif

#ifdef SERIAL_CONSOLE_ENABLE_TAB_COMPLETION
/**
 * @internal
 * @brief Handle simple tab-completion for the current input buffer.
 *
 * Attempts to complete the current token against registered commands and the
 * built-in `help` command. Writes completed characters into
 * `console->input_buffer` and echoes completion candidates to the output.
 *
 * @param console Pointer to the active `serial_console_t` instance.
 */
static void handle_tab_completion(serial_console_t *console)
{
    // find token start at beginning + leading spaces
    size_t len = console->buffer_index;
    size_t start = 0;
    while (start < len && isspace((unsigned char)console->input_buffer[start]))
    {
        start++;
    }

    size_t token_len = len - start;
    if (token_len == 0)
    {
        return;
    }

    char *token = console->input_buffer + start;
    command_t *match = NULL;
    uint8_t matches = 0;
    bool help_matches = (strncmp("help", token, token_len) == 0);

    // Check user-defined commands
    for (uint8_t i = 0; i < console->num_commands; i++)
    {
        if (strncmp(console->commands[i].name, token, token_len) == 0)
        {
            matches++;
            if (matches == 1)
            {
                match = (command_t *)&console->commands[i];
            }
        }
    }

    // Include built-in help command
    if (help_matches)
    {
        matches++;
        if (matches == 1)
        {
            static command_t help_cmd = {"help", "Show available commands", "help", NULL};
            match = &help_cmd;
        }
    }

    if (matches == 0)
    {
        return;
    }

    if (matches == 1 && match != NULL)
    {
        size_t cmd_len = strlen(match->name);
        if (token_len == cmd_len)
        {
            char line[256];
            snprintf(line, sizeof(line), "\nUsage: %s\n", match->usage_text ? match->usage_text : match->name);
            console_print_impl(line);
            console_print_impl("> ");
            console_print_impl(console->input_buffer);
            return;
        }

        const char *rest = match->name + token_len;
        while (*rest && console->buffer_index < sizeof(console->input_buffer) - 1)
        {
            console->input_buffer[console->buffer_index++] = *rest;
            rest++;
        }
        console->input_buffer[console->buffer_index] = '\0';

        console_print_impl(match->name + token_len);
        return;
    }

    // Multiple matches: show candidates
    console_print_impl("\nPossible matches:\n");

    if (help_matches)
    {
        console_print_impl("  help\n");
    }

    // Show user-defined commands that match
    for (uint8_t i = 0; i < console->num_commands; i++)
    {
        if (strncmp(console->commands[i].name, token, token_len) == 0)
        {
            char line[80];
            snprintf(line, sizeof(line), "  %s\n", console->commands[i].name);
            console_print_impl(line);
        }
    }
    console_print_impl("> ");
    console_print_impl(console->input_buffer);
}
#endif

/** =======================================================================
 *  Public API
 *  ========================================================================
 *  See `serial_console.h` for documentation.
 */

void serial_console_update(serial_console_t *console)
{
    int16_t c = console_getchar_impl();

    if (c == -1)
    {
        return;
    }

#ifdef SERIAL_CONSOLE_ENABLE_TAB_COMPLETION
    if (c == '\t')
    {
        handle_tab_completion(console);
        return;
    }
#endif

    if (c == '\r' || c == '\n')
    {
        if (console->buffer_index > 0)
        {
            console->input_buffer[console->buffer_index] = '\0';

            trim_string(console->input_buffer);

            console_print_impl("\n");

            char *command = console->input_buffer;
            char *args = strstr(console->input_buffer, " ");
            if (args)
            {
                *args = '\0';
                args++;
                trim_string(args);
            }

            if (strcmp(command, "help") == 0)
            {
                cmd_help(console);

#ifdef SERIAL_CONSOLE_INCLUDE_BANG_COMMAND
                save_last_command(command, args);
#endif
            }
#ifdef SERIAL_CONSOLE_INCLUDE_BANG_COMMAND
            else if (strcmp(command, "!") == 0)
            {
                cmd_bang(console, args);
            }
#endif

            else
            {
                bool found = false;
                for (uint8_t i = 0; i < console->num_commands; i++)
                {
                    if (strcmp(command, console->commands[i].name) == 0)
                    {
                        if (console->commands[i].callback)
                        {
                            console->commands[i].callback(args);
                        }
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    console_print_impl("Unknown command. Type 'help' for available commands.\n");
                }

#ifdef SERIAL_CONSOLE_INCLUDE_BANG_COMMAND
                if (found && strcmp(command, "!") != 0)
                {
                    save_last_command(command, args);
                }
#endif
            }

            console->buffer_index = 0;
            memset(console->input_buffer, 0, sizeof(console->input_buffer));
            console_print_impl("> ");
        }
        return;
    }

    if (c == '\b' || c == 127)
    {
        if (console->buffer_index > 0)
        {
            console->buffer_index--;
            console->input_buffer[console->buffer_index] = '\0';
            console_print_impl("\b \b"); // Backspace, space, backspace
        }
        return;
    }

    if (console->buffer_index < sizeof(console->input_buffer) - 1 && isprint((unsigned char)c))
    {
        console->input_buffer[console->buffer_index++] = c;
        char ch[2] = {(char)c, '\0'};
        console_print_impl(ch); // Echo character
    }
}

void console_print(const char *message)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "%s\n", message);
    console_print_impl(buf);
}

void console_print_raw(const char *message)
{
    console_print_impl(message);
}
