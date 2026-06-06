#include "postquantiquecoin/core/Config.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace pqc {
namespace {
std::string Trim(std::string s) {
    auto notSpace = [](unsigned char c){ return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}
bool On(const std::string& v){ auto x=Trim(v); std::transform(x.begin(), x.end(), x.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); }); return x=="1"||x=="true"||x=="yes"||x=="on"; }
std::vector<std::string> SplitCsv(const std::string& v){ std::vector<std::string> out; std::stringstream ss(v); std::string item; while(std::getline(ss,item,',')){ item=Trim(item); if(!item.empty()) out.push_back(item); } return out; }
void Apply(NodeConfig& c, const std::string& key, const std::string& value) {
    auto k = Trim(key); auto v = Trim(value);
    if(k=="config_version") c.config_version=static_cast<uint32_t>(std::stoul(v));
    else if(k=="network") c.network=v;
    else if(k=="datadir") c.datadir=v;
    else if(k=="port") c.port=static_cast<uint16_t>(std::stoul(v));
    else if(k=="maxpeers"||k=="maxconnections") c.maxpeers=static_cast<uint32_t>(std::stoul(v));
    else if(k=="listen") c.listen=On(v);
    else if(k=="bind") c.bind=v;
    else if(k=="connect") c.connect=SplitCsv(v);
    else if(k=="addnode") c.addnode=SplitCsv(v);
    else if(k=="seednode") c.seednode=SplitCsv(v);
    else if(k=="seedpeer") c.seedpeer=v;
    else if(k=="mining_threads") c.mining_threads=static_cast<uint32_t>(std::stoul(v));
    else if(k=="log_level") c.log_level=v;
    else if(k=="wallet_dir") c.wallet_dir=v;
    else if(k=="require_liboqs") c.require_liboqs=On(v);
    else if(k=="allow_dev_crypto") c.allow_dev_crypto=On(v);
    else if(k=="allow_legacy_pow") c.allow_legacy_pow=On(v);
    else if(k=="safe_start") c.safe_start=On(v);
    else if(k=="json") c.json=On(v);
}
}

Result<NodeConfig> Config::Load(const std::filesystem::path& path){
    NodeConfig c; c.conf_path = path;
    std::ifstream in(path); if(!in) return Result<NodeConfig>::Ok(c);
    std::string line;
    while(std::getline(in,line)){
        line = Trim(line); if(line.empty() || line[0]=='#') continue;
        auto pos=line.find('='); if(pos==std::string::npos) return Result<NodeConfig>::Err("invalid config line: " + line);
        Apply(c, line.substr(0,pos), line.substr(pos+1));
    }
    auto v = Validate(c); if(v.IsErr()) return Result<NodeConfig>::Err(v.Error());
    return Result<NodeConfig>::Ok(c);
}

Result<void> Config::Save(const std::filesystem::path& path,const NodeConfig& c){
    std::filesystem::create_directories(path.parent_path().empty()?std::filesystem::path("."):path.parent_path());
    std::ofstream out(path); if(!out) return Result<void>::Err("config write failed");
    out<<"config_version="<<c.config_version<<"\n"<<"network="<<c.network<<"\n"<<"datadir="<<c.datadir.string()<<"\n"<<"port="<<c.port<<"\n"<<"maxconnections="<<c.maxpeers<<"\n"<<"listen="<<(c.listen?1:0)<<"\n"<<"bind="<<c.bind<<"\n"<<"seedpeer="<<c.seedpeer<<"\n"<<"mining_threads="<<c.mining_threads<<"\n"<<"log_level="<<c.log_level<<"\n"<<"wallet_dir="<<c.wallet_dir.string()<<"\n"<<"require_liboqs="<<(c.require_liboqs?1:0)<<"\n"<<"allow_dev_crypto="<<(c.allow_dev_crypto?1:0)<<"\n"<<"allow_legacy_pow="<<(c.allow_legacy_pow?1:0)<<"\n"<<"safe_start="<<(c.safe_start?1:0)<<"\n";
    return Result<void>::Ok();
}

Result<NodeConfig> Config::FromArgs(int argc, char** argv, const NodeConfig& base) {
    NodeConfig cfg = base;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto needValue = [&](const std::string& opt)->Result<std::string>{ if(i+1>=argc) return Result<std::string>::Err("missing value for " + opt); return Result<std::string>::Ok(argv[++i]); };
        if (a == "--network") { auto v=needValue(a); if(v.IsErr()) return Result<NodeConfig>::Err(v.Error()); cfg.network=v.Value(); }
        else if (a == "--datadir") { auto v=needValue(a); if(v.IsErr()) return Result<NodeConfig>::Err(v.Error()); cfg.datadir=v.Value(); }
        else if (a == "--conf") { auto v=needValue(a); if(v.IsErr()) return Result<NodeConfig>::Err(v.Error()); cfg.conf_path=v.Value(); auto loaded=Load(cfg.conf_path); if(loaded.IsErr()) return loaded; cfg=loaded.Value(); }
        else if (a == "--listen") cfg.listen = true;
        else if (a == "--no-listen" || a == "--listen=0") cfg.listen = false;
        else if (a == "--bind") { auto v=needValue(a); if(v.IsErr()) return Result<NodeConfig>::Err(v.Error()); cfg.bind=v.Value(); }
        else if (a == "--connect") { auto v=needValue(a); if(v.IsErr()) return Result<NodeConfig>::Err(v.Error()); cfg.connect.push_back(v.Value()); }
        else if (a == "--addnode") { auto v=needValue(a); if(v.IsErr()) return Result<NodeConfig>::Err(v.Error()); cfg.addnode.push_back(v.Value()); }
        else if (a == "--seednode") { auto v=needValue(a); if(v.IsErr()) return Result<NodeConfig>::Err(v.Error()); cfg.seednode.push_back(v.Value()); }
        else if (a == "--maxconnections") { auto v=needValue(a); if(v.IsErr()) return Result<NodeConfig>::Err(v.Error()); cfg.maxpeers=static_cast<uint32_t>(std::stoul(v.Value())); }
        else if (a == "--json") cfg.json = true;
        else if (a == "--safe-start") cfg.safe_start = true;
        else if (a == "--unsafe-start") cfg.safe_start = false;
    }
    auto v = Validate(cfg); if(v.IsErr()) return Result<NodeConfig>::Err(v.Error());
    return Result<NodeConfig>::Ok(cfg);
}

Result<void> Config::Validate(const NodeConfig& c) {
    if (!(c.network=="mainnet" || c.network=="testnet" || c.network=="devnet" || c.network=="regtest")) return Result<void>::Err("network must be mainnet/testnet/devnet/regtest");
    if (c.datadir.empty()) return Result<void>::Err("datadir cannot be empty");
    if (c.port == 0) return Result<void>::Err("port cannot be zero");
    if (c.maxpeers == 0 || c.maxpeers > 10000) return Result<void>::Err("maxconnections out of range");
    if (c.network == "mainnet" && c.allow_dev_crypto) return Result<void>::Err("allow_dev_crypto is forbidden on mainnet");
    return Result<void>::Ok();
}
}
