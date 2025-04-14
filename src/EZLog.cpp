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
 * Gets an EZLog* Instance for the actual Task.
 * This is necessary, if there are more than one task (multiple Cores/ multiple Tasks) using EZLog.
 */
EZLog* EZLog::getInstanceForCurrentTask() {
    TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();

    static std::map<TaskHandle_t, EZLog*> logInstances;
    static std::mutex logMutex;

    // locking the map, while accessing
    std::lock_guard<std::mutex> guard(logMutex);

    // create new EZLog-Instance, if not yet existing:
    if (logInstances.find(currentTask) == logInstances.end()) {
        lastTaskID++;
        logInstances[currentTask] = new EZLog(lastTaskID);
    }
    return logInstances[currentTask];
}


/**
 * Marks the start of a function in your code.
 * Will be called by EZ_LOG() / EZ_LOG_STATIC() automatically.
 *
 * !! DON'T CALL IT DIRECTLY !!
 */
bool EZLog::start(const String& cls, const String& method) {
#ifndef EZLOG_DISABLE_COMPLETELY
    EZLog* instance = getInstanceForCurrentTask();
    return instance->_start(cls, method);
#endif
    return false;
}

/**
 * Marks the end of a function in your code.
 * Will be called by EZ_LOG() / EZ_LOG_STATIC() automatically.
 *
 * !! DON'T CALL IT DIRECTLY !!
 */
void EZLog::end() {
#ifndef EZLOG_DISABLE_COMPLETELY
    EZLog* instance = getInstanceForCurrentTask();
    instance->_end();
#endif
}


/** ***************************************
 *
 *          PUBLIC LOGGING METHODS
 *
 *************************************** */
void EZLog::error(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
    getInstanceForCurrentTask()->_msg(Loglevel::ERROR, msg);
#endif
}

void EZLog::errorln(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
    getInstanceForCurrentTask()->_errorln(msg);
#endif
}

void EZLog::warn(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= WARN
    getInstanceForCurrentTask()->_warn(msg);
#endif
#endif
}

void EZLog::warnln(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= WARN
    getInstanceForCurrentTask()->_warnln(msg);
#endif
#endif
}

void EZLog::info(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 2
    getInstanceForCurrentTask()->_info(msg);
#endif
#endif
}

void EZLog::infoln(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 2
    getInstanceForCurrentTask()->_infoln(msg);
#endif
#endif
}

void EZLog::debug(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 3
    getInstanceForCurrentTask()->_debug(msg);
#endif
#endif
}

void EZLog::debugln(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 3
    getInstanceForCurrentTask()->_debugln(msg);
#endif
#endif
}

void EZLog::verbose(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 4
    getInstanceForCurrentTask()->_verbose(msg);
#endif
#endif
}

void EZLog::verboseln(const String& msg) {
#ifndef EZLOG_DISABLE_COMPLETELY
#if EZLOG_MAX_LOG_LEVEL >= 4
    getInstanceForCurrentTask()->_verboseln(msg);
#endif
#endif
}


/**
 * Prints the actual memory-usage in kilobytes (in bytes if inBytes=true)
 * Allows a custom prefix being printed before
 */
void EZLog::freeMem(const String& prefix, bool inBytes) {
    getInstanceForCurrentTask()->_freeMem(prefix, inBytes);
}


/** ***************************************
 *
 *          PRIVATE LOGGING METHODS
 *
 *************************************** */


bool EZLog::_start(const String& cls, const String& method) {
    if (!config.enabled) return false;

    if (xSemaphoreTake(logSemaphoreStartStop, 1000 / portTICK_PERIOD_MS) != pdTRUE) {
        Serial.println("Log-Semaphore not available!");
        esp_backtrace_print(30);
        return false;
    }

    classStack.push(cls);
    methodStack.push(method);
    startTimeStack.push(millis());
    String prefix = cls + "::" + method;

    if (!_shouldLog(prefix, Loglevel::DEBUG)) {
        lastPrefix = prefix;
        xSemaphoreGive(logSemaphoreStartStop);
        return true;
    }

    newLineStarted = true;
    lastPrefix = prefix;
    if (config.printStartEndMessages) _msg(Loglevel::DEBUG, "\n", true);
    depth++;

    xSemaphoreGive(logSemaphoreStartStop);
    return true;
}

