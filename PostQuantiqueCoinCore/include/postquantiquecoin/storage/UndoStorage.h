#pragma once
#include "postquantiquecoin/blockchain/BlockUndo.h"
#include "postquantiquecoin/core/Result.h"
#include <filesystem>
namespace pqc { class UndoStorage { public: explicit UndoStorage(std::filesystem::path dir); Result<void> WriteUndoAtomic(const BlockUndo& undo); Result<BlockUndo> ReadUndo(const std::string& blockHash) const; private: std::filesystem::path dir_; }; }
