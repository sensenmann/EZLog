# EZLOG Configuration
## Initializing

At the beginning of your code (for example in setup()), you have to initialize EZLog with a Logging-Configuration.  
You can do this by:
```c+++
Log::init(loggingConfig);           // Sets the EZLog Configuration
```

You can change parameters of the configuration at runtime and update it with:
```c+++
Log::updateConfig(loggingConfig);   // Updates the EZLog Configuration
```

## Configurations
### General Properties

| Property                     | Default           | Description                                                                                                                 |
|------------------------------|-------------------|-----------------------------------------------------------------------------------------------------------------------------|
| `enabled`                    | `true`            | Completely enables or disables the EZLog Output                                                                             |
| `loglevel`                   | `Loglevel::DEBUG` | Maximum severity, which will be shown in output (see [Loglevels](#log-levels))                                              |
| `addMemInfo`                 | `false`           | Adds additional Info about the free Memory to each line                                                                    |
| `overrideLogAll`             | `false`           | displays every Log-Message, ignoring current max. Loglevel or exclude-Filters                                               |
| `printStartEndMessages`      | `true`            | displays [START] and [END] message for each function, which uses `EZ_LOG()` / `EZ_LOG_CLASS()`                              |
| `restartESPonError`          | `false`           | Executes an `abort()` after Log::error(), which causes the ESP32 to reboot. This can be usefull on a non-development build. |


### Callback Properties

| Property                     | Description                                                                   |
|------------------------------|-------------------------------------------------------------------------------|
| `customErrorAction(msg)`     | Callback-Function, which allows custom code, when `Log::error()` occurs       |
| `customWarningAction(msg)`   | Callback-Function, which allows custom code, when `Log::warn()` occurs        |
| `customInfoAction(msg)`      | Callback-Function, which allows custom code, when `Log::info()` occurs        |
| `customDebugAction(msg)`     | Callback-Function, which allows custom code, when `Log::debug()` occurs       |
| `customVerboseAction(msg)`   | Callback-Function, which allows custom code, when `Log::verbose()` occurs     |

### CustomLogging-Elements

| Property                     | Description                                                                                                 |
|------------------------------|-------------------------------------------------------------------------------------------------------------|
| `customLoggingElements`      | A List of LoggingElements, which can have custom Logging-Options (see [Logging-Element](#logging-element)). |


### Logging-Element

A `LoggingElement` defines a special Logging-Configuration, for Logging-Messages matching the name.

For example, you can set the default loglevel of the EZLog-Configuation to `Loglevel::WARN`, but override it for the Output of a specific class or method to  `Loglevel::DEBUG`.   
The matching is made by the filter (startsWith)

| Property      | Description                                                                                                             |
|---------------|-------------------------------------------------------------------------------------------------------------------------|
| `filter`      | The Name/Filter of the Logging-Element. Each output, matching this name will apply to this LoggingElement-Configuration |
| `loglevel`    | Overriding the Loglevel for this Logging-Element                                                                        |
| `subElements` | A List of Logging-Elements, for better organization                                                     |





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