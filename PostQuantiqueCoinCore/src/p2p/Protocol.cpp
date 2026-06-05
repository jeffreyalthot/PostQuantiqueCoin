#include "postquantiquecoin/p2p/Protocol.h"
namespace pqc::p2p { const std::vector<std::string>& KnownCommands(){ static std::vector<std::string> c{"version","verack","ping","pong","getheaders","headers","getblocks","inv","getdata","block","tx","mempool","addr","getaddr","reject","pq_client_hello","pq_server_hello","pq_session_confirm"}; return c; } }
