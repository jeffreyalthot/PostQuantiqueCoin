#include "postquantiquecoin/storage/FileStorage.h"
#include "postquantiquecoin/core/Hex.h"
#include "postquantiquecoin/crypto/Hashing.h"
#include <fstream>
namespace pqc {
FileStorage::FileStorage(std::filesystem::path d):dataDir_(std::move(d)){}
const std::filesystem::path& FileStorage::DataDir() const { return dataDir_; }
Result<void> FileStorage::EnsureDataDirectories() const { try{ std::filesystem::create_directories(dataDir_/"blocks"); std::filesystem::create_directories(dataDir_/"chainstate"); std::filesystem::create_directories(dataDir_/"wallets"); std::filesystem::create_directories(dataDir_/"logs"); return Result<void>::Ok(); }catch(const std::exception& e){ return Result<void>::Err(e.what()); } }
Result<void> FileStorage::AtomicWrite(const std::filesystem::path& p,const std::vector<uint8_t>& b){ try{ std::filesystem::create_directories(p.parent_path()); auto tmp=p; tmp+= ".tmp"; { std::ofstream f(tmp,std::ios::binary); if(!f) return Result<void>::Err("open write failed"); f.write(reinterpret_cast<const char*>(b.data()),static_cast<std::streamsize>(b.size())); if(!f) return Result<void>::Err("write failed"); } if(std::filesystem::exists(p)) std::filesystem::remove(p); std::filesystem::rename(tmp,p); return Result<void>::Ok(); }catch(const std::exception& e){ return Result<void>::Err(e.what()); } }
Result<std::vector<uint8_t>> FileStorage::ReadFile(const std::filesystem::path& p){ std::ifstream f(p,std::ios::binary); if(!f) return Result<std::vector<uint8_t>>::Err("open read failed"); return Result<std::vector<uint8_t>>::Ok(std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),{})); }
Result<void> FileStorage::WriteText(const std::filesystem::path& p,const std::string& t){ return AtomicWrite(p,std::vector<uint8_t>(t.begin(),t.end())); }
Result<std::string> FileStorage::ReadText(const std::filesystem::path& p){ auto r=ReadFile(p); if(r.IsErr()) return Result<std::string>::Err(r.Error()); return Result<std::string>::Ok(std::string(r.Value().begin(),r.Value().end())); }
bool FileStorage::FileExists(const std::filesystem::path& p){ return std::filesystem::is_regular_file(p); }
bool FileStorage::DirectoryExists(const std::filesystem::path& p){ return std::filesystem::is_directory(p); }
Result<std::vector<std::filesystem::path>> FileStorage::ListFiles(const std::filesystem::path& p){ try{ std::vector<std::filesystem::path> v; if(!std::filesystem::exists(p)) return Result<std::vector<std::filesystem::path>>::Ok(v); for(auto& e:std::filesystem::directory_iterator(p)) if(e.is_regular_file()) v.push_back(e.path()); return Result<std::vector<std::filesystem::path>>::Ok(v); }catch(const std::exception& e){ return Result<std::vector<std::filesystem::path>>::Err(e.what()); } }
Result<std::string> FileStorage::ComputeFileChecksum(const std::filesystem::path& p){ auto r=ReadFile(p); if(r.IsErr()) return Result<std::string>::Err(r.Error()); return Result<std::string>::Ok(Hashing::Sha3_256Hex(r.Value())); }
Result<std::filesystem::path> FileStorage::BackupFile(const std::filesystem::path& p){ try{ if(!FileExists(p)) return Result<std::filesystem::path>::Err("file missing"); auto dst=p; dst += ".bak"; std::filesystem::copy_file(p,dst,std::filesystem::copy_options::overwrite_existing); return Result<std::filesystem::path>::Ok(dst); }catch(const std::exception& e){ return Result<std::filesystem::path>::Err(e.what()); } }
Result<void> FileStorage::AppendLogSafe(const std::filesystem::path& p,const std::string& line){ try{ std::filesystem::create_directories(p.parent_path()); std::ofstream f(p,std::ios::app|std::ios::binary); if(!f) return Result<void>::Err("log open failed"); f<<line<<'\n'; return Result<void>::Ok(); }catch(const std::exception& e){ return Result<void>::Err(e.what()); } }
}
