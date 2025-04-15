#ifndef EZ_LOG_H
#define EZ_LOG_H

#include <Arduino.h>
#include <stack>
#include <vector>
#include <string>
#include "structs.h"
#include "Loggable.h"


/**
 * Compiler Directives:
 *    EZLOG_DISABLE_COLORS:      Disables colorful ANSI-Output, for IDEs like ArduinoIDE (default = not set)
 *    EZLOG_DISABLE_COMPLETELY:  Disables Logging completely, if defined  (default = not set)
 */


/**
 * Maximum Loglevel, which will be available. This should be set f.e. in platform.ini with
 * -D EZLOG_MAX_LOG_LEVEL=<value>, so the compiler knows what to do.
 * You can set this level to 2 (WARNING) on release-builds.
 */
#ifndef EZLOG_MAX_LOG_LEVEL
    #define EZLOG_MAX_LOG_LEVEL       4
#endif


class EZLog {

public:
    // Init:
    static void init(const LoggingConfig& _loggingConfig);
    static void updateConfig(const LoggingConfig& _loggingConfig);

    static bool start(const String& cls, const String& method);
    static void end();

    static void error(const String& msg);
    static void errorln(const String& msg = "");

    static void warn(const String& msg);
    static void warnln(const String& msg = "");

    static void info(const String& msg);
    static void infoln(const String& msg = "");

    static void debug(const String& msg);
    static void debugln(const String& msg = "");

    static void verbose(const String& msg);
    static void verboseln(const String& msg = "");

    static void freeMem(const String& prefix = "", bool inBytes = false);


private:
    void _msg(Loglevel loglevel, String msg, boolean isStart = false, boolean isEnd = false);


private:
    /** Singleton Werte (für alle Instanzen): */
    static LoggingConfig config;


public:
    static String loglevelStrings[];

    static String ANSICOLOR_RESET;
    static String ANSICOLOR_RED;
    static String ANSICOLOR_GREEN;
    static String ANSICOLOR_YELLOW;
    static String ANSICOLOR_WHITE;
    static String ANSICOLOR_CYAN;
    static String ANSICOLOR_BLUE;
    static String ANSICOLOR_BLACK;
    static String ANSICOLOR_BRIGHT_MAGENTA;
    static String ANSICOLOR_BRIGHT_YELLOW;
    static String ANSICOLOR_BRIGHT_BLACK;
    static String ANSICOLOR_BRIGHT_RED;
    static String ANSICOLOR_BG_BRIGHT_YELLOW;
    static String ANSICOLOR_BG_GREEN;
    static String ANSICOLOR_BG_RED;
    static String ANSICOLOR_BG_CYAN;
    static String ANSICOLOR_BG_YELLOW;
    static String ANSICOLOR_BG_BRIGHT_BLACK;
    static String ANSICOLOR_BG_BRIGHT_MAGENTA;
};


#define Log     EZLog


#endif  // EZ_LOG_H
