#include "EZLog.h"

#include <sstream>
#include <ArduinoJson.h>
#include <esp_debug_helpers.h>
#include <map>
#include <mutex>
#include <optional>

String Log::loglevelStrings[]  = {
    "[ERROR]   ",
    "[WARN]    ",
    "[INFO]    ",
    "[DEBUG]   ",
    "[VERBOSE] "
};

String Log::ANSICOLOR_RESET =    "\033[0m";
String Log::ANSICOLOR_RED =      "[1;31m";
String Log::ANSICOLOR_GREEN =    "[1;32m";
String Log::ANSICOLOR_YELLOW =   "[1;33m";
String Log::ANSICOLOR_WHITE =    "\033[1;37m";
String Log::ANSICOLOR_CYAN =    "\033[1;36m";
String Log::ANSICOLOR_BLUE =     "\033[1;34m";
String Log::ANSICOLOR_BLACK =     "\033[1;30m";
String Log::ANSICOLOR_BRIGHT_MAGENTA =   "[1;95m";
String Log::ANSICOLOR_BRIGHT_YELLOW =   "[1;93m";
String Log::ANSICOLOR_BRIGHT_BLACK =    "[1;90m";
String Log::ANSICOLOR_BRIGHT_RED =    "[1;91m";
String Log::ANSICOLOR_BG_YELLOW_BRIGHT =   "[1;103m";
String Log::ANSICOLOR_BG_GREEN =   "[1;102m";
String Log::ANSICOLOR_BG_RED =   "[1;101m";
String Log::ANSICOLOR_BG_CYAN =   "[1;46m";


std::array<String, 4> Log::TaskIdColors = {
        ANSICOLOR_WHITE,
        ANSICOLOR_BLUE,
        ANSICOLOR_YELLOW,
        ANSICOLOR_GREEN,
};

std::array<String, 3> Log::TaskIdBGColors = {
        ANSICOLOR_RESET,
        ANSICOLOR_BG_CYAN,
        ANSICOLOR_BG_RED,
};


// https://i.stack.imgur.com/9UVnC.png
String Log::loglevelPrefixColors[] = {
    ANSICOLOR_RED, // 1=bold, 31=red
    ANSICOLOR_BRIGHT_MAGENTA,
    ANSICOLOR_GREEN,
    ANSICOLOR_WHITE,
    ANSICOLOR_BRIGHT_BLACK,
};
String Log::loglevelTextColors[] = {
    ANSICOLOR_RED,
    ANSICOLOR_RED, // ANSICOLOR_BLACK + ANSICOLOR_BG_YELLOW_BRIGHT,
    ANSICOLOR_GREEN,
    ANSICOLOR_WHITE, // debug
    ANSICOLOR_BRIGHT_BLACK,
};
int Log::lastMemoryUsageHeap = 0;
int Log::lastMemoryUsagePSRam = 0;
LoggingConfig Log::config;
int Log::lastTaskID = 0;
SemaphoreHandle_t Log::logSemaphoreStartStop = xSemaphoreCreateMutex();
SemaphoreHandle_t Log::logSemaphoreMessage = xSemaphoreCreateMutex();

void Log::init(LoggingConfig& _loggingConfig) {
    Log::config = _loggingConfig;
}

void Log::updateConfig(LoggingConfig _loggingConfig) {
    Log::config = _loggingConfig;
}

String Log::serializeLoggingConfig(const LoggingConfig& config) {
    StaticJsonDocument<2048> doc;
    doc["enabled"] = config.enabled;
    doc["addMemInfo"] = config.addMemInfo;
    doc["overrideLogAll"] = config.overrideLogAll;
    doc["printStartEndMessages"] = config.printStartEndMessages;
    doc["restartESPonError"] = config.restartESPonError;
    doc["enableFreeMem"] = config.enableFreeMem;
    doc["loglevel"] = (int)config.loglevel;

    JsonArray elements = doc.createNestedArray("elements");

    std::function<void(const std::vector<LoggingElement>&, JsonArray&)> serializeElements;
    serializeElements = [&](const std::vector<LoggingElement>& elements, JsonArray& jsonArray) {
        for (const auto& elem : elements) {
            JsonObject obj = jsonArray.createNestedObject();
            obj["filter"] = elem.filter;
            obj["loglevel"] = (int)elem.loglevel;

            if (!elem.subElements.empty()) {
                JsonArray subArray = obj.createNestedArray("subElements");
                serializeElements(elem.subElements, subArray);
            }
        }
    };

    serializeElements(config.elements, elements);

    String output;
    serializeJson(doc, output);
    return output;
}

