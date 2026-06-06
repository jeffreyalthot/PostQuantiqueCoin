#include "postquantiquecoin/core/BuildInfo.h"
#include <sstream>

namespace pqc::build {
std::string BuildManifest() {
    std::ostringstream out;
    out << "PostQuantiqueCoin " << Version << "\n";
#ifdef PQC_USE_LIBOQS
    out << "crypto_provider=liboqs\n";
#else
    out << "crypto_provider=dev-only-fallback\n";
#endif
#ifdef NDEBUG
    out << "build_type=Release\n";
#else
    out << "build_type=Debug\n";
#endif
    out << "config_version=" << ConfigVersion << "\n";
    return out.str();
}
}
