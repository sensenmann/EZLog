![Logo](https://github.com/sensenmann/EZLog/blob/main/doc/EZLog_small.png?raw=true)

[![GitHub Stars](https://img.shields.io/github/stars/sensenmann/EZLog?style=social)](https://github.com/sensenmann/EZLog/stargazers)
[![GitHub Forks](https://img.shields.io/github/forks/sensenmann/EZLog?style=social)](https://github.com/sensenmann/EZLog/network/members)
![License](https://img.shields.io/github/license/sensenmann/EZLog)
![Platform](https://img.shields.io/badge/platform-ESP32-orange)
![Language](https://img.shields.io/github/languages/top/sensenmann/EZLog)
![Last Commit](https://img.shields.io/github/last-commit/sensenmann/EZLog)

# EZLog
An easy Logging-Framework for ESP32.


## Table of Contents
- [Features](#features)
- [Quickstart](#quickstart)
- [Usage](#usage)
- [EZLog Macros](#ezlog-macros)
- [Compiler Options](#compiler-preprocessor-variables)
- [Log Levels](#log-levels)
- [Further Information](#further-information)

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

![Example](https://github.com/sensenmann/EZLog/blob/main/doc/console-output1.png?raw=true)


## Quickstart

```cpp
#include "EZLog.h"

void setup() {
    Serial.begin(115200);
    Log::init({ .defaultLoglevel = Loglevel::DEBUG });
}

void loop() {
    EZ_LOG(__FILE__);
    Log::infoln("Hello, World!");
    Log::verboseln("This will only be printed in verbose mode");
}
```







## Usage

Including the `EZLog.h` Header-File in your project and initialize the Logger with your own [configuration](doc/Configuration.MD).

```c++
#include "EZLog.h"
void setup() {   
    Serial.begin(115200);
    
    /** Init EZLog: */
    Log::init({ .defaultLoglevel = Loglevel::DEBUG });
}
```


Use one of the three [EZLog Macros](#ezlog-macros) at the beginning of your methods, you want to use EZLog:
- `EZ_LOG(__FILE__)`
- `EZ_LOG_STATIC(__FILE__)`
- `EZ_LOG_CLASS()`


Both register the Start and the End of a function.  
Every Logging-Output will be prefixed by the given String or Classname, and will be indented.  
The name of your function will be calculated by EZLog automatically and does not need to be set.

```c++
void loop() {
    EZ_LOG(__FILE__);

    Log::infoln("Starting main loop");
    Log::debugln("Let's do some calculations");
    Log::verboseln("This is a very verbose message");
    Log::warnln("This is a very important warning!");
}
```

![Example](https://github.com/sensenmann/EZLog/blob/main/doc/console-output2.png?raw=true)


The easiest way to implement EZLog in a Class context is deriving from `Loggable` and implementing the `fileName()` Method.  
After that you only need to add the `EZ_LOG_CLASS()` Macro, where you wish to have EZLog available.
The Macro will output a "[START]"- and "[END]"-message  at the start/end of the function.

You can always use `String fileName() const override { return __FILE__; }` for your fileName().  
The C++ compiler will replace it with the actual filename of your actual file.

```c++
class YourClass : public Loggable {
public:
    String fileName() const override { return __FILE__; }   
private:
    void method1() {
        EZ_LOG_CLASS();        
        Log::debugln("doing things...");
    }       
    void method2() {
        EZ_LOG_CLASS();       
        Log::debugln("doing other things...");
    }   
}
```


If the method of the class is static, you can use `EZ_LOG_STATIC(__FILE__)` instead:
```c++
void MyClass::myFunction {
    EZ_LOG_STATIC(__FILE__);
    Log::debugln("Hello, World!");     
}
```


## Further Information

### IDE Integration
Click [here](doc/SETUP.md) to see how to integrate EZLog in ArduinoIDE or platformio.

### API
You can find a description of the available logging-methods in the [API Documentation](doc/API.md).

### EZLog Macros

- `EZ_LOG(__FILE__)`  
  → automatically logging of method start/end  
  → indents the logging-output one level to the right   
  → measures the duration of the method

- `EZ_LOG_CLASS()`  
  → same like above, but for use in classes - needs `Loggable`-inheritance

- `EZ_LOG_STATIC(__FILE__)`  
  → same like EZ_LOG(), but for use in static methods in classes

**Import:** _Don't use_ `Log::start()` and `Log::end()` directly!




### Compiler Preprocessor Variables
- `EZLOG_MAX_LOG_LEVEL`: Restricts maximum Loglevel, which can be used  (default = VERBOSE)
- `EZLOG_DISABLE_COLORS`: Disables colorful ANSI-Output, for IDEs like ArduinoIDE (default = not set)
- `EZLOG_DISABLE_COMPLETELY`: Disables Logging completely, if defined  (default = not set)


### Log-Levels
| Log-Level         | Description                                                         |
|-------------------|---------------------------------------------------------------------|
| `Loglevel::VERBOSE` | Extreme detailed information, which is only relevant for debugging. |
| `Loglevel::DEBUG`   | Detailed information, which is only relevant for debugging.         |
| `Loglevel::INFO`    | General information about the program flow.                         |
| `Loglevel::WARNING` | Indicates a potential problem, which is not critical.               |
| `Loglevel::ERROR`   | Indicates a critical problem, which should be fixed.                |

