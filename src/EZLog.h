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
    /**
     * Creates a EZLog-Instance for a taskID (usefull for multiple Threads)
     */
    EZLog(int _taskID) { this->taskID = _taskID; }

    static String loglevelStrings[];

    static std::vector<String> TaskIdColors;
    static std::vector<String> TaskIdBGColors;

    static String loglevelPrefixColors[];
    static String loglevelTextColors[];

private:
    /** Singleton Werte (für alle Instanzen): */
    static LoggingConfig config;
    static int lastMemoryUsageHeap;
    static int lastMemoryUsagePSRam;
    static int lastTaskID;
    static SemaphoreHandle_t logSemaphoreStartStop;
    static SemaphoreHandle_t logSemaphoreMessage;

private:;
    int taskID = 0;
    int depth = 0;
    bool newLineStarted = true;
    std::stack<String> classStack;
    std::stack<String> methodStack;
    std::stack<unsigned long> startTimeStack;
    String lastPrefix = "";
    String multilineBuffer = "";
    Loglevel lastloglevel = Loglevel::ERROR;

private:
    static EZLog* getInstanceForCurrentTask();

public:
    // Init:
    static void init(const LoggingConfig& _loggingConfig);
    static void updateConfig(const LoggingConfig& _loggingConfig);


    static void start(const String& cls, const String& method);
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
    void _start(const String& cls, const String& method);
    void _end();

    void _msg(Loglevel loglevel, String msg, boolean isStart = false, boolean isEnd = false);

    void _errorln(const String& msg = "");

    void _warn(const String& msg);
    void _warnln(const String& msg = "");

    void _info(const String& msg);
    void _infoln(const String& msg = "");

    void _debug(const String& msg);
    void _debugln(const String& msg = "");

    void _verbose(const String& msg);
    void _verboseln(const String& msg = "");

    static void _freeMem(const String& prefix, bool inBytes = false);
    static void _freeMem();

    String _colorPrefix(String prefix, boolean isStart = false, boolean isEnd = false);
    void _addFreeMemToMessage();

    String getBGColor() const;
    String ansiColorReset();

    static bool _shouldLog(const String& prefix, Loglevel requestedLoglevel);
    bool _shouldLog(Loglevel loglevel) const;

    /** Timestamp-Prefix: */
    String ts();


    /** String-Tools: */
    static std::vector<std::string> split(const std::string& s, char delimiter);
    static std::vector<char*> split(char* str, char delimiter);
    static std::vector<String> split(const std::string& input, const std::string& delimiter);
    static String ltrim(const std::string& s);
    static String rtrim(const std::string& s);
    static String trim(const std::string& s);

    /** Formatiert Ganz-Zahl mit tausender-Punkt(en) */
    static String formatNumber(int number);

public:
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
