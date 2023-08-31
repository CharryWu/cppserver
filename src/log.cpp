#include "log.h"

namespace cppserver
{

    Logger::Logger(const std::string &name) : m_name(name)
    {
    }

    void Logger::addAppender(LogAppender::ptr appender)
    {
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender)
    {
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::log(LogLevel level, const LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            for (auto &appender : m_appenders)
            {
                appender->log(level, event); // delegate the call to appender's function
            }
        }
    }

    void Logger::info(LogEvent::ptr event)
    {
        log(LogLevel::INFO, event);
    }

    void Logger::debug(LogEvent::ptr event)
    {
        log(LogLevel::DEBUG, event);
    }

    void Logger::warn(LogEvent::ptr event)
    {
        log(LogLevel::WARN, event);
    }

    void Logger::fatal(LogEvent::ptr event)
    {
        log(LogLevel::FATAL, event);
    }

    FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename)
    {
    }

    void FileLogAppender::log(LogLevel level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            m_filestream << m_formatter->format(event);
        }
    }

    bool FileLogAppender::reopen()
    {
        if (m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        !!m_filestream;
    }

    void StdoutLogAppender::log(LogLevel level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            std::cout << m_formatter->format(event);
        }
    }

    std::string LogFormatter::format(LogEvent::ptr event)
    {
        std::stringstream ss;
        for (const auto &i : m_items)
        {
            i->format(ss, event);
        }
        return ss.str();
    }

    // %xxx %xxx(xxx) %%
    void LogFormatter::init()
    {
        // str, format, type
        std::vector<std::pair<std::string, int>> vec;
        std::string str; // store non-token characters
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            // non-format characters
            if (m_pattern[i] != '%')
            {
                str.append(1, m_pattern[i]);
                continue;
            }
            size_t n = i + 1;
            int fmt_status = 0; // 1 => inside bracket
            std::string str;
            std::string fmt;
            size_t fmt_begin = 0;

            while (n < m_pattern.size())
            {
                if (isspace(m_pattern[n]))
                {
                    break;
                }

                if (fmt_status == 0)
                {
                    if (m_pattern[n] == '{')
                    {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_status = 1; // parse format
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                if (fmt_status == 1)
                {
                    if (m_pattern[n] == '}')
                    {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        fmt_status = 2;
                        ++n;
                        continue;
                    }
                }
            }
        }
    }
}
