#include "Loggable.h"
#include "EZLog.h"

#ifndef DISABLE_ALL_LOGGING
    String _extractClassName(const String &filePath) {
        int lastSlash = filePath.lastIndexOf('/');
        int lastBackslash = filePath.lastIndexOf('\\');
        int start = (lastSlash > lastBackslash) ? lastSlash + 1 : lastBackslash + 1;

        int lastDot = filePath.lastIndexOf('.');
        if (lastDot > start) {
            return filePath.substring(start, lastDot);  // Entfernt den Pfad und die Dateiendung
        }
        return filePath.substring(start);  // Falls keine Dateiendung vorhanden ist
    }

    String Loggable::extractClassName(const String &filePath) {
        return _extractClassName(filePath);
    }

    String Loggable::className() const {
        return _extractClassName(fileName());
    }

    String AutoLogFree::extractClassName(const String &filePath) {
        return _extractClassName(filePath);
    }

    AutoLog::AutoLog(const String &cls, const String &method) {
        Log::start(cls, method);
    }

    AutoLog::~AutoLog() {
        Log::end();
    }

    AutoLogFree::AutoLogFree(const String &fileName, const String &method) {
        Log::start(extractClassName(fileName), method);
    }

    AutoLogFree::~AutoLogFree() {
        Log::end();
    }
#endif
