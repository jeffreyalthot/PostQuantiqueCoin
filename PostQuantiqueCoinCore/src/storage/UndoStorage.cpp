#include "postquantiquecoin/storage/UndoStorage.h"
#include "postquantiquecoin/storage/FileStorage.h"
namespace pqc { UndoStorage::UndoStorage(std::filesystem::path d):dir_(std::move(d)){ std::filesystem::create_directories(dir_); } Result<void> UndoStorage::WriteUndoAtomic(const BlockUndo& u){ return FileStorage::AtomicWrite(dir_/(u.blockHash+".undo"),u.Serialize()); } Result<BlockUndo> UndoStorage::ReadUndo(const std::string& h) const{ auto b=FileStorage::ReadFile(dir_/(h+".undo")); if(b.IsErr()) return Result<BlockUndo>::Err(b.Error()); return BlockUndo::Deserialize(b.Value()); } }