void Log::deserializeLoggingConfig(const String& json, LoggingConfig& config) {
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.println("Failed to parse JSON");
        return;
    }

    config.enabled = doc["enabled"].as<bool>();
    config.addMemInfo = doc["addMemInfo"].as<bool>();
    config.overrideLogAll = doc["overrideLogAll"].as<bool>();
    config.printStartEndMessages = doc["printStartEndMessages"].as<bool>();
    config.restartESPonError = doc["restartESPonError"].as<bool>();
    config.enableFreeMem = doc["enableFreeMem"].as<bool>();
    config.loglevel = static_cast<Loglevel>(doc["loglevel"].as<int>());

    config.elements.clear();

    std::function<void(JsonArray, std::vector<LoggingElement>&)> deserializeElements;
    deserializeElements = [&](JsonArray jsonArray, std::vector<LoggingElement>& elements) {
        for (JsonObject obj : jsonArray) {
            LoggingElement elem = LoggingElement(
                obj["filter"].as<String>(),
                static_cast<Loglevel>(obj["loglevel"].as<int>())
            );

            if (obj.containsKey("subElements")) {
                JsonArray subArray = obj["subElements"].as<JsonArray>();
                deserializeElements(subArray, elem.subElements);
            }

            elements.push_back(elem);
        }
    };

    deserializeElements(doc["elements"].as<JsonArray>(), config.elements);
}

Log* Log::getInstanceForCurrentTask() {
    // Ermittelt den aktuellen Task-Handle
    TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();

    // Statische Map für Task-spezifische Log-Instanzen
    static std::map<TaskHandle_t, Log*> logInstances;
    static std::map<TaskHandle_t, int> taskIDs;

    // Mutex zum Schutz der Map;
    static std::mutex logMutex;

    // Sperren der Map während des Zugriffs
    std::lock_guard<std::mutex> guard(logMutex);

    // Falls noch keine Log-Instanz für die aktuelle Task existiert, wird sie angelegt.
    if (logInstances.find(currentTask) == logInstances.end()) {
        lastTaskID++;
        logInstances[currentTask] = new Log(lastTaskID);
    }
    return logInstances[currentTask];
}

void Log::start(String cls, String method) {
    #ifndef DISABLE_ALL_LOGGING
    Log* instance = getInstanceForCurrentTask();
    instance->_start(cls, method);
    #endif
}

void Log::end() {
    #ifndef DISABLE_ALL_LOGGING
    Log* instance = getInstanceForCurrentTask();
    instance->_end();
    #endif
}

void Log::error(String msg) {
    #ifndef DISABLE_ALL_LOGGING
    getInstanceForCurrentTask()->_msg(Loglevel::ERROR, msg);
    #endif
}

void Log::errorln(const String& msg) {
    #ifndef DISABLE_ALL_LOGGING
    getInstanceForCurrentTask()->_errorln(msg);
    #endif
}


void Log::warn(String msg) {
    #ifndef DISABLE_ALL_LOGGING
    #if MAX_LOG_LEVEL >= WARN
        getInstanceForCurrentTask()->_warn(msg);
    #endif
    #endif
}
void Log::warnln(String msg) {
    #ifndef DISABLE_ALL_LOGGING
    #if MAX_LOG_LEVEL >= WARN
        getInstanceForCurrentTask()->_warnln(msg);
    #endif
    #endif
}


void Log::info(String msg) {
    #ifndef DISABLE_ALL_LOGGING
    #if MAX_LOG_LEVEL >= 2
        getInstanceForCurrentTask()->_info(msg);
    #endif
    #endif
}
void Log::infoln(String msg) {
    #ifndef DISABLE_ALL_LOGGING
    #if MAX_LOG_LEVEL >= 2
        getInstanceForCurrentTask()->_infoln(msg);
    #endif
    #endif
}

void Log::debug(String msg) {
    #ifndef DISABLE_ALL_LOGGING
    #if MAX_LOG_LEVEL >= 3
        getInstanceForCurrentTask()->_debug(msg);
    #endif
    #endif
}
void Log::debugln(String msg) {
    #ifndef DISABLE_ALL_LOGGING
    #if MAX_LOG_LEVEL >= 3
        getInstanceForCurrentTask()->_debugln(msg);
    #endif
    #endif
}

