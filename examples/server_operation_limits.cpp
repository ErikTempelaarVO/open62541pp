// Example server with configurable OPC UA operation limits
//
// This server allows setting operation limits via command-line arguments:
// - MaxMonitoredItemsPerCall: Maximum number of monitored items per CreateMonitoredItems call
// - MaxNodesPerRead: Maximum number of nodes per Read service call
// - MaxNodesPerWrite: Maximum number of nodes per Write service call
// - MaxNodesPerBrowse: Maximum number of nodes per Browse service call
//
// The server creates 200 test variables that clients can use to test chunking behavior.
// Use together with client_operation_limits to validate automatic request chunking.
//
// Usage:
//   ./server_operation_limits --max-monitored-items-per-call 15 --max-nodes-per-read 20
//

#include <cstdlib>
#include <iostream>

#include <open62541pp/node.hpp>
#include <open62541pp/server.hpp>

#include "helper.hpp"

void printHelp(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n"
              << "\n"
              << "Options:\n"
              << "  --max-monitored-items-per-call <limit>  Set MaxMonitoredItemsPerCall (default: 1000)\n"
              << "  --max-nodes-per-read <limit>            Set MaxNodesPerRead (default: 1000)\n"
              << "  --max-nodes-per-write <limit>           Set MaxNodesPerWrite (default: 1000)\n"
              << "  --max-nodes-per-browse <limit>          Set MaxNodesPerBrowse (default: 1000)\n"
              << "  --help, -h                              Display this help message\n"
              << "\n"
              << "Example:\n"
              << "  " << programName << " --max-monitored-items-per-call 10 --max-nodes-per-read 20\n";
}

int main(int argc, char* argv[]) {
    CliParser cli(argc, argv);

    if (cli.hasFlag("--help") || cli.hasFlag("-h")) {
        printHelp(argv[0]);
        return 0;
    }

    // Parse command line arguments
    uint32_t maxMonitoredItemsPerCall = 1000;
    uint32_t maxNodesPerRead = 1000;
    uint32_t maxNodesPerWrite = 1000;
    uint32_t maxNodesPerBrowse = 1000;

    if (auto value = cli.value("--max-monitored-items-per-call")) {
        maxMonitoredItemsPerCall = std::atoi(value->data());
    }
    if (auto value = cli.value("--max-nodes-per-read")) {
        maxNodesPerRead = std::atoi(value->data());
    }
    if (auto value = cli.value("--max-nodes-per-write")) {
        maxNodesPerWrite = std::atoi(value->data());
    }
    if (auto value = cli.value("--max-nodes-per-browse")) {
        maxNodesPerBrowse = std::atoi(value->data());
    }

    // Create and configure server
    opcua::ServerConfig config;
    config.setApplicationName("OPC UA Server with Operation Limits");
    config.setApplicationUri("urn:open62541pp.server.operation_limits");

    // Set operation limits directly in the underlying UA_ServerConfig
    // These values will be exposed to clients through standard OPC UA nodes
    config.handle()->maxMonitoredItemsPerCall = maxMonitoredItemsPerCall;
    config.handle()->maxNodesPerRead = maxNodesPerRead;
    config.handle()->maxNodesPerWrite = maxNodesPerWrite;
    config.handle()->maxNodesPerBrowse = maxNodesPerBrowse;

    std::cout << "\nConfiguring server operation limits:\n";
    std::cout << "  - MaxMonitoredItemsPerCall: " << maxMonitoredItemsPerCall << "\n";
    std::cout << "  - MaxNodesPerRead: " << maxNodesPerRead << "\n";
    std::cout << "  - MaxNodesPerWrite: " << maxNodesPerWrite << "\n";
    std::cout << "  - MaxNodesPerBrowse: " << maxNodesPerBrowse << "\n";

    opcua::Server server{std::move(config)};

    // Create some test nodes for clients to interact with
    std::cout << "\nCreating test nodes...\n";
    opcua::Node objectsFolder{server, opcua::ObjectId::ObjectsFolder};
    
    // Create a folder to hold test variables
    opcua::Node testFolder = objectsFolder.addFolder({1, "TestFolder"}, "TestFolder");
    std::cout << "  - Created TestFolder\n";

    // Create 200 test variables for testing large operations
    for (int i = 0; i < 200; ++i) {
        std::string name = "TestVariable" + std::to_string(i);
        opcua::Node varNode = testFolder.addVariable({1, name}, name);
        varNode.writeValue(opcua::Variant{static_cast<int32_t>(i)});
    }
    std::cout << "  - Created 200 test variables (TestVariable0 - TestVariable199)\n";

    std::cout << "\nServer started at opc.tcp://localhost:4840\n";
    std::cout << "Press Ctrl+C to stop the server\n\n";

    server.run();
}
