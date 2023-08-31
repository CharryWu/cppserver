# CPP Server by Charry Wu
## Dev env
Ubuntu, GCC 11.4.0, CMake

## Project Directory
```
src -- source code path
tests -- test file code path
bin -- binary ouptuts
build -- intermediary output files
cmake -- cmake function folder
lib -- library output path
CMakeLists.txt -- cmake definition
```

## Components
### Logging Module
Logger (Define log levels)
|
|---- Formatter (Format log streams on config)
|
Appender (Output destinations)

Inspired by the log4j framework, this logging module abstracts the logging process into three main components: `Logger`, `LogAppender`, and `LogFormatter`.

`Logger`: The Logger class is the primary interface for external usage. Log entries are only written if their log level is greater than or equal to the Logger's log level setting. Multiple Logger instances can coexist, allowing the separation of different types of logs. For example, system framework logs can be separated from business logic logs.

`LogAppender`: The `LogAppender` defines the destination for log outputs. Currently, two types of appenders are implemented: `StdoutLogAppender` (for console logging) and `FileLogAppender` (for file-based logging). Each appender has its own log level and log format, enabling flexible and distinct log outputs. This is particularly useful for categorizing logs of different levels, such as isolating error logs into a separate file to prevent them from being overshadowed by other types of logs.

`LogFormatter`: LogFormatter handles the formatting of log messages using a custom string format, akin to the `printf` format. This feature provides the flexibility to define log message formats according to specific needs.

### Fiber Encapsulation

### Socket Library

### HTTP Protocols


### Distributed Server Protocol

### Recommendation System