void Log::verbose(String msg) {
    #ifndef DISABLE_ALL_LOGGING
    #if MAX_LOG_LEVEL >= 4
        getInstanceForCurrentTask()->_verbose(msg);
    #endif
    #endif
}
void Log::verboseln(String msg) {
    #ifndef DISABLE_ALL_LOGGING
    #if MAX_LOG_LEVEL >= 4
        getInstanceForCurrentTask()->_verboseln(msg);
    #endif
    #endif
}

void Log::freeMem(String prefix, bool inBytes) {
    getInstanceForCurrentTask()->_freeMem(prefix, inBytes);
}

void Log::freeMem() {
    getInstanceForCurrentTask()->_freeMem();
}


void Log::_start(String cls, String method) {
    if (!Log::config.enabled) return;

    // Serial.printf("Log::_start(): TaskID = %d, Class = %s, Method = %s\n", taskID, cls.c_str(), method.c_str());


    if (xSemaphoreTake(logSemaphoreStartStop, 1000 / portTICK_PERIOD_MS) != pdTRUE) {
        Serial.println("Log-Semaphore not available!");
        esp_backtrace_print(30);
        while (true) {
            delay(1);
        }
    }

    classStack.push(cls);
    methodStack.push(method);
    startTimeStack.push(millis());
    String prefix = cls + "::" + method;


    if (!_shouldLog(prefix, Loglevel::DEBUG)) {
        lastPrefix = prefix;
        xSemaphoreGive(logSemaphoreStartStop);
        return;
    }

    newLineStarted = true;
    lastPrefix = prefix;
    if (config.printStartEndMessages) _msg(Loglevel::DEBUG, "\n", true);
    depth++;

    xSemaphoreGive(logSemaphoreStartStop);
}

void Log::_end() {
    if (!Log::config.enabled) return;

    // Serial.printf("Log::_end(): TaskID = %d, lastPrefix = %s\n", taskID, lastPrefix.c_str());


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

    // Serial.print("prefix: [");
    // Serial.flush();
    // Serial.print(prefix);
    // Serial.println("] (" + String(prefix.length()) + " Bytes)");
    // Serial.flush();

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
        Log::warnln("ERROR - depth < 0:  " + String(depth));
    }

    if (!newLineStarted) {
        newLineStarted = true;
    }

    String durationMsg = ANSICOLOR_RESET + " " + ANSICOLOR_BRIGHT_BLACK + " (" + String(millis() - startTime) + "ms)" +
        ANSICOLOR_RESET;

    const String savedPrefix = prefix;
    lastPrefix = prefix;
    if (config.printStartEndMessages) _msg(Loglevel::DEBUG, durationMsg + "\n", false, true);
    lastPrefix = "";

    if (!classStack.empty() && !methodStack.empty()) {
        cls = classStack.top();
        method = methodStack.top();
        lastPrefix = cls + "::" + method;
    }
    else {
        lastPrefix = "";
    }
    xSemaphoreGive(logSemaphoreStartStop);
}

void Log::_errorln(const String& msg) {
    error(msg + "\n");

    config.customErrorAction(msg);

    if (Log::config.restartESPonError) abort();
}

void Log::_warn(String msg) {
    config.customWarningAction(msg);
    _msg(Loglevel::WARN, msg);
}

void Log::_warnln(String msg) {
    config.customWarningAction(msg);
    _msg(Loglevel::WARN, msg + "\n");
}

void Log::_info(String msg) {
    _msg(Loglevel::INFO, msg);
}

void Log::_infoln(String msg) {
    _msg(Loglevel::INFO, msg + "\n");
}

void Log::_debug(String msg) {
    _msg(Loglevel::DEBUG, msg);
}

void Log::_debugln(String msg) {
    _msg(Loglevel::DEBUG, msg + "\n");
}

void Log::_verbose(String msg) {
    _msg(Loglevel::VERBOSE, msg);
}

void Log::_verboseln(String msg) {
    _msg(Loglevel::VERBOSE, msg + "\n");
}