void EZLog::_end() {
    if (!config.enabled) return;

    if (xSemaphoreTake(logSemaphoreStartStop, 1000 / portTICK_PERIOD_MS) != pdTRUE) {
        Serial.println("Log-Semaphore not available!");
        esp_backtrace_print(30);
        while (true) {
            delay(1);
        }
    }

    if (classStack.empty() || methodStack.empty()) {
        // should not happen....
        Serial.println("ERROR - Log::_end() without Log::_start() !!");
        if (Serial) Serial.flush();
        esp_backtrace_print(30);
        delay(1000);

        xSemaphoreGive(logSemaphoreStartStop);
        return;
    }
    String cls = classStack.top();
    String method = methodStack.top();
    unsigned long startTime = startTimeStack.top();
    String prefix = cls + "::" + method;
    classStack.pop();
    methodStack.pop();
    startTimeStack.pop();

    if (prefix.length() == 0 || !_shouldLog(prefix, Loglevel::DEBUG)) {
        if (!classStack.empty() && !methodStack.empty()) {
            cls = classStack.top();
            method = methodStack.top();
            lastPrefix = cls + "::" + method;
        }
        xSemaphoreGive(logSemaphoreStartStop);
        return;
    }
    depth--;
    if (depth < 0) {
        warnln("ERROR - depth < 0:  " + String(depth));
    }

    if (!newLineStarted) {
        newLineStarted = true;
    }

    String durationMsg = ANSICOLOR_RESET + " " + ANSICOLOR_BRIGHT_BLACK + " (" + String(millis() - startTime) + "ms)" +
        ANSICOLOR_RESET;

    lastPrefix = prefix;
    if (config.printStartEndMessages) _msg(Loglevel::DEBUG, durationMsg + "\n", false, true);
    lastPrefix = "";

    if (!classStack.empty() && !methodStack.empty()) {
        cls = classStack.top();
        method = methodStack.top();
        lastPrefix = cls + "::" + method;
    } else {
        lastPrefix = "";
    }
    xSemaphoreGive(logSemaphoreStartStop);
}

void EZLog::_errorln(const String& msg) {
    config.customErrorAction(taskID, msg);
    error(msg + "\n");
    if (config.restartESPonError) abort();
}

void EZLog::_warn(const String& msg) {
    _msg(Loglevel::WARN, msg);
}

void EZLog::_warnln(const String& msg) {
    config.customWarningAction(taskID, msg);
    _msg(Loglevel::WARN, msg + "\n");
}

void EZLog::_info(const String& msg) {
    _msg(Loglevel::INFO, msg);
}

void EZLog::_infoln(const String& msg) {
    config.customInfoAction(taskID, msg);
    _msg(Loglevel::INFO, msg + "\n");
}

void EZLog::_debug(const String& msg) {
    _msg(Loglevel::DEBUG, msg);
}

void EZLog::_debugln(const String& msg) {
    config.customDebugAction(taskID, msg);
    _msg(Loglevel::DEBUG, msg + "\n");
}

void EZLog::_verbose(const String& msg) {
    _msg(Loglevel::VERBOSE, msg);
}

void EZLog::_verboseln(const String& msg) {
    config.customVerboseAction(taskID, msg);
    _msg(Loglevel::VERBOSE, msg + "\n");
}

String EZLog::_colorPrefix(String prefix, boolean isStart, boolean isEnd) {
    if (prefix == "") return prefix;

    String startStopPrefix;
    String startStopSuffix;
    if (isStart) {
        startStopPrefix = " ++ ";
        // startStopPrefix = "[START] ";
        startStopSuffix = " - [START]";
    } else if (isEnd) {
        startStopPrefix = " -- ";
        // startStopPrefix = "[END] ";
        startStopSuffix = " - [END]";
    }

    const auto result = split(prefix.c_str(), "::");
    if (result.empty() || result.size() < 2) {
        _errorln("ERROR!! Result empty: " + String(prefix.c_str()));
    }
    const String& cls = result[0];
    const String& method = result[1];

    String retVal = ANSICOLOR_YELLOW + startStopPrefix + ansiColorReset()
        + ANSICOLOR_GREEN + cls + ansiColorReset() + "::" + ANSICOLOR_BLUE + method
        + ANSICOLOR_YELLOW + startStopSuffix
        + ansiColorReset();

    return retVal;
}

String EZLog::getBGColor() const {
    if (taskID == 1) return TaskIdBGColors[0];

    // Rolliere ab Index 1 (also ab dem 2. Eintrag)
    const int numColors = TaskIdBGColors.size() - 1;
    int colorIdx = ((taskID - 2) % numColors) + 1;
    return TaskIdBGColors[colorIdx];
}

