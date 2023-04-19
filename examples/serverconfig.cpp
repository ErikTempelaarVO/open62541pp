#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    auto config = opcua::ServerConfig::builder()
    .defaults()
    .minimal(4841)
    .hostname("localhost.local")
    .build();

    // Does not compile, no modifications allowed after build()
    //config.minimal(4842);

    // Server receiver ServerConfig-object and converts it toNative UA_ServerConfig*
    opcua::Server server(config);
    server.run();
}
