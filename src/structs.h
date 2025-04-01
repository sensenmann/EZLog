#ifndef EZ_LOG_STRUCTS_H
#define EZ_LOG_STRUCTS_H

/**
 * Log-Levels
 */
enum class Loglevel {
    ERROR = 0,
    WARN = 1,
    INFO = 2,
    DEBUG = 3,
    VERBOSE = 4
};


/**
 * Custom LoggingElement, which allows overwriting the default Logging-Configuration
 */
struct LoggingElement {
    String filter;
    Loglevel loglevel = Loglevel::WARN;
    std::vector<LoggingElement> subElements;

    LoggingElement(const String& _filter, Loglevel _loglevel) : filter(_filter), loglevel(_loglevel) {};

    LoggingElement(const String& _filter, const std::vector<LoggingElement>& _subElements) : filter(_filter),
        subElements(_subElements) {
    };
};

/**
 * EZLog Logging-Configuration
 */
struct LoggingConfig {
    // Completely enable/disables Log-Output of EZLog
    bool enabled = true;

    // Maximum Loglevel (Default) which will be printed.
    // Can be overwritten for custom LoggingElement later:
    Loglevel loglevel = Loglevel::DEBUG;

    // Adds Information of free/max Memory to each Output:
    bool addMemInfo = false;

    // Ignores custom LoggingElements and Logs everything:
    bool overrideLogAll = false;

    // Shows a Message, when a Method starts and ends.
    // If a lot of start- and stop-messages without real logging messages are shown, disabling this option can be useful
    bool printStartEndMessages = true;

    // Restarts the ESP, if a Log::error() has been executed.
    // This can make sense on an production environment, if you want to reboot the ESP32, rather than looping endlessly
    bool restartESPonError = false;

    // Custom-Warn/Error Callback-Functions for Warning/Error-Actions.
    // Can be used to show something on a TFT, end the whole process with a while(true); or somehting else
    std::function<void(int taskID, String msg)> customErrorAction = [](const int taskID, const String& msg) {};
    std::function<void(int taskID, String msg)> customWarningAction = [](const int taskID, const String& msg) {};
    std::function<void(int taskID, String msg)> customInfoAction = [](const int taskID, const String& msg) {};
    std::function<void(int taskID, String msg)> customDebugAction = [](const int taskID, const String& msg) {};
    std::function<void(int taskID, String msg)> customVerboseAction = [](const int taskID, const String& msg) {};

    // Custom LoggingElement-Configurations can be used, to override the default logLevel for matching messages.
    // Example: You can set the default-logLevel to DEBUG, but teh loglevel vor alle Methods from class "xyz" to VERBOSE
    std::vector<LoggingElement> customLoggingElements;
};


#endif // EZ_LOG_STRUCTS_H
