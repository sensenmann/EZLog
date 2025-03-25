# EZLog
An easy Logging-Framework for ESP32.

## Features:
- Minimal code-adaptions necessary 
- **Colorful** Output
- Easy Call stack visualisation by **indention**
- Automatic logging of **Start** **and** End of a function
- Measuring the **duration** of functions
- Measuring the actual **Memory-Usage**
- Multiple Loglevels: ERROR, WARN, INFO, DEBUG, VERBOSE
- Multicore/-thread Support
- Logging **Filters** configurable

![Example](https://github.com/sensenmann/EZLog/blob/develop/doc/console-output1.png?raw=true)


## Getting started

### Setup up yor IDE
-> [Setup up yor IDE](doc/SETUP.md)


## Adapt your Code

Including the `EZLog.h` Header-File in your project  and initialize the Logger with your [configuration](#configuration):
`Log::init(loggingConfig);`

```c++
// main.cpp

/** Inlcude Header-File: */
#include "EZLog.h"

void setup() {
    /** Setting Serial Console */
    Serial.begin(115200);

    /** Create a simple loggingConfig: */
    LoggingConfig loggingConfig = {};
    loggingConfig.defaultLoglevel = Loglevel::VERBOSE;

    /** Setup EZLog: */
    Log::init(loggingConfig);
}
```

Including the Macro `EZ_LOG()` in your function(s) and use the `Log::...`-commands to   
```c++
void loop() {
    EZ_LOG("main");

    Log::infoln("Starting main loop");
    Log::debugln("Let's do some calculations");
    Log::verboseln("This is a very verbose message");
    Log::warnln("This is a very important warning!");
        
    /** Your Code... */
}
```

The `EZ_LOG()` / `EZ_LOG_CLASS()` - Makros at the beginning of a method, are a **failsafe** alternative, to Log::start() and Log::end().   
> Do not use Log::start() / end() NEVER EVER!

```c++
void loop() {
    EZ_LOG("main");
    // Log::start("main", loop()");    // <- Don't do this anymore!
        
    /** Your Code... */
    
    // Log::end();                      // <- Don't do this anymore!
}
```

![Example](https://github.com/sensenmann/EZLog/blob/develop/doc/console-output2.png?raw=true)


## Methods
| Method                           | Description                                                                          |
|----------------------------------|--------------------------------------------------------------------------------------|
| ~~start(className, methodName)~~ | Deprecated: Defines the Start of a method/function. <br> Use `EZ_START()` instead!   |
| ~~end()~~                        | Deprecated: Defines the End of a method/function. <br> Use `EZ_START()` instead!     |
| error(msg)                       | Prints an Error-Message _(without CRLF at the end)_                                  | 
| errorln(msg)                     | Prints an Error-Message                                                              |  
| warn(Stng msg)                   | Prints an Warning-Message _(without CRLF at the end)_                                |                   
| warnln(msg)                      | Prints an Warning-Message                                                            |                   
| info(msg)                        | Prints an Info-Message _(without CRLF at the end_)                                   |          
| infoln(msg)                      | Prints an Info-Message                                                               | 
| debug(msg)                       | Prints an Debug-Message _(without CRLF at the end)_                                  |   
| debugln(msg)                     | Prints an Debug-Message                                                              |      
| verbose(msg)                     | Prints an Verbose-Message _(without CRLF at the end)_                                |      
| verboseln(msg)                   | Prints an Verbose-Message                                                            |     
| freeMem()                        | Prints a Information about the free Memory on the system                             |
| freeMem(prefix, inBytes=  false) | Prints a Information about the free Memory on the system, with custom prefix         |



## Use EZLog in your application 

EZLog offers two simple Macros to include in your code:
- `EZ_LOG_CLASS()`
- `EZ_LOG(<String>)`

Both register the Start and the End of a function.  
Every Logging-Output will be prefixed by the given String or Classname, and will be indented.  


### EZLog in a class context

The easiest way to implement EZLog in a Class context is deriving from `Loggable` and implementing the `fileName()` Method.  
After that you only need to add the `EZ_LOG_CLASS()` Makro, where you wish to have EZLog avaible.
The Makro will output a "[START]"- and "[END]"-message  at the start/endof the function.

Example:
```c++
class YourClass : public Loggable {
public:
    String fileName() const override { return __FILE__; }   

private:
    void method1() {
        EZ_LOG_CLASS();        
        /** Your Code...*/
        Log::debugln("doing things...");
    }       

    void method2() {
        EZ_LOG_CLASS();       
        /** Your Code...*/
        Log::debugln("doing other things...");
    }   
}
```




### EZLog in a non-class context

```c++
EZ_LOG( String )
```  
Place this Macro on the first line of your function - it marks the START of your function.  

The `String`-parameter should give a hint to your filename.
The name of your function will be calculated by EZLog automatically and does not need to be set. 

Example (_main.cpp_):
```c++
int sum(int a, int b) {
    EZ_LOG("main");

    Log::verbosln("Summing up: " + String(a) + " + " + String(b));
   
    int sum = a + b;   
    Log::debugln("The result is: " + String(sum);
    
    return sum;
}
```

Output:
![Example](https://github.com/sensenmann/EZLog/blob/develop/doc/console-output3.png?raw=true)




## Configuration

At the beginning of your code (for example in setup()), you have to initialize EZLog with a Logging-Configuration.  
You can do this by:
```c+++
Log::init(loggingConfig);           // Sets the EZLog Configuration
```

You can change parameters of the configuration at runtime and update it with:
```c+++
Log::updateConfig(loggingConfig);   // Updates the EZLog Configuration
```

Available Properties:

| Property                   | Default           | Description                                                                                                                 |
|----------------------------|-------------------|-----------------------------------------------------------------------------------------------------------------------------|
| `enabled`                  | `true`            | Completely enables or disables the EZLog Output                                                                             |
| `addMemInfo`               | `false`           | Add's additional Info about the free Memory to each line                                                                    |
| `overrideLogAll`           | `false`           | displays every Log-Message, ignoring current max. Loglevel or exclude-Filters                                               |
| `printStartEndMessages`    | `true`            | displays [START] and [END] message for each function, which uses `EZ_LOG()` / `EZ_LOG_CLASS()`                              |
| `restartESPonError`        | `false`           | Executes an `abort()` after Log::error(), which causes the ESP32 to reboot. This can be usefull on a non-development build. |
| `loglevel`                 | `Loglevel::DEBUG` | Maximum severity, which will be shown in output (see [Loglevels](#log-levels))                                              |
| `customWarningAction(msg)` | -                 | Callback-Function, which allows custom code, when `Log::warn()` occurs                                                      |
| `customErrorAction(msg)` | -                 | Callback-Function, which allows custom code, when `Log::error()` occurs                                                  |


| `elements`              | -                | A List of LoggingElements, which can have custom Logging-Options (see [LoggingElements](#logging-elements)).                |


Example:
```c++
LoggingConfig loggingConfig = {
        .enabled = true,
        .addMemInfo = false,
        .overrideLogAll = false,
        .printStartEndMessages = true,
        .restartESPonError = false,        
        .loglevel = LogLevel::DEBUG,
        .customErrorAction = [](String errorMsg) { while(true); /** Stop execution */ },
        .elements = {
                        {"main",      LogLevel::VERBOSE},   // Single LoggingElement
                        
                        {"TFT / Framebuffer",               // Entry with Sub-Elements
                            {
                                 {"TFTService::",         LogLevel::DEBUG},
                                 {"Sprite::",             LogLevel::WARN},
                                 {"AnimatedSprite::",     LogLevel::WARN},
                                 {"Text::",               LogLevel::VERBOSE},
                            }
                    }              
        }
};
```


### Logging-Element

A `LoggingElement` defines a special Logging-Configuration, for Logging-Messages matching the name.

For example, you can set the default loglevel of the EZLog-Configuation to `Loglevel::WARN`, but override it for the Output of a specific class or method to  `Loglevel::DEBUG`.   
The matching is made by the filter (startsWith)

| Property      | Description                                                                                                             |
|---------------|-------------------------------------------------------------------------------------------------------------------------|
| `filter`      | The Name/Filter of the Logging-Element. Each output, matching this name will apply to this LoggingElement-Configuration |
| `loglevel`    | Overriding the Loglevel for this Logging-Element                                                                        |
| `subElements` | A List of Logging-Elements, for better organization                                                     |




### Log-Levels
| Log-Level         | Description                                                         |
|-------------------|---------------------------------------------------------------------|
| `Loglevel::VERBOSE` | Extreme detailed information, which is only relevant for debugging. |
| `Loglevel::DEBUG`   | Detailed information, which is only relevant for debugging.         |
| `Loglevel::INFO`    | General information about the program flow.                         |
| `Loglevel::WARNING` | Indicates a potential problem, which is not critical.               |
| `Loglevel::ERROR`   | Indicates a critical problem, which should be fixed.                |

