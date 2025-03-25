#ifndef LOGGABLE_H
#define LOGGABLE_H

#include "EZLog.h"

#ifndef DISABLE_ALL_LOGGING
    class AutoLog {
    public:
        AutoLog(const String& cls, const String& method);
        ~AutoLog();
    };

    class Loggable {
    protected:
        // Berechnet den Klassennamen aus dem Dateinamen (__FILE__)
        static String extractClassName(const String& filePath);

    public:
        virtual ~Loggable() = default;
        virtual String fileName() const = 0;  // Muss überschrieben werden
        String className() const;

    protected:
        // Automatisches Logging für Instanzmethoden
        #define EZ_LOG_CLASS() AutoLog _autoLogInstance(className(), __FUNCTION__)

        // Automatisches Logging für statische Methoden, jetzt mit __FILE__
        #define EZ_LOG_STATIC(file) AutoLog _autoLogInstance(Loggable::extractClassName(file), __FUNCTION__)
    public:
        Loggable() {}  // Standardkonstruktor
    };


    /**
     * Wird von methoden verwendet, welche sich in keiner Klasse befinden und daher nicht von Loggable ableiten können:
     **/
    class AutoLogFree {
    public:
        AutoLogFree(const String& fileName, const String& method);
        ~AutoLogFree();
    private:
        static String extractClassName(const String& filePath);
    };

    // Makro für Logging in freien Funktionen
        #define EZ_LOG(fileName) AutoLogFree _freeFunctionLoggerInstance(fileName, __FUNCTION__)




#else
    /**
     * Logging komplett deaktiviert:
     */
    class Loggable {
        virtual String fileName() const = 0;  // Muss überschrieben werden
    };
    #define EZ_LOG_CLASS()
    #define EZ_LOG_STATIC(file)
    #define EZ_LOG(fileName)
#endif


#endif  // LOGGABLE_H
