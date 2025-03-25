#ifndef EZ_LOG_H
#define EZ_LOG_H

#include <Arduino.h>
#include <stack>
#include <vector>
#include <string>
#include "Loggable.h"

enum class Loglevel {
    ERROR = 0,
    WARN = 1,
    INFO = 2,
    DEBUG = 3,
    VERBOSE = 4
};

#ifndef MAX_LOG_LEVEL
    #define MAX_LOG_LEVEL       4
#endif


struct LoggingElement {
    String filter;
    Loglevel loglevel = Loglevel::WARN;
    std::vector<LoggingElement> subElements;

    LoggingElement(const String& filter, Loglevel loglevel) : filter(filter), loglevel(loglevel) {
    };

    LoggingElement(const String& filter, const std::vector<LoggingElement>& subElements) : filter(filter),
        subElements(subElements) {
    };
};

struct LoggingConfig {
    bool enabled = true;
    bool addMemInfo = false;
    bool overrideLogAll = false;
    bool printStartEndMessages = true;

    bool restartESPonError = false;
    bool enableFreeMem = true; // Aktiviert/Deaktiviert funktion  Log::freeMem()  global
    Loglevel loglevel = Loglevel::DEBUG;

    // Custom-Warn/Error Callback-Functions (z.B. auf dem TFT ausgeben)
    std::function<void(String warnMsg)> customWarningAction = [](String warnMsg) {};
    std::function<void(String errorMsg)> customErrorAction = [](String errorMsg) {};

    std::vector<LoggingElement> elements;
};

class Log {
public:
    Log(int taskID) { this->taskID = taskID; }

    static String loglevelStrings[];

    static std::array<String, 4> TaskIdColors;
    static std::array<String, 3> TaskIdBGColors;

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
    static Log* getInstanceForCurrentTask();

public:
    // Init:
    static void init(LoggingConfig& _loggingConfig);
    static void updateConfig(LoggingConfig _loggingConfig);


    // Funktion zum Serialisieren der LoggingConfig in JSON
    static String serializeLoggingConfig(const LoggingConfig& config);
    // Funktion zum Deserialisieren der LoggingConfig aus JSON
    static void deserializeLoggingConfig(const String& json, LoggingConfig& config);


    static void start(String cls, String method);
    static void end();

    static void error(String msg);
    static void errorln(const String& msg = "");

    static void warn(String msg);
    static void warnln(String msg = "");

    static void info(String msg);
    static void infoln(String msg = "");

    static void debug(String msg);
    static void debugln(String msg = "");

    static void verbose(String msg);
    static void verboseln(String msg = "");

    static void freeMem();
    static void freeMem(String prefix, bool inBytes = false);

private:
    void _start(String cls, String method);
    void _end();

    void _msg(Loglevel loglevel, String msg, boolean isStart = false, boolean isEnd = false);

    void _errorln(const String& msg = "");

    void _warn(String msg);
    void _warnln(String msg = "");

    void _info(String msg);
    void _infoln(String msg = "");

    void _debug(String msg);
    void _debugln(String msg = "");

    void _verbose(String msg);
    void _verboseln(String msg = "");

    void _freeMem(String prefix, bool inBytes = false);
    void _freeMem();

    String _colorPrefix(String prefix, boolean isStart = false, boolean isEnd = false);
    void _addFreeMemToMessage();
    bool _shouldLog(String prefix, Loglevel requestedLoglevel);
    bool _shouldLog(Loglevel loglevel);

    String ts();
    String getBGColor();
    String ansiColorReset();

    std::vector<std::string> split(const std::string& s, char delimiter);
    std::vector<char*> split(char* str, char delimiter);
    std::vector<String> split(const std::string& input, const std::string& delimiter);
    String ltrim(const std::string& s);
    String rtrim(const std::string& s);
    String trim(const std::string& s);
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
    static String ANSICOLOR_BG_YELLOW_BRIGHT;
    static String ANSICOLOR_BG_GREEN;
    static String ANSICOLOR_BG_RED;
    static String ANSICOLOR_BG_CYAN;
};

#endif  // EZ_LOG_H
