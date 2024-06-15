#ifndef __CPPSERVER_LOG_H__
#define __CPPSERVER_LOG_H__
#include <stdint.h>
#include <memory>
#include <string>
#include <list>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>

namespace cppserver
{
    enum LogLevel
    {
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    // Each LogEvent represents one log
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent();

    private:
        const char *m_file = nullptr; // filename
        int m_line = 0;               // line no.
        unsigned int m_elapse = 0;    // time in milliseconds from the start of program
        unsigned int m_threadId = 0;  // thread no.
        unsigned int m_fiberId = 0;   // fiber no.
        time_t m_time = 0;            // timestamp
        std::string m_content;        //
    };

    // 日志器
    class Logger
    {
    public:
        typedef Spinlock MutexType;
        typedef std::shared_ptr<Logger> ptr;
        Logger(const std::string &name = "root");
        void log(LogLevel level, const LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);
        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        void clearAppenders(); // remove all appenders
        LogLevel getLevel() const { return m_level; }
        void setLevel(LogLevel val) { m_level = val; }
        const std::string& getName() const { return m_name; }
        void setFormatter(LogFormatter::ptr val);
        void setFormatter(const std::string& val);
        LogFormatter::ptr getFormatter();
        std::string toYamlString();

    private:
        std::string m_name;                      // name of logger
        LogLevel m_level;                        // NOTE: logs will be filtered based on logger m_level
        MutexType m_mutex;
        std::list<LogAppender::ptr> m_appenders; // collection of log output destinations, single log can be outputted to multiple destinations
        LogFormatter::ptr m_formatter;
        Logger::ptr m_root;
    };

    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string &pattern);
        std::string format(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event);

    public:
        // submodule for log formats
        class FormatItem
        {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem() {}
            // Write log content to `os`
            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) = 0;
        };

        void init();

        bool isError() const { return has_error; }

        const std::string getPattern() const { return m_pattern; }

    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items; // FormatItem parsed from m_pattern
        bool has_error = false;
    };

    // 日志输出地
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        // Deleting a derived class object using a pointer of base class type that has a non-virtual destructor
        // results in undefined behavior. To correct this situation,
        // the base class should be defined with a virtual destructor.
        // https://www.geeksforgeeks.org/virtual-destructor/#
        virtual ~LogAppender();
        virtual void log(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) = 0;
        virtual std::string toYamlString() = 0;
        void setFormatter(LogFormatter::ptr val);
        LogFormatter::ptr getFormatter();
        LogLevel getLevel() const { return m_level; }
        void setLevel(LogLevel val) { m_level = val; }

    protected:
        LogLevel m_level;
        LogFormatter::ptr m_formatter;
    };

    // Output to console
    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(Logger::ptr logger, LogLevel level, LogEvent::ptr event) override; // https://en.cppreference.com/w/cpp/language/override
        std::string toYamlString() override;

    private:
    };

    // Output to single file
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        FileLogAppender(const std::string &filename);
        void log(Logger::ptr logger, LogLevel level, LogEvent::ptr event) override; // https://en.cppreference.com/w/cpp/language/override
        bool reopen();                                          // reopen the file, return true if open success, false otherwise
        std::string toYamlString() override;

    private:
        std::string m_filename;
        std::ofstream m_filestream;
        uint64_t m_lastTime = 0;
    };

} // namespace cppserver

#endif
