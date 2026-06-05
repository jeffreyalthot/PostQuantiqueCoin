#pragma once
#include <filesystem>
#include <string>
namespace pqc { enum class LogLevel { Debug=0, Info=1, Warn=2, Error=3 }; class Logger { public: static void SetLogLevel(LogLevel level); static void SetLogFile(const std::filesystem::path& path); static void RotateLogs(); static void LogInfo(const std::string& category,const std::string& message); static void LogWarn(const std::string& category,const std::string& message); static void LogError(const std::string& category,const std::string& message); static void LogDebug(const std::string& category,const std::string& message); }; }
