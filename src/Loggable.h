#ifndef LOGGABLE_H
#define LOGGABLE_H

#include "EZLog.h"

#ifndef EZLOG_DISABLE_COMPLETELY
    class AutoLog {
    public:
        AutoLog(const String& cls, const String& method);
        ~AutoLog();
    private:
        bool enabled = false;
    };



    class Loggable {
    public:
        Loggable() = default;
        virtual ~Loggable() = default;

        virtual String fileName() const = 0;  // must be overwritten by child-object
        String className() const;

    protected:
        // Calculates the Classname from the Filename (__FILE__)
        static String extractClassName(const String& filePath);

        // Automatic Logging for Instance methods
        #define EZ_LOG_CLASS() AutoLog _autoLogInstance(className(), __FUNCTION__)

        // Automatic Logging for static methods
        #define EZ_LOG_STATIC(file) AutoLog _autoLogInstance(Loggable::extractClassName(file), __FUNCTION__)
    };


    /**
     * Will be used by methods, which are NOT inside a class and can therefore not be derived from Loggable:
     **/
    class AutoLogFree {
    public:
        AutoLogFree(const String& fileName, const String& method);
        ~AutoLogFree();
    private:
        static String extractClassName(const String& filePath);
        bool enabled = false;
    };

    // Makro for Logging in this free functions
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
