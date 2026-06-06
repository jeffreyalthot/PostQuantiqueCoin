#include "postquantiquecoin/application/NodeService.h"

namespace pqc::application {
NodeService::NodeService(Blockchain& chain, NodeConfig config) : chain_(chain), config_(std::move(config)) {}

NodeInfo NodeService::GetInfo() const {
    return {chain_.Params().networkName, chain_.GetTipHash(), chain_.GetHeight(), chain_.GetMempool().Size(), chain_.GetMintedSupply()};
}

Result<void> NodeService::ValidateStartup() const {
    if (config_.network != chain_.Params().networkName) return Result<void>::Err("configuration network does not match chain params");
    if (config_.datadir.empty()) return Result<void>::Err("datadir is required");
    if (chain_.Params().networkName == "mainnet" && config_.allow_dev_crypto) return Result<void>::Err("dev-only crypto cannot be enabled on mainnet");
    return Result<void>::Ok();
}
}
