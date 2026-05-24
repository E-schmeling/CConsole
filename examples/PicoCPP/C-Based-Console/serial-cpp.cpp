/**
 * @file serial-cpp.cpp
 * @brief Arduino-specific implementations of serial console platform functions.
 * @author ERS
 *
 * This C++ file provides the platform-specific implementations for the serial
 * console using Arduino's Serial interface. It bridges C code with C++ calls.
 */

#include <Arduino.h>
#include <cstdint>
#include <cstring>

extern "C"
{
    /**
     * @internal
     * @brief Low-level output routine using Arduino Serial.
     *
     * Sends a nul-terminated string to the Arduino Serial interface.
     *
     * @param text Nul-terminated string to send.
     */
    void console_print_impl(const char *text)
    {
        Serial.print(text);
    }

    /**
     * @internal
     * @brief Non-blocking input character provider using Arduino Serial.
     *
     * Checks if a character is available on Serial and returns it, or -1
     * if no character is currently available.
     *
     * @return Next input character (0–255), or -1 if none available.
     */
    int16_t console_getchar_impl(void)
    {
        if (Serial.available() > 0) {
            return Serial.read();
        }
        return -1;
    }

    /**
     * @internal
     * @brief Initialize Serial for the console.
     *
     * Sets up the Arduino Serial interface at 115200 baud.
     */
    void device_specific_init(void)
    {
        Serial.begin(115200);
    }
}