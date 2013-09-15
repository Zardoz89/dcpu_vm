#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_ 1

#include <iostream>

#ifndef _DEBUG
#define _DEBUG 0
#endif



namespace logger {
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

/**
* @brief Desired Logging level to show. 
* If N in "debug(N)" is <= LOG_level, when will be show
*/
static LogLevel LOG_level = LogLevel::DEBUG;

/**
* @brief Output stream to use
*/
static std::ostream& out = std::clog;

class Debug
{
public:
    Debug( LogLevel level ) 
#if _DEBUG >= 1
    : m_output( level <= logger::LOG_level ), level(level)
    {
        if( m_output ) {
            switch (level) {
            case LogLevel::ERROR:
                logger::out << "[ERROR] ";
                break;

            case LogLevel::WARN:
                logger::out << "[WARNING] ";
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
            // Auto ident more if is more severe
            //logger::out << std::string( static_cast<int>(logger::LOG_level) -
            //        static_cast<int>(level), '\t');
        }
#else
    {
#endif
    }

    ~Debug()
    {
#if _DEBUG>= 1
        if (m_output) {
            logger::out << std::endl;
            logger::out.flush();
        }
#endif
    }

    template<typename T>
    Debug& operator<<( T t)
    {
        #if _DEBUG >= 1
        if( m_output ) {
            logger::out << t;
            return *this;
        }
        #endif
        return *this;
    }
private:
#if _DEBUG >= 1
    bool m_output;
    LogLevel level;
#endif
};

} // END OF NAMESPACE logger

// Macros to type less
#define LOG_DEBUG   logger::Debug(logger::LogLevel::DEBUG) 
#define LOG         logger::Debug(logger::LogLevel::INFO) 
#define LOG_WARN    logger::Debug(logger::LogLevel::WARN)
#define LOG_ERROR   logger::Debug(logger::LogLevel::ERROR)


#endif // _DEBUG_HPP_
