// main.cpp

/** Include Header-File: */
#include <Arduino.h>
#include "EZLog.h"

void process1(void*);
void process2(void*);
void process3(void*);
void logSomething(int cnt, int processId);

void setup() {
    /** Setting Serial Console */
    Serial.begin(115200);

    /** Create a simple loggingConfig: */
    LoggingConfig loggingConfig = {};
    loggingConfig.loglevel = Loglevel::VERBOSE;

    /** Setup EZLog: */
    Log::init(loggingConfig);
}

void loop() {
    EZ_LOG("main");

    delay(1000);
    Log::infoln("Starting main loop");

    xTaskCreatePinnedToCore(process1,"process1",10240,NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(process2,"process2",10240,NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(process3,"process3",10240,NULL, 2, NULL, 0);


    Log::debugln("Let's do some logging:");
    int cnt = 1;
    while (true) {
        Log::debugln("This is run #" + String(cnt++));
        delay(1000);
    }
}

void process1(void*) {
    EZ_LOG(__FILE__);
    for (int cnt = 0; cnt < 10; cnt++) {
        logSomething(cnt, 1);
        delay(500);
    }
    vTaskDelete(NULL);
}

void process2(void*) {
    EZ_LOG(__FILE__);
    delay(1500);
    for (int cnt = 0; cnt < 10; cnt++) {
        logSomething(cnt, 2);
        delay(1500);
    }
    vTaskDelete(NULL);
}

void process3(void*) {
    EZ_LOG(__FILE__);
    delay(5000);
    for (int cnt = 0; cnt < 10; cnt++) {
        logSomething(cnt, 3);
        delay(500);
    }
    vTaskDelete(NULL);
}

void logSomething(int cnt, int processId) {
    EZ_LOG(__FILE__);
    Log::debugln("This is run #" + String(cnt) + " of process #" + String(processId));
}

