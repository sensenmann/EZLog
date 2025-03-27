# EZLOG API Documentation

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