void EZLog::_msg(Loglevel loglevel, String msg, const boolean isStart, const boolean isEnd) {
    if (!config.enabled) return;

    if (lastPrefix.equals("")) {
        String errorMsg = "EZLog ERROR: Log-Aufruf ohne gültigen Prefix (kein start() erfolgt?)";
        config.customErrorAction(taskID, errorMsg);
        Serial.println(errorMsg);
        esp_backtrace_print(30);
        if (config.restartESPonError) {
            abort();
        }
    }

    /** Handling Mutliline-Messages */
    char crlf = '\n';
    size_t occurrences = std::count(msg.begin(), msg.end(), crlf);

    if (occurrences > 1) {
        std::istringstream tokenStream(msg.c_str());
        std::string msgPart;
        while (std::getline(tokenStream, msgPart, '\n')) {
            if (!msgPart.empty() && _shouldLog(loglevel)) {
                // filling multiLineBuffer:
                _msg(loglevel, String(msgPart.c_str()) + '\n', isStart, isEnd);

                newLineStarted = true;
            }
        }

        lastloglevel = loglevel;
        return;
    }

    if (occurrences == 0) {
        multilineBuffer += msg;
        lastloglevel = loglevel;
        return;
    }

    // occurrences == 1:
    if (loglevel != lastloglevel) {
        if (!newLineStarted) {
            newLineStarted = true;
        }
        // Der sollte eigentlich eh leer sein, trotzdem...
        multilineBuffer = "";
    }

    if (xSemaphoreTake(logSemaphoreMessage, 1000 / portTICK_PERIOD_MS) != pdTRUE) {
        Serial.println("Log-Semaphore not available!");
        esp_backtrace_print(30);
        while (true) {
            delay(1);
        }
    }

    const bool shouldLog = _shouldLog(loglevel) || loglevel <= Loglevel::WARN;

    if (newLineStarted) {
        /**
         * [TaskID] Timestamp [Loglevel] <<indent>>
         */
        if (shouldLog) {
            Serial.print(ANSICOLOR_RESET); // Reset everything

            // TaskID - Text-Color:
            int colorIdx = taskID <= TaskIdColors.size() ? taskID - 1 : 0;
            Serial.print("[" + TaskIdColors[colorIdx] + String(taskID) + "] ");

            // Timestamp:
            Serial.print(ansiColorReset() + ts());

            Serial.print(loglevelPrefixColors[(int)loglevel]);
            Serial.print(loglevelStrings[(int)loglevel]);
            Serial.print(ansiColorReset());

            int indent = std::min(std::max(depth * 4, 0), 80);
            Serial.print(std::string(indent, ' ').c_str());
        }

        newLineStarted = false;

        /**
         * [START] / [END] (optional)
         */
        if (shouldLog) {
            if (!lastPrefix.equals("")) {
                Serial.print(_colorPrefix(lastPrefix, isStart, isEnd));

                if (!msg.equals("") && !isStart && !isEnd) {
                    Serial.print(": ");
                }
            }

            if (isStart || isEnd) {
                newLineStarted = true;
            }
        }
    }

    /**
     * [FREE MEM] (optional)
     */
    _addFreeMemToMessage();

    /**
     * MULTILINEBUFFER (= MSG)
     */
    if (shouldLog) {
        Serial.print(ansiColorReset());
        Serial.print(multilineBuffer);
        Serial.print(ansiColorReset());
        Serial.print(loglevelTextColors[(int)loglevel]);

        if (msg.endsWith("\n")) {
            Serial.print(msg.substring(0, msg.length() - 1) + ANSICOLOR_RESET + "\n");
            newLineStarted = true;
        } else {
            Serial.print(msg);
        }
    }
    multilineBuffer = "";

    lastloglevel = loglevel;

    xSemaphoreGive(logSemaphoreMessage);
}

