#include "postquantiquecoin/storage/BlockIndex.h"
#include "postquantiquecoin/storage/FileStorage.h"
#include <sstream>
namespace pqc {
BlockIndex::BlockIndex(std::filesystem::path p):path_(std::move(p)){}
void BlockIndex::SetPath(std::filesystem::path p){ path_=std::move(p); }
Result<void> BlockIndex::Add(const BlockIndexEntry& e){ if(e.hash.empty()) return Result<void>::Err("empty block hash"); byHash_[e.hash]=e; byHeight_[e.height]=e.hash; if(tip_.empty() || e.height>=byHash_[tip_].height) tip_=e.hash; return Result<void>::Ok(); }
bool BlockIndex::HasHash(const std::string& h) const { return byHash_.count(h)!=0; }
std::optional<BlockIndexEntry> BlockIndex::GetByHash(const std::string& h) const { auto it=byHash_.find(h); if(it==byHash_.end()) return std::nullopt; return it->second; }
std::optional<BlockIndexEntry> BlockIndex::GetByHeight(uint64_t h) const { auto it=byHeight_.find(h); if(it==byHeight_.end()) return std::nullopt; return GetByHash(it->second); }
std::optional<BlockIndexEntry> BlockIndex::GetTip() const { return tip_.empty()?std::optional<BlockIndexEntry>{}:GetByHash(tip_); }
Result<void> BlockIndex::SetTip(const std::string& h){ if(!HasHash(h)) return Result<void>::Err("unknown tip"); tip_=h; return Result<void>::Ok(); }
Result<void> BlockIndex::Save() const { std::ostringstream o; o<<"PQC_BLOCK_INDEX_V1\n"<<tip_<<"\n"; for(const auto& kv:byHash_){ const auto& e=kv.second; o<<e.hash<<'|'<<e.previousHash<<'|'<<e.height<<'|'<<e.bits<<'|'<<e.timestamp<<'|'<<e.fileNumber<<'|'<<e.fileOffset<<'|'<<e.serializedSize<<'|'<<e.cumulativeWork<<"\n"; } return FileStorage::WriteText(path_,o.str()); }
Result<void> BlockIndex::Load(){ Clear(); if(!FileStorage::FileExists(path_)) return Result<void>::Ok(); auto t=FileStorage::ReadText(path_); if(t.IsErr()) return Result<void>::Err(t.Error()); std::istringstream in(t.Value()); std::string line; if(!std::getline(in,line) || line!="PQC_BLOCK_INDEX_V1") return Result<void>::Err("bad block index format"); std::getline(in,tip_); while(std::getline(in,line)){ if(line.empty()) continue; std::vector<std::string> f; std::string x; std::istringstream ls(line); while(std::getline(ls,x,'|')) f.push_back(x); if(f.size()<9) return Result<void>::Err("bad block index row"); BlockIndexEntry e{f[0],f[1],std::stoull(f[2]),static_cast<uint32_t>(std::stoul(f[3])),std::stoull(f[4]),std::stoull(f[5]),std::stoull(f[6]),std::stoull(f[7]),f[8]}; auto r=Add(e); if(r.IsErr()) return r; } return Result<void>::Ok(); }
void BlockIndex::Clear(){ byHash_.clear(); byHeight_.clear(); tip_.clear(); }
}
