/** Include Header-File: */
#include <Arduino.h>
#include "EZLog.h"

int sum(int a, int b);
void doMathStuff();

void setup() {
    /** Setting Serial Console */
    Serial.begin(115200);

    /** Create a simple loggingConfig: */
    LoggingConfig loggingConfig = {};
    loggingConfig.loglevel = Loglevel::VERBOSE;
    loggingConfig.addMemInfo = true;

    /** Setup EZLog: */
    Log::init(loggingConfig);
}

void loop() {
    EZ_LOG("main");

    delay(1000);
    Log::infoln("Starting main loop");
    Log::debugln("Let's do some calculations");
    Log::verboseln("This is a very verbose message");
    Log::warnln("This is a very important warning!");


    int cnt = 1;
    while (true) {
        Log::debugln("This is run #" + String(cnt++));
        Log::verboseln("cnt = " + String(cnt));
        doMathStuff();

        delay(1000);
        Log::warnln("This is a demo warning!");
        delay(2000);
    }
}

void doMathStuff() {
    EZ_LOG("main");
    int result = sum(2, 3);
    Log::infoln("The result is: " + String(result));
}

int sum(int a, int b) {
    EZ_LOG("main");
    Log::verboseln("Summing up: " + String(a) + " + " + String(b));

    int sum = a + b;
    Log::debugln("The result is: " + String(sum));

    return sum;
}
