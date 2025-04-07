#include "Loggable.h"
#include "EZLog.h"

#ifndef EZLOG_DISABLE_COMPLETELY
    String _extractClassName(const String &filePath) {
        const int lastSlash = filePath.lastIndexOf('/');
        const int lastBackslash = filePath.lastIndexOf('\\');
        const int start = (lastSlash > lastBackslash) ? lastSlash + 1 : lastBackslash + 1;

        const int lastDot = filePath.lastIndexOf('.');
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
       enabled = EZLog::start(cls, method);
    }

    AutoLog::~AutoLog() {
        if (enabled) EZLog::end();
    }

    AutoLogFree::AutoLogFree(const String &fileName, const String &method) {
        enabled = EZLog::start(extractClassName(fileName), method);
    }

    AutoLogFree::~AutoLogFree() {
        EZLog::end();
    }
#endif
