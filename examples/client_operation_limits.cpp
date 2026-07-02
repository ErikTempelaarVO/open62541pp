// Example demonstrating automatic chunking of OPC UA operations based on server limits
//
// This client uses LimitRespectingClient (from client_operation_limits.hpp) to automatically
// split large requests into smaller chunks that respect the server's operation limits.
//
// Features:
// - Queries operation limits from server (or uses user overrides)
// - Automatically chunks CreateMonitoredItems, Read, Write, and Browse operations
// - Shows chunking notifications when requests are split
// - Configurable via command-line arguments
//
// Usage examples:
//   ./client_operation_limits opc.tcp://localhost:4840
//   ./client_operation_limits --num-monitored-items 150 --num-reads 75 opc.tcp://localhost:4840
//
// Use together with server_operation_limits to test with a server that has specific limits.
//

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <open62541pp/client.hpp>
#include <open62541pp/subscription.hpp>
#include <open62541pp/ua/nodeids.hpp>

#include "client_operation_limits.hpp"
#include "helper.hpp"  // CliParser

inline static std::atomic<bool> isRunning = true;  // NOLINT(*global-variables)

static void signalHandler(int sig) noexcept {
    if (sig == SIGINT || sig == SIGTERM) {
        isRunning = false;
    }
}

int main(int argc, char* argv[]) {
    const CliParser parser{argc, argv};
    if (parser.nargs() < 2 || parser.hasFlag("-h") || parser.hasFlag("--help")) {
        std::cout
            << "usage: client_operation_limits [options] opc.tcp://<host>:<port>\n"
            << "options:\n"
            << "  --num-monitored-items <count>  Number of monitored items to create (default: 100)\n"
            << "  --num-reads <count>            Number of read operations (default: 50)\n"
            << "  --num-browses <count>          Number of browse operations (default: 30)\n"
            << "  --help, -h                     Show this help message\n"
            << std::flush;
        return 2;
    }

    const auto endpointUrl = std::string(parser.args().back());
    
    // Parse operation counts
    size_t numMonitoredItems = 100;
    if (auto val = parser.value("--num-monitored-items")) {
        numMonitoredItems = std::stoul(std::string(*val));
    }
    
    size_t numReads = 50;
    if (auto val = parser.value("--num-reads")) {
        numReads = std::stoul(std::string(*val));
    }
    
    size_t numBrowses = 30;
    if (auto val = parser.value("--num-browses")) {
        numBrowses = std::stoul(std::string(*val));
    }

    std::cout << "=== Limit Respecting Client Example ===\n\n";

    opcua::Client client;

    try {
        // Connect to server
        std::cout << "Connecting to " << endpointUrl << "...\n";
        client.connect(endpointUrl);
        std::cout << "Connected successfully!\n\n";

        // Create limit-respecting client wrapper
        opcua::LimitRespectingClient limitClient(client);

        // Query and configure operation limits from server
        std::cout << "Server operation limits (queried):\n";
        
        limitClient.enableMaxMonitoredItemsPerCall();
        std::cout << "- MaxMonitoredItemsPerCall: " << limitClient.getMaxMonitoredItemsPerCall() << "\n";
        
        limitClient.enableMaxNodesPerRead();
        std::cout << "- MaxNodesPerRead: " << limitClient.getMaxNodesPerRead() << "\n";
        
        limitClient.enableMaxNodesPerWrite();
        std::cout << "- MaxNodesPerWrite: " << limitClient.getMaxNodesPerWrite() << "\n";
        
        limitClient.enableMaxNodesPerBrowse();
        std::cout << "- MaxNodesPerBrowse: " << limitClient.getMaxNodesPerBrowse() << "\n";
        
        std::cout << "\nTest configuration:\n";
        std::cout << "- Monitored items to create: " << numMonitoredItems << "\n";
        std::cout << "- Read operations: " << numReads << "\n";
        std::cout << "- Browse operations: " << numBrowses << "\n";
        std::cout << "\n";

        // ========== Example 1: Create many monitored items ==========
        std::cout << "=== Example 1: Creating monitored items with automatic chunking ===\n";

        // First, create a subscription using the regular client
        auto sub = opcua::Subscription(client);

        // Prepare a large batch of monitored items
        // In a real scenario, this might exceed the server's MaxMonitoredItemsPerCall limit
        std::vector<opcua::MonitoredItemCreateRequest> itemsToCreate;
        itemsToCreate.reserve(numMonitoredItems);

        for (size_t i = 0; i < numMonitoredItems; ++i) {
            // Monitor various server status variables
            // In a real application, you would monitor your own nodes
            const opcua::NodeId nodeId =
                (i % 3 == 0) ? opcua::VariableId::Server_ServerStatus_CurrentTime
                : (i % 3 == 1) ? opcua::VariableId::Server_ServerStatus_StartTime
                             : opcua::VariableId::Server_ServerStatus_State;

            itemsToCreate.emplace_back(
                opcua::ReadValueId(nodeId, opcua::AttributeId::Value),
                opcua::MonitoringMode::Reporting,
                opcua::MonitoringParameters{}
            );
        }

        std::cout << "Creating " << itemsToCreate.size()
                  << " monitored items (will be chunked if needed)...\n";
        
        const uint32_t monLimit = limitClient.getMaxMonitoredItemsPerCall();
        if (monLimit > 0 && itemsToCreate.size() > monLimit) {
            const size_t numChunks = (itemsToCreate.size() + monLimit - 1) / monLimit;
            std::cout << "  -> Chunking into " << numChunks << " requests of max " 
                      << monLimit << " items each\n";
        }

        // Create the request
        opcua::CreateMonitoredItemsRequest createRequest(
            opcua::RequestHeader{},
            sub.subscriptionId(),
            opcua::TimestampsToReturn::Both,
            itemsToCreate
        );

        // Use the limit-respecting client - it will automatically chunk if needed
        auto createResponse = limitClient.createMonitoredItemsDataChange(
            createRequest,
            [](opcua::IntegerId subId,
               opcua::IntegerId monId,
               const opcua::DataValue& value) {
                (void)subId;
                (void)monId;
                (void)value;
                // Data change callback - normally you'd process the notification here
                // For this example, we just count them
                static std::atomic<int> notificationCount{0};
                if (++notificationCount % 100 == 1) {
                    std::cout << "Received data change notifications (count: " << notificationCount
                              << ")\n";
                }
            },
            [](opcua::IntegerId subId, opcua::IntegerId monId) {
                (void)subId;
                // Delete callback
                std::cout << "Monitored item " << monId << " deleted\n";
            }
        );

        std::cout << "Created " << createResponse.results().size() << " monitored items\n";

        // Check results
        size_t successCount = 0;
        for (const auto& result : createResponse.results()) {
            if (result.statusCode().isGood()) {
                ++successCount;
            }
        }
        std::cout << "- Successful: " << successCount << "\n";
        std::cout << "- Failed: " << (createResponse.results().size() - successCount) << "\n\n";

        // ========== Example 2: Read multiple nodes ==========
        std::cout << "=== Example 2: Reading multiple nodes with automatic chunking ===\n";

        // Prepare read values
        std::vector<opcua::ReadValueId> nodesToRead;
        nodesToRead.reserve(numReads);

        for (size_t i = 0; i < numReads; ++i) {
            const opcua::NodeId nodeId =
                (i % 2 == 0) ? opcua::VariableId::Server_ServerStatus_CurrentTime
                             : opcua::VariableId::Server_ServerStatus_BuildInfo;
            nodesToRead.emplace_back(nodeId, opcua::AttributeId::Value);
        }

        std::cout << "Reading " << nodesToRead.size() << " nodes (will be chunked if needed)...\n";
        
        const uint32_t readLimit = limitClient.getMaxNodesPerRead();
        if (readLimit > 0 && nodesToRead.size() > readLimit) {
            const size_t numChunks = (nodesToRead.size() + readLimit - 1) / readLimit;
            std::cout << "  -> Chunking into " << numChunks << " requests of max " 
                      << readLimit << " items each\n";
        }

        opcua::ReadRequest readRequest(
            opcua::RequestHeader{}, 0.0, opcua::TimestampsToReturn::Neither, nodesToRead
        );

        auto readResponse = limitClient.read(readRequest);

        std::cout << "Read " << readResponse.results().size() << " values\n";
        successCount = 0;
        for (const auto& result : readResponse.results()) {
            if (result.status().isGood()) {
                ++successCount;
            }
        }
        std::cout << "- Successful: " << successCount << "\n";
        std::cout << "- Failed: " << (readResponse.results().size() - successCount) << "\n\n";

        // ========== Example 3: Browse multiple nodes ==========
        std::cout << "=== Example 3: Browsing multiple nodes with automatic chunking ===\n";

        // Prepare browse descriptions
        std::vector<opcua::BrowseDescription> nodesToBrowse;
        nodesToBrowse.reserve(numBrowses);

        for (size_t i = 0; i < numBrowses; ++i) {
            const opcua::NodeId nodeId =
                (i % 2 == 0) ? opcua::ObjectId::ObjectsFolder : opcua::ObjectId::Server;
            nodesToBrowse.emplace_back(
                nodeId,
                opcua::BrowseDirection::Forward,
                opcua::ReferenceTypeId::References,
                true,  // includeSubtypes
                0U,  // nodeClassMask (all)
                opcua::BrowseResultMask::All
            );
        }

        std::cout << "Browsing " << nodesToBrowse.size()
                  << " nodes (will be chunked if needed)...\n";
        
        const uint32_t browseLimit = limitClient.getMaxNodesPerBrowse();
        if (browseLimit > 0 && nodesToBrowse.size() > browseLimit) {
            const size_t numChunks = (nodesToBrowse.size() + browseLimit - 1) / browseLimit;
            std::cout << "  -> Chunking into " << numChunks << " requests of max " 
                      << browseLimit << " items each\n";
        }

        opcua::BrowseRequest browseRequest(
            opcua::RequestHeader{}, opcua::ViewDescription{}, 0U, nodesToBrowse
        );

        auto browseResponse = limitClient.browse(browseRequest);

        std::cout << "Browsed " << browseResponse.results().size() << " nodes\n";
        successCount = 0;
        for (const auto& result : browseResponse.results()) {
            if (result.statusCode().isGood()) {
                ++successCount;
            }
        }
        std::cout << "- Successful: " << successCount << "\n";
        std::cout << "- Failed: " << (browseResponse.results().size() - successCount) << "\n\n";

        // ========== Keep running to receive notifications ==========
        std::cout << "=== Monitoring for 10 seconds ===\n";
        std::cout << "Press Ctrl+C to exit early\n\n";

        std::signal(SIGINT, signalHandler);  // NOLINT

        const auto startTime = std::chrono::steady_clock::now();
        while (isRunning) {
            client.runIterate(100);

            // Exit after 10 seconds
            const auto elapsed = std::chrono::steady_clock::now() - startTime;
            if (elapsed > std::chrono::seconds{10}) {
                break;
            }
        }

        std::cout << "\nDisconnecting...\n";
        client.disconnect();
        std::cout << "Example completed successfully!\n";

    } catch (const opcua::BadStatus& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << "Make sure a OPC UA server is running at opc.tcp://localhost:4840\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
