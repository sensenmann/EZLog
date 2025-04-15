/** Include Header-File: */
#include <Arduino.h>
#include "EZLog.h"

LoggingConfig loggingConfig;

void doSomething();

void setup() {
    /** Setting Serial Console */
    Serial.begin(115200);

    /** Create a simple loggingConfig: */
    loggingConfig.loglevel = Loglevel::INFO;
    loggingConfig.customLoggingElements = {
        {
            .filter = "main::doSomething",
            .loglevel = Loglevel::VERBOSE,
        }
    };

    /** Setup EZLog: */
    Log::init(loggingConfig);
}

void loop() {
    EZ_LOG("main");

    delay(2000);
    Log::infoln("Starting our example");

    // We should see the verbose message:
    doSomething();

    /** Changing Config: */
    loggingConfig.customLoggingElements = {};
    Log::updateConfig(loggingConfig);

    Log::infoln("Now our second try!");

    /** Do it again: */
    doSomething();


    Log::infoln("Finished!");
    while (true);
}

void doSomething() {
    EZ_LOG("main");
    Log::verboseln("This is our secret message!");
    delay(2000);
}