void EZLog::_addFreeMemToMessage() {
    if (!config.addMemInfo) return;

    int freeMem = esp_get_free_heap_size();
    // int freeMemKB = freeMem / 1024;
    int largestFreeBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    int largestFreeBlockKB = largestFreeBlock / 1024;
    // UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);

    multilineBuffer =
        ANSICOLOR_BRIGHT_YELLOW + " [ " +
        ANSICOLOR_WHITE + String(formatNumber(heap_caps_get_free_size(MALLOC_CAP_DMA) / 1024)) +
        " kB " +
        //            ANSICOLOR_BRIGHT_YELLOW + "/ " +
        //            ANSICOLOR_WHITE + String(formatNumber(freeMemKB)) + " kB " +
        ANSICOLOR_BRIGHT_YELLOW + "/ " +
        ANSICOLOR_WHITE + String(formatNumber(largestFreeBlockKB)) + " kB " +
        //            ANSICOLOR_BRIGHT_YELLOW + "/ " +
        //            ANSICOLOR_WHITE + String(stackHighWaterMark)  +
        ANSICOLOR_BRIGHT_YELLOW + " ] " +
        multilineBuffer;
}

bool EZLog::_shouldLog(const String& prefix, const Loglevel requestedLoglevel) {
    if (config.overrideLogAll) return true;

    // Hilfsfunktion, die entweder nullptr oder einen Zeiger auf ein bool zurückgibt
    std::function<const bool*(const std::vector<LoggingElement>&)> searchLoggingElements;
    searchLoggingElements = [&](const std::vector<LoggingElement>& elements) -> const bool* {
        for (const auto& elem : elements) {
            std::string searchPrefix = prefix.c_str();
            std::string name = elem.filter.c_str();

            if (searchPrefix.find(name) == 0) {
                // In Logging-Config gefunden: Ergebnis speichern
                static bool result;
                result = (requestedLoglevel <= elem.loglevel);
                return &result;
            }

            // Rekursiv in Unterelementen suchen
            if (!elem.subElements.empty()) {
                const bool* result = searchLoggingElements(elem.subElements);
                if (result != nullptr) {
                    return result;
                }
            }
        }
        return nullptr; // Kein Eintrag gefunden
    };

    const bool* found = searchLoggingElements(config.customLoggingElements);
    if (found != nullptr) {
        return *found;
    }

    // Fallback auf Default-Loglevel
    return requestedLoglevel <= config.loglevel;
}


bool EZLog::_shouldLog(const Loglevel loglevel) const {
    if (!lastPrefix.equals("")) {
        return _shouldLog(lastPrefix, loglevel);
    }
    return true;
}


void EZLog::_freeMem(const String& prefix, const bool inBytes) {
    if (!_shouldLog(prefix, Loglevel::DEBUG)) return;

    const int freePSRam = esp_get_free_heap_size() * (inBytes ? 1 : 1.0 / 1024.0);
    const int freeHeap = heap_caps_get_free_size(MALLOC_CAP_DMA) * (inBytes ? 1 : 1.0 / 1024.0);
    UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(nullptr);
    const String unit = inBytes ? "B" : "kB";
    const int largestFreeBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) * (inBytes ? 1 : 1.0 / 1024.0);
    //    if (!newLineStarted) {
    Serial.println();
    //        newLineStarted = true;
    //    }
    if (!prefix.equals("")) {
        Serial.print(String(prefix));
        Serial.print(std::string((strlen(prefix.c_str()) < 40 ? 40 - strlen(prefix.c_str()) : 0), ' ').c_str());
    }
    Serial.print("- Free Mem: " + ANSICOLOR_GREEN + formatNumber(freeHeap) + " " + unit + ANSICOLOR_RESET);
    // Serial.print(" - Free Mem: " + ANSICOLOR_GREEN + formatNumber(freePSRam) + " " + unit + ANSICOLOR_WHITE);
    Serial.print(",\tPSRAM: " + ANSICOLOR_GREEN + formatNumber(largestFreeBlock) + " " + unit +
        ANSICOLOR_WHITE);
    Serial.print(",\tDelta Heap: " + (String(freeHeap - lastMemoryUsageHeap)) + " " + unit);
    Serial.print(",\tDelta PSRAM: " + (String(freePSRam - lastMemoryUsagePSRam)) + " " + unit);
    Serial.print(",\tFree Stack: " + ANSICOLOR_GREEN + String(stackHighWaterMark) + ANSICOLOR_RESET);

    Serial.println();

    lastMemoryUsageHeap = freeHeap;
    lastMemoryUsagePSRam = freePSRam;
}

void EZLog::_freeMem() {
    _freeMem("");
}

String EZLog::ts() {
    unsigned long millisVal = millis();
    unsigned long hours = millisVal / 3600000;
    unsigned long minutes = (millisVal % 3600000) / 60000;
    unsigned long seconds = (millisVal % 60000) / 1000;
    unsigned long milliseconds = millisVal % 1000;

    char buffer[13]; // Platz für "HH:MM:SS.mmm"
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu.%03lu", hours, minutes, seconds, milliseconds);

    return ANSICOLOR_WHITE + buffer + " " + ansiColorReset();
}

String EZLog::ansiColorReset() {
    return ANSICOLOR_RESET + getBGColor();
}

// Funktion zum Aufteilen eines Strings anhand eines Delimiters
std::vector<std::string> EZLog::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(s);

    while (std::getline(ss, token, delimiter)) {
        tokens.emplace_back(trim(token).c_str());
    }

    return tokens;
}

std::vector<char*> EZLog::split(char* str, char delimiter) {
    std::vector<char*> result;
    char* start = str;
    char* ptr = str;

    while (*ptr) {
        if (*ptr == delimiter) {
            *ptr = '\0';
            result.push_back(start);
            start = ptr + 1;
        }
        ptr++;
    }
    result.push_back(start);
    return result;
}

std::vector<String> EZLog::split(const std::string& input, const std::string& delimiter) {
    std::vector<String> tokens;
    size_t start = 0, end;

    while ((end = input.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(input.substr(start, end - start).c_str());
        start = end + delimiter.length();
    }

    tokens.push_back(input.substr(start).c_str());
    return tokens;
}

String EZLog::ltrim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    return String((start == std::string::npos) ? "" : s.substr(start).c_str());
}

String EZLog::rtrim(const std::string& s) {
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    return {(end == std::string::npos) ? "" : s.substr(0, end + 1).c_str()};
}

String EZLog::trim(const std::string& s) {
    return rtrim(ltrim(s).c_str());
}

String EZLog::formatNumber(const int number) {
    std::stringstream ss;
    ss << number;
    const std::string numberStr = ss.str();

    const int len = numberStr.length();
    const int numCommas = (len - 1) / 3;
    if (numCommas == 0)
        return numberStr.c_str();

    std::string formattedNumber;
    int i = 0;
    const int firstGroupSize = len % 3 == 0 ? 3 : len % 3;
    for (; i < firstGroupSize; ++i) {
        formattedNumber += numberStr[i];
    }
    if (firstGroupSize != len)
        formattedNumber += '.';

    int groupCount = 0;
    for (; i < len; ++i) {
        formattedNumber += numberStr[i];
        ++groupCount;
        if (groupCount == 3 && i != len - 1) {
            formattedNumber += '.';
            groupCount = 0;
        }
    }
    return formattedNumber.c_str();
}


String EZLog::loglevelStrings[] = {
    "[ERROR]   ",
    "[WARN]    ",
    "[INFO]    ",
    "[DEBUG]   ",
    "[VERBOSE] "
};

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


std::vector<String> EZLog::TaskIdColors = {
    ANSICOLOR_WHITE,
    ANSICOLOR_BLUE,
    ANSICOLOR_YELLOW,
    ANSICOLOR_GREEN,
};

std::vector<String> EZLog::TaskIdBGColors = {
    ANSICOLOR_RESET,
    ANSICOLOR_BG_BRIGHT_BLACK,
    ANSICOLOR_BG_CYAN,
    ANSICOLOR_BG_YELLOW,
};


String EZLog::loglevelPrefixColors[] = {
    ANSICOLOR_RED, // 1=bold, 31=red
    ANSICOLOR_BRIGHT_MAGENTA,
    ANSICOLOR_GREEN,
    ANSICOLOR_WHITE,
    ANSICOLOR_BRIGHT_BLACK,
};

String EZLog::loglevelTextColors[] = {
    ANSICOLOR_RED,
    ANSICOLOR_RED, // ANSICOLOR_BLACK + ANSICOLOR_BG_BRIGHT_YELLOW,
    ANSICOLOR_GREEN,
    ANSICOLOR_WHITE, // debug
    ANSICOLOR_BRIGHT_BLACK,
};

int EZLog::lastMemoryUsageHeap = 0;
int EZLog::lastMemoryUsagePSRam = 0;
LoggingConfig EZLog::config;
int EZLog::lastTaskID = 0;
SemaphoreHandle_t EZLog::logSemaphoreStartStop = xSemaphoreCreateMutex();
SemaphoreHandle_t EZLog::logSemaphoreMessage = xSemaphoreCreateMutex();
