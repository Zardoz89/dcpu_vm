#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_ 1

#include <iostream>

#ifndef _DEBUG
#define _DEBUG 0
#endif

/**
 * Logging levels
 */
enum class LogLevel {
    OFF     = -1,
    ERROR   =  0,
    WARN    =  1,
    INFO    =  2,
    DEBUG   =  3,
};


namespace logger {
    /**
    * @brief Desired Logging level to show. 
    * If N in "debug(N)" is <= LOG_level, when will be show
    */
    static LogLevel LOG_level = LogLevel::WARN;

    /**
    * @brief Output stream to use
    */
    static std::ostream& out = std::clog;
}

class Debug
{
public:
    Debug( LogLevel level ) 
#if _DEBUG >= 1
    : m_output( level <= logger::LOG_level ), level(level)
#endif
    { }

    template<typename T>
    Debug& operator<<( T t)
    {
        #if _DEBUG >= 1
        if( m_output )
        {
            switch (level) {
                case LogLevel::ERROR:
                    logger::out << "[ERROR] ";
                    break;

                case LogLevel::WARN:
                    logger::out << "[WARN] ";
                    break;

                case LogLevel::INFO:
                    logger::out << "[INFO] ";
                    break;

                case LogLevel::DEBUG:
                    logger::out << "[DEBUG] ";
                    break;

                default:
                    break;
            }

            logger::out << t << std::endl;
            return *this;
        }
        else
        #endif
            return *this;
    }
private:
#if _DEBUG >= 1
    bool m_output;
    LogLevel level;
#endif
};

#endif // _DEBUG_HPP_
