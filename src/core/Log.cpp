#include "core/Log.h"

#include <cstdarg>
#include <cstdio>

static LogLevel g_min_level = LogLevel::Info;

void Log(LogLevel level, const char* format, ...) {
    if (level < g_min_level) {
        return;
    }
    
    const char* prefix = "";
    switch (level) {
        case LogLevel::Debug:
            prefix = "[DEBUG] ";
            break;
        case LogLevel::Info:
            prefix = "[INFO] ";
            break;
        case LogLevel::Warning:
            prefix = "[WARN] ";
            break;
        case LogLevel::Error:
            prefix = "[ERROR] ";
            break;
    }
    
    std::printf("%s", prefix);
    va_list args;
    va_start(args, format);
    std::vprintf(format, args);
    va_end(args);
    std::printf("\n");
}

void LogDebug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(LogLevel::Debug, "%s", buffer);
}

void LogInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(LogLevel::Info, "%s", buffer);
}

void LogWarning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(LogLevel::Warning, "%s", buffer);
}

void LogError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(LogLevel::Error, "%s", buffer);
}