String Log::_colorPrefix(String prefix, boolean isStart, boolean isEnd) {
    if (prefix == "") return prefix;

    String retVal;
    String startStopPrefix;
    String startStopSuffix;
    if (isStart) {
        startStopPrefix = " ++ ";
        // startStopPrefix = "[START] ";
        startStopSuffix = " - [START]";
    }
    else if (isEnd) {
        startStopPrefix = " -- ";
        // startStopPrefix = "[END] ";
        startStopSuffix = " - [END]";
    }


    auto result = split(prefix.c_str(), "::");
    if (result.empty() || result.size() < 2) {
        _errorln("ERROR!! Result empty: " + String(prefix.c_str()));
    }
    String cls = result[0];
    String method = result[1];

    retVal = ANSICOLOR_YELLOW + startStopPrefix + ansiColorReset()
        + ANSICOLOR_GREEN + cls + ansiColorReset() + "::" + ANSICOLOR_BLUE + method
        + ANSICOLOR_YELLOW + startStopSuffix
        + ansiColorReset();

    return retVal;
}

String Log::getBGColor() {
    int colorIdx = taskID <= TaskIdColors.size() ? taskID - 1 : 0;
    return TaskIdBGColors[colorIdx];
}

void Log::_msg(Loglevel loglevel, String msg, boolean isStart, boolean isEnd) {
    if (!config.enabled) return;

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

        // printing multilineBuffer:
        // _msg(loglevel, "\n");
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

    bool shouldLog = _shouldLog(loglevel) || loglevel <= Loglevel::WARN;

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
        }
        else {
            Serial.print(msg);
        }
    }
    multilineBuffer = "";

    lastloglevel = loglevel;

    xSemaphoreGive(logSemaphoreMessage);
}

void Log::_addFreeMemToMessage() {
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

bool Log::_shouldLog(String prefix, Loglevel requestedLoglevel) {
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

    const bool* found = searchLoggingElements(config.elements);
    if (found != nullptr) {
        return *found;
    }

    // Fallback auf Default-Loglevel
    return requestedLoglevel <= config.loglevel;
}



bool Log::_shouldLog(Loglevel loglevel) {
    if (!lastPrefix.equals("")) {
        return _shouldLog(lastPrefix, loglevel);
    }
    // Log::errorln("no last prefix!");
    return true;
}


void Log::_freeMem(String prefix, bool inBytes) {
    if (!config.enableFreeMem) return;
    if (!_shouldLog(prefix, Loglevel::DEBUG)) return;

    int freePSRam = esp_get_free_heap_size() * (inBytes ? 1 : 1.0 / 1024.0);
    int freeHeap = heap_caps_get_free_size(MALLOC_CAP_DMA) * (inBytes ? 1 : 1.0 / 1024.0);
    UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    String unit = inBytes ? "B" : "kB";
    int largestFreeBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) * (inBytes ? 1 : 1.0 / 1024.0);
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

void Log::_freeMem() {
    _freeMem("");
}

String Log::ts() {
    unsigned long millisVal = millis();
    unsigned long hours = millisVal / 3600000;
    unsigned long minutes = (millisVal % 3600000) / 60000;
    unsigned long seconds = (millisVal % 60000) / 1000;
    unsigned long milliseconds = millisVal % 1000;

    char buffer[13]; // Platz für "HH:MM:SS.mmm"
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu.%03lu", hours, minutes, seconds, milliseconds);

    return ANSICOLOR_WHITE + buffer + " " + ansiColorReset();
}

String Log::ansiColorReset() {
    return ANSICOLOR_RESET + getBGColor();
}

// Funktion zum Aufteilen eines Strings anhand eines Delimiters
std::vector<std::string> Log::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(s);

    while (std::getline(ss, token, delimiter)) {
        tokens.emplace_back(trim(token).c_str());
    }

    return tokens;
}

std::vector<char*> Log::split(char* str, char delimiter) {
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

std::vector<String> Log::split(const std::string &input, const std::string &delimiter) {
    std::vector<String> tokens;
    size_t start = 0, end;

    while ((end = input.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(input.substr(start, end - start).c_str());
        start = end + delimiter.length();
    }

    tokens.push_back(input.substr(start).c_str());
    return tokens;
}

String Log::ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    return String((start == std::string::npos) ? "" : s.substr(start).c_str());
}

String Log::rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    return String((end == std::string::npos) ? "" : s.substr(0, end + 1).c_str());
}

String Log::trim(const std::string &s) {
    return rtrim(ltrim(s).c_str());
}

String Log::formatNumber(int number) {
    std::stringstream ss;
    ss << number;
    std::string numberStr = ss.str();

    int len = numberStr.length();
    int numCommas = (len - 1) / 3;
    if (numCommas == 0)
        return numberStr.c_str();

    std::string formattedNumber;
    int i = 0;
    int firstGroupSize = len % 3 == 0 ? 3 : len % 3;
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
