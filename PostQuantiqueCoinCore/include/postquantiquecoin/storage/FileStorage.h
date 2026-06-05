#pragma once
#include "postquantiquecoin/core/Result.h"
#include <filesystem>
#include <string>
#include <vector>
namespace pqc {
class FileStorage {
public:
    explicit FileStorage(std::filesystem::path dataDir);
    Result<void> EnsureDataDirectories() const;
    static Result<void> AtomicWrite(const std::filesystem::path& path, const std::vector<uint8_t>& bytes);
    static Result<std::vector<uint8_t>> ReadFile(const std::filesystem::path& path);
    static Result<void> WriteText(const std::filesystem::path& path, const std::string& text);
    static Result<std::string> ReadText(const std::filesystem::path& path);
    static bool FileExists(const std::filesystem::path& path);
    static bool DirectoryExists(const std::filesystem::path& path);
    static Result<std::vector<std::filesystem::path>> ListFiles(const std::filesystem::path& path);
    static Result<std::string> ComputeFileChecksum(const std::filesystem::path& path);
    static Result<std::filesystem::path> BackupFile(const std::filesystem::path& path);
    static Result<void> AppendLogSafe(const std::filesystem::path& path, const std::string& line);
    const std::filesystem::path& DataDir() const;
private: std::filesystem::path dataDir_; };
}
