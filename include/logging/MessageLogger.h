#ifndef MESSAGELOGGER_H
#define MESSAGELOGGER_H

#include <string>

enum Severity {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class MessageLogger {
    public:
        virtual void log(std::string source, std::string msg, std::size_t timestamp, Severity severity) = 0;
        
    protected:
        static std::string severityToString(Severity severity) {
            switch (severity) {
                case TRACE: return "TRACE"; break;
                case DEBUG: return "DEBUG"; break;
                case INFO: return "INFO"; break;
                case WARNING: return "WARNING"; break;
                case ERROR: return "ERROR"; break;
                case FATAL: return "FATAL"; break;
                default: return "UNKNOWN"; break;
            }
        }
};

#endif