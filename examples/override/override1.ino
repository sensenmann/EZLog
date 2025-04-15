/** Include Header-File: */
#include <Arduino.h>
#include "EZLog.h"

LoggingConfig loggingConfig;

void doSomething();

void setup() {
    /** Setting Serial Console */
    Serial.begin(115200);

    /** Create a simple loggingConfig: */
    loggingConfig.loglevel = Loglevel::VERBOSE;
    loggingConfig.customLoggingElements = {
        {
            .filter = "main::doSomething",        // all Logging in doSomething() should be limited to INFO-Level and higher
            .loglevel = Loglevel::INFO,
        }
    };


    /** Setup EZLog: */
    Log::init(loggingConfig);
}

void loop() {
    EZ_LOG("main");

    delay(2000);
    Log::infoln("Starting our example");

    doSomething();

    /** Changing Config: */
    loggingConfig.overrideLogAll = true;
    Log::updateConfig(loggingConfig);

    Log::infoln("Now our second try!");

    /** Do it again: */
    doSomething();  // Now you should see the verbose

    while (true);
}

void doSomething() {
    EZ_LOG("main");
    Log::verboseln("You should not see this!");
    delay(2000);
}
