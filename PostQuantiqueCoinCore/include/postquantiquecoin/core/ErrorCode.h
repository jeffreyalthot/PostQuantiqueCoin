#pragma once
#include <string>
namespace pqc { enum class ErrorCode { Ok, InvalidBlock, InvalidTransaction, InvalidSignature, InvalidAddress, StorageError, CryptoError, NetworkError, WalletLocked, WalletCorrupted, InsufficientFunds, ConsensusError, SerializationError, CorruptionDetected, PowInvalid, MempoolRejected, UnauthorizedSpend }; const char* ErrorCodeName(ErrorCode code); }
