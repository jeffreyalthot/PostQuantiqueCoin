#include "postquantiquecoin/crypto/PQCryptoProvider.h"
#include "postquantiquecoin/crypto/OqsPqCryptoProvider.h"
#include "postquantiquecoin/crypto/DevOnlyFallbackCryptoProvider.h"
#include <stdexcept>
namespace pqc {
std::unique_ptr<PQCryptoProvider> CreateDefaultCryptoProvider(){
#ifdef PQC_USE_LIBOQS
    return std::make_unique<OqsPqCryptoProvider>();
#elif defined(PQC_DEV_ONLY_ALLOW_INSECURE_CRYPTO)
    return std::make_unique<DevOnlyFallbackCryptoProvider>();
#else
    throw std::runtime_error("PostQuantiqueCoin requires liboqs for production cryptography");
#endif
}
}
