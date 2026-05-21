#include "../include/commands/string/set.h"
#include "../include/commands/string/get.h"
#include "../include/commands/string/strlen.h"
#include "../include/commands/string/append.h"
#include "../include/commands/list/lpush.h"
#include "../include/commands/list/rpush.h"
#include "../include/commands/list/lpop.h"
#include "../include/commands/list/rpop.h"
#include "../include/commands/list/llen.h"
#include "../include/commands/list/lrange.h"
#include "../include/commands/list/lindex.h"
#include "../include/commands/list/lset.h"
#include "../include/commands/list/linsert.h"
#include "../include/commands/set/sadd.h"
#include "../include/commands/set/srem.h"
#include "../include/commands/set/sismember.h"
#include "../include/commands/set/smembers.h"
#include "../include/commands/set/scard.h"
#include "../include/commands/set/sdiff.h"
#include "../include/commands/set/sunion.h"
#include "../include/commands/set/sinter.h"
#include "../include/commands/set/smove.h"
#include "../include/commands/geo/compute_func.h"
#include "../include/commands/geo/geoadd.h"
#include "../include/commands/geo/geopos.h"
#include "../include/commands/geo/geodist.h"
#include "../include/commands/geo/geosearch.h"
#include "../include/commands/geo/geosearch_store.h"
#include "../include/commands/generic/del.h"
#include "../include/commands/generic/exists.h"
#include "../include/commands/generic/type.h"
#include "../include/commands/generic/keys.h"
#include "../include/commands/generic/flushdb.h"
#include "../include/commands/generic/dbsize.h"
#include "../include/commands/generic/config.h"
#include "../include/commands/generic/memory_usage.h"
#include "../include/commands/generic/expire.h"
#include "../include/commands/generic/ttl.h"
#include "../include/commands/generic/exit.h"
#include "../include/dispatcher.h"

#include <iostream>

void CommandsDispatcher::Register(std::unique_ptr<ICommand> cmd) {
  std::string name = cmd->Name();
  commands_[name] = std::move(cmd);
}

CommandsDispatcher::CommandsDispatcher() {
  Register(std::make_unique<Set>());
  Register(std::make_unique<Get>());
  Register(std::make_unique<Strlen>());
  Register(std::make_unique<Append>());
  Register(std::make_unique<LPush>());
  Register(std::make_unique<RPush>());
  Register(std::make_unique<LPop>());
  Register(std::make_unique<RPop>());
  Register(std::make_unique<LLen>());
  Register(std::make_unique<LRange>());
  Register(std::make_unique<LIndex>());
  Register(std::make_unique<LSet>());
  Register(std::make_unique<LInsert>());
  Register(std::make_unique<SAdd>());
  Register(std::make_unique<SRem>());
  Register(std::make_unique<SIsMember>());
  Register(std::make_unique<SMembers>());
  Register(std::make_unique<SCard>());
  Register(std::make_unique<SUnion>());
  Register(std::make_unique<SInter>());
  Register(std::make_unique<SDiff>());
  Register(std::make_unique<SMove>());
  Register(std::make_unique<GeoAdd>());
  Register(std::make_unique<GeoPos>());
  Register(std::make_unique<GeoDist>());
  Register(std::make_unique<GeoSearch>());
  Register(std::make_unique<GeoSearchStore>());
  Register(std::make_unique<Del>());
  Register(std::make_unique<Exists>());
  Register(std::make_unique<Type>());
  Register(std::make_unique<Keys>());
  Register(std::make_unique<FlushDB>());
  Register(std::make_unique<DBSize>());
  Register(std::make_unique<Config>());
  Register(std::make_unique<Memory>());
  Register(std::make_unique<Expire>());
  Register(std::make_unique<Ttl>());
  Register(std::make_unique<Exit>());
}

OptionalResult CommandsDispatcher::Dispatch(DataBase& db, const Command& command) {
  if (command.name.empty()) {
    return std::nullopt;
  }
  auto it = commands_.find(command.name);
  if (it == commands_.end()) {
    std::cerr << "(error) unknown command '" << command.name << "'\n";
    return std::nullopt;
  }
  return it->second->Execute(db, command.args);
}