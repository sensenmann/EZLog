#include "EZLog.h"
#include <sstream>
#include <esp_debug_helpers.h>
#include <map>
#include <mutex>

/**
 * Sets LoggingConfig
 */
void EZLog::init(const LoggingConfig& _loggingConfig) {
    config = _loggingConfig;
}

/**
 * Updates LoggingConfig
 */
void EZLog::updateConfig(const LoggingConfig& _loggingConfig) {
    config = _loggingConfig;
}


/**
 * Marks the start of a function in your code.
 * Will be called by EZ_LOG() / EZ_LOG_STATIC() automatically.
 *
 * !! DON'T CALL IT DIRECTLY !!
 */
bool EZLog::start(const String& cls, const String& method) {
    return true;
}

/**
 * Marks the end of a function in your code.
 * Will be called by EZ_LOG() / EZ_LOG_STATIC() automatically.
 *
 * !! DON'T CALL IT DIRECTLY !!
 */
void EZLog::end() {

}


/** ***************************************
 *
 *          PUBLIC LOGGING METHODS
 *
 *************************************** */
void EZLog::error(const String& msg) {
    Serial.print(msg);
}

void EZLog::errorln(const String& msg) {
    Serial.println(msg);
}

void EZLog::warn(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= WARN
    Serial.print(msg);
#endif
#endif
}

void EZLog::warnln(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= WARN
    Serial.println(msg);
#endif
#endif
}

void EZLog::info(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 2
    Serial.print(msg);
#endif
#endif
}

void EZLog::infoln(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 2
    Serial.println(msg);
#endif
#endif
}

void EZLog::debug(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 3
    Serial.print(msg);
#endif
#endif
}

void EZLog::debugln(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 3
    Serial.println(msg);
#endif
#endif
}

void EZLog::verbose(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 4
    Serial.print(msg);
#endif
#endif
}

void EZLog::verboseln(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 4
    Serial.println(msg);
#endif
#endif
}


/**
 * Prints the actual memory-usage in kilobytes (in bytes if inBytes=true)
 * Allows a custom prefix being printed before
 */
void EZLog::freeMem(const String& prefix, bool inBytes) {

}

// https://i.stack.imgur.com/9UVnC.png
#ifndef EZLOG_DISABLE_COLORS
    String EZLog::ANSICOLOR_RESET = "\033[0m";
    String EZLog::ANSICOLOR_RED = "\033[1;31m";
    String EZLog::ANSICOLOR_GREEN = "\033[1;32m";
    String EZLog::ANSICOLOR_YELLOW = "\033[1;33m";
    String EZLog::ANSICOLOR_WHITE = "\033[1;37m";
    String EZLog::ANSICOLOR_CYAN = "\033[1;36m";
    String EZLog::ANSICOLOR_BLUE = "\033[1;34m";
    String EZLog::ANSICOLOR_BLACK = "\033[1;30m";
    String EZLog::ANSICOLOR_BRIGHT_MAGENTA = "\033[1;95m";
    String EZLog::ANSICOLOR_BRIGHT_YELLOW = "\033[1;93m";
    String EZLog::ANSICOLOR_BRIGHT_BLACK = "\033[1;90m";
    String EZLog::ANSICOLOR_BRIGHT_RED = "\033[1;91m";
    String EZLog::ANSICOLOR_BG_BRIGHT_YELLOW = "\033[1;103m";
    String EZLog::ANSICOLOR_BG_GREEN = "\033[1;102m";
    String EZLog::ANSICOLOR_BG_RED = "\033[1;101m";
    String EZLog::ANSICOLOR_BG_CYAN = "\033[1;46m";
    String EZLog::ANSICOLOR_BG_YELLOW = "\033[1;43m";
    String EZLog::ANSICOLOR_BG_BRIGHT_BLACK = "\033[1;100m";
    String EZLog::ANSICOLOR_BG_BRIGHT_MAGENTA = "\033[1;105m";
#else
    String EZLog::ANSICOLOR_RESET =             "";
    String EZLog::ANSICOLOR_RED =               "";
    String EZLog::ANSICOLOR_GREEN =             "";
    String EZLog::ANSICOLOR_YELLOW =            "";
    String EZLog::ANSICOLOR_WHITE =             "";
    String EZLog::ANSICOLOR_CYAN =              "";
    String EZLog::ANSICOLOR_BLUE =              "";
    String EZLog::ANSICOLOR_BLACK =             "";
    String EZLog::ANSICOLOR_BRIGHT_MAGENTA =    "";
    String EZLog::ANSICOLOR_BRIGHT_YELLOW =     "";
    String EZLog::ANSICOLOR_BRIGHT_BLACK =      "";
    String EZLog::ANSICOLOR_BRIGHT_RED =        "";
    String EZLog::ANSICOLOR_BG_BRIGHT_YELLOW =  "";
    String EZLog::ANSICOLOR_BG_GREEN =          "";
    String EZLog::ANSICOLOR_BG_RED =            "";
    String EZLog::ANSICOLOR_BG_CYAN =           "";
    String EZLog::ANSICOLOR_BG_YELLOW =         "";
    String EZLog::ANSICOLOR_BG_BRIGHT_BLACK =   "";
    String EZLog::ANSICOLOR_BG_BRIGHT_MAGENTA = "";
#endif





LoggingConfig EZLog::config;

