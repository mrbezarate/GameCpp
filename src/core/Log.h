#pragma once

#include <cstdio>
#include <string>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

void Log(LogLevel level, const char* format, ...);
void LogDebug(const char* format, ...);
void LogInfo(const char* format, ...);
void LogWarning(const char* format, ...);
void LogError(const char* format, ...);


