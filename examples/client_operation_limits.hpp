#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <vector>

#include "open62541pp/client.hpp"
#include "open62541pp/services/attribute.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"
#include "open62541pp/services/monitoreditem.hpp"
#include "open62541pp/services/view.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/ua/nodeids.hpp"
#include "open62541pp/ua/types.hpp"

namespace opcua {

/**
 * @brief Helper class to make a client respect server operation limits.
 *
 * This decorator wraps an open62541pp Client to automatically respect server operation limits
 * by chunking large batch requests. Supports MonitoredItems, Read, Write, and Browse operations
 * with opt-in limit management.
 *
 * Large requests will be automatically split into chunks to meet the limits set by the server.
 * The limits are automatically requested from the server when first enabled, then cached for reuse.
 * If the server doesn't advertise a limit, a conservative default of 1000 is used.
 *
 * This class does NOT depend on internal open62541pp code (detail:: namespace), so it can be
 * copied to client applications and used there.
 *
 * ## Usage Example
 *
 * ```cpp
 * opcua::Client client;
 * client.connect("opc.tcp://localhost:4840");
 *
 * LimitRespectingClient limitClient(client);
 * limitClient.enableMaxMonitoredItemsPerCall();
 * limitClient.enableMaxNodesPerRead();
 *
 * // Now create 200 monitored items - automatically chunked if server limit is lower
 * auto response = limitClient.createMonitoredItemsDataChange(...);
 * ```
 *
 * ## Threading Considerations
 *
 * This class is not thread-safe. It follows the same threading model as the Client class it wraps.
 * Do not call methods from multiple threads simultaneously.
 *
 * ## Error Handling
 *
 * When chunking operations, this helper continues processing all chunks even if some fail.
 * All results (successful and failed) are aggregated in the response. This allows callers to
 * see partial success and handle errors appropriately.
 */
class LimitRespectingClient {
public:
    /**
     * @brief Construct a limit-respecting client wrapper.
     * @param client Reference to the client to wrap (non-owning).
     */
    explicit LimitRespectingClient(Client& client) noexcept
        : client_(client) {}

    /**
     * @brief Enable and cache MaxMonitoredItemsPerCall limit.
     *
     * Queries the server for the operation limit on the first call, then caches the value.
     * If the server doesn't advertise this limit, uses a conservative default of 1000.
     *
     * @return true if limit was successfully queried or already cached, false on query failure.
     */
    bool enableMaxMonitoredItemsPerCall() {
        return enableLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXMONITOREDITEMSPERCALL)
        );
    }

    /**
     * @brief Enable MaxMonitoredItemsPerCall with a custom limit value.
     *
     * Sets a specific limit value instead of querying from the server.
     *
     * @param limit Custom limit value to use.
     */
    void enableMaxMonitoredItemsPerCall(uint32_t limit) {
        setLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXMONITOREDITEMSPERCALL),
            limit
        );
    }

    /**
     * @brief Get the current MaxMonitoredItemsPerCall limit.
     *
     * @return The limit value, or 0 if not enabled.
     */
    uint32_t getMaxMonitoredItemsPerCall() const {
        return getLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXMONITOREDITEMSPERCALL)
        );
    }

    /**
     * @brief Enable and cache MaxNodesPerRead limit.
     *
     * Queries the server for the operation limit on the first call, then caches the value.
     * If the server doesn't advertise this limit, uses a conservative default of 1000.
     *
     * @return true if limit was successfully queried or already cached, false on query failure.
     */
    bool enableMaxNodesPerRead() {
        return enableLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERREAD)
        );
    }

    /**
     * @brief Enable MaxNodesPerRead with a custom limit value.
     *
     * Sets a specific limit value instead of querying from the server.
     *
     * @param limit Custom limit value to use.
     */
    void enableMaxNodesPerRead(uint32_t limit) {
        setLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERREAD),
            limit
        );
    }

    /**
     * @brief Get the current MaxNodesPerRead limit.
     *
     * @return The limit value, or 0 if not enabled.
     */
    uint32_t getMaxNodesPerRead() const {
        return getLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERREAD)
        );
    }

    /**
     * @brief Enable and cache MaxNodesPerWrite limit.
     *
     * Queries the server for the operation limit on the first call, then caches the value.
     * If the server doesn't advertise this limit, uses a conservative default of 1000.
     *
     * @return true if limit was successfully queried or already cached, false on query failure.
     */
    bool enableMaxNodesPerWrite() {
        return enableLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERWRITE)
        );
    }

    /**
     * @brief Enable MaxNodesPerWrite with a custom limit value.
     *
     * Sets a specific limit value instead of querying from the server.
     *
     * @param limit Custom limit value to use.
     */
    void enableMaxNodesPerWrite(uint32_t limit) {
        setLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERWRITE),
            limit
        );
    }

    /**
     * @brief Get the current MaxNodesPerWrite limit.
     *
     * @return The limit value, or 0 if not enabled.
     */
    uint32_t getMaxNodesPerWrite() const {
        return getLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERWRITE)
        );
    }

    /**
     * @brief Enable and cache MaxNodesPerBrowse limit.
     *
     * Queries the server for the operation limit on the first call, then caches the value.
     * If the server doesn't advertise this limit, uses a conservative default of 1000.
     *
     * @return true if limit was successfully queried or already cached, false on query failure.
     */
    bool enableMaxNodesPerBrowse() {
        return enableLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERBROWSE)
        );
    }

    /**
     * @brief Enable MaxNodesPerBrowse with a custom limit value.
     *
     * Sets a specific limit value instead of querying from the server.
     *
     * @param limit Custom limit value to use.
     */
    void enableMaxNodesPerBrowse(uint32_t limit) {
        setLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERBROWSE),
            limit
        );
    }

    /**
     * @brief Get the current MaxNodesPerBrowse limit.
     *
     * @return The limit value, or 0 if not enabled.
     */
    uint32_t getMaxNodesPerBrowse() const {
        return getLimit(
            NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERBROWSE)
        );
    }

    /**
     * @brief Create and add monitored items to a subscription for data change notifications.
     *
     * Automatically chunks the request based on MaxMonitoredItemsPerCall if that limit is enabled.
     * If the limit is not enabled, delegates directly to services::createMonitoredItemsDataChange.
     *
     * All chunks are processed even if some fail. Results are aggregated in the response.
     *
     * @param request Create monitored items request
     * @param dataChangeCallback Invoked when a monitored item's value changes
     * @param deleteCallback Invoked when a monitored item is deleted
     * @return Aggregated response with all results
     */
    CreateMonitoredItemsResponse createMonitoredItemsDataChange(
        const CreateMonitoredItemsRequest& request,
        services::DataChangeNotificationCallback dataChangeCallback,
        services::DeleteMonitoredItemCallback deleteCallback
    );

    /**
     * @brief Create and add monitored items to a subscription for event notifications.
     *
     * Automatically chunks the request based on MaxMonitoredItemsPerCall if that limit is enabled.
     * If the limit is not enabled, delegates directly to services::createMonitoredItemsEvent.
     *
     * All chunks are processed even if some fail. Results are aggregated in the response.
     *
     * @param request Create monitored items request
     * @param eventCallback Invoked when an event occurs
     * @param deleteCallback Invoked when a monitored item is deleted
     * @return Aggregated response with all results
     */
    CreateMonitoredItemsResponse createMonitoredItemsEvent(
        const CreateMonitoredItemsRequest& request,
        services::EventNotificationCallback eventCallback,
        services::DeleteMonitoredItemCallback deleteCallback
    );

    /**
     * @brief Read one or more attributes of one or more nodes.
     *
     * Automatically chunks the request based on MaxNodesPerRead if that limit is enabled.
     * If the limit is not enabled, delegates directly to services::read.
     *
     * All chunks are processed even if some fail. Results are aggregated in the response.
     *
     * @param request Read request
     * @return Aggregated response with all results
     */
    ReadResponse read(const ReadRequest& request);

    /**
     * @brief Write one or more attributes of one or more nodes.
     *
     * Automatically chunks the request based on MaxNodesPerWrite if that limit is enabled.
     * If the limit is not enabled, delegates directly to services::write.
     *
     * All chunks are processed even if some fail. Results are aggregated in the response.
     *
     * @param request Write request
     * @return Aggregated response with all results
     */
    WriteResponse write(const WriteRequest& request);

    /**
     * @brief Discover the references of one or more nodes.
     *
     * Automatically chunks the request based on MaxNodesPerBrowse if that limit is enabled.
     * If the limit is not enabled, delegates directly to services::browse.
     *
     * All chunks are processed even if some fail. Results are aggregated in the response.
     *
     * @param request Browse request
     * @return Aggregated response with all results
     */
    BrowseResponse browse(const BrowseRequest& request);

private:
    /**
     * @brief Conservative default limit if server doesn't advertise one.
     */
    static constexpr uint32_t kDefaultLimit = 1000;

    /**
     * @brief Enable and cache a specific operation limit.
     * @param limitNodeId NodeId of the operation limit to query
     * @return true if limit was successfully queried or already cached
     */
    bool enableLimit(const NodeId& limitNodeId) {
        // Already cached?
        if (limits_.find(limitNodeId) != limits_.end()) {
            return true;
        }

        // Query from server
        const uint32_t limit = queryLimit(limitNodeId);
        limits_[limitNodeId] = limit;
        return true;
    }

    /**
     * @brief Set a specific operation limit to a custom value.
     * @param limitNodeId NodeId of the operation limit
     * @param limit Custom limit value
     */
    void setLimit(const NodeId& limitNodeId, uint32_t limit) {
        limits_[limitNodeId] = limit;
    }

    /**
     * @brief Query an operation limit from the server.
     *
     * Reads the limit value from the server. If the read fails or the value can't be
     * converted to uint32_t, returns the conservative default of 1000.
     *
     * @param limitNodeId NodeId of the operation limit to query
     * @return The limit value (or default if unavailable)
     */
    uint32_t queryLimit(const NodeId& limitNodeId) {
        auto result = services::readValue(client_, limitNodeId);
        if (!result) {
            return kDefaultLimit;
        }

        // Try to convert to uint32_t
        try {
            return result.value().scalar<uint32_t>();
        } catch (...) {
            // Fallback to default if conversion fails
            return kDefaultLimit;
        }
    }

    /**
     * @brief Get a cached limit value for a specific operation.
     * @param limitNodeId NodeId of the operation limit
     * @return The limit value, or 0 if not enabled (meaning no chunking)
     */
    uint32_t getLimit(const NodeId& limitNodeId) const {
        const auto it = limits_.find(limitNodeId);
        if (it != limits_.end()) {
            return it->second;
        }
        return 0;  // Not enabled - no limit
    }

    /**
     * @brief Chunk a span into multiple smaller spans based on a maximum size.
     * @param span The span to chunk
     * @param maxChunkSize Maximum size of each chunk
     * @return Vector of spans, each with at most maxChunkSize elements
     */
    template <typename T>
    static std::vector<Span<const T>> chunkSpan(Span<const T> span, size_t maxChunkSize) {
        std::vector<Span<const T>> chunks;
        if (maxChunkSize == 0 || span.size() <= maxChunkSize) {
            chunks.push_back(span);
            return chunks;
        }

        size_t offset = 0;
        while (offset < span.size()) {
            const size_t chunkSize = std::min(maxChunkSize, span.size() - offset);
            chunks.emplace_back(span.data() + offset, chunkSize);
            offset += chunkSize;
        }
        return chunks;
    }

    Client& client_;
    std::map<NodeId, uint32_t> limits_;
};

// ========== Implementation ==========

inline CreateMonitoredItemsResponse LimitRespectingClient::createMonitoredItemsDataChange(
    const CreateMonitoredItemsRequest& request,
    services::DataChangeNotificationCallback dataChangeCallback,
    services::DeleteMonitoredItemCallback deleteCallback
) {
    const uint32_t limit = getLimit(
        NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXMONITOREDITEMSPERCALL)
    );

    const auto itemsToCreate = request.itemsToCreate();

    // No limit enabled or items within limit - delegate directly
    if (limit == 0 || itemsToCreate.size() <= limit) {
        return services::createMonitoredItemsDataChange(
            client_, request, dataChangeCallback, deleteCallback
        );
    }

    // Chunk the request
    const auto chunks = chunkSpan(itemsToCreate, limit);

    // Process each chunk and aggregate results
    std::vector<MonitoredItemCreateResult> allResults;
    allResults.reserve(itemsToCreate.size());

    for (const auto& chunk : chunks) {
        // Create chunked request
        const CreateMonitoredItemsRequest chunkRequest(
            request.requestHeader(),
            request.subscriptionId(),
            request.timestampsToReturn(),
            chunk
        );

        // Process chunk
        auto chunkResponse = services::createMonitoredItemsDataChange(
            client_, chunkRequest, dataChangeCallback, deleteCallback
        );

        // Aggregate results
        const auto chunkResults = chunkResponse.results();
        for (const auto& result : chunkResults) {
            allResults.emplace_back(result);
        }
    }

    // Build aggregated response
    CreateMonitoredItemsResponse response;
    // Note: We can't easily reconstruct the full response with proper ResponseHeader,
    // so we create a minimal response with the aggregated results.
    // The native handle will need to be properly set up.
    auto* handle = response.handle();
    handle->resultsSize = allResults.size();
    handle->results = static_cast<UA_MonitoredItemCreateResult*>(
        UA_Array_new(allResults.size(), &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT])
    );
    
    for (size_t i = 0; i < allResults.size(); ++i) {
        UA_copy(
            allResults[i].handle(),
            &handle->results[i],
            &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]
        );
    }

    return response;
}

inline CreateMonitoredItemsResponse LimitRespectingClient::createMonitoredItemsEvent(
    const CreateMonitoredItemsRequest& request,
    services::EventNotificationCallback eventCallback,
    services::DeleteMonitoredItemCallback deleteCallback
) {
    const uint32_t limit = getLimit(
        NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXMONITOREDITEMSPERCALL)
    );

    const auto itemsToCreate = request.itemsToCreate();

    // No limit enabled or items within limit - delegate directly
    if (limit == 0 || itemsToCreate.size() <= limit) {
        return services::createMonitoredItemsEvent(
            client_, request, eventCallback, deleteCallback
        );
    }

    // Chunk the request
    const auto chunks = chunkSpan(itemsToCreate, limit);

    // Process each chunk and aggregate results
    std::vector<MonitoredItemCreateResult> allResults;
    allResults.reserve(itemsToCreate.size());

    for (const auto& chunk : chunks) {
        // Create chunked request
        const CreateMonitoredItemsRequest chunkRequest(
            request.requestHeader(),
            request.subscriptionId(),
            request.timestampsToReturn(),
            chunk
        );

        // Process chunk
        auto chunkResponse = services::createMonitoredItemsEvent(
            client_, chunkRequest, eventCallback, deleteCallback
        );

        // Aggregate results
        const auto chunkResults = chunkResponse.results();
        for (const auto& result : chunkResults) {
            allResults.emplace_back(result);
        }
    }

    // Build aggregated response
    CreateMonitoredItemsResponse response;
    auto* handle = response.handle();
    handle->resultsSize = allResults.size();
    handle->results = static_cast<UA_MonitoredItemCreateResult*>(
        UA_Array_new(allResults.size(), &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT])
    );
    
    for (size_t i = 0; i < allResults.size(); ++i) {
        UA_copy(
            allResults[i].handle(),
            &handle->results[i],
            &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]
        );
    }

    return response;
}

inline ReadResponse LimitRespectingClient::read(const ReadRequest& request) {
    const uint32_t limit = getLimit(
        NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERREAD)
    );

    const auto nodesToRead = request.nodesToRead();

    // No limit enabled or nodes within limit - delegate directly
    if (limit == 0 || nodesToRead.size() <= limit) {
        return services::read(client_, request);
    }

    // Chunk the request
    const auto chunks = chunkSpan(nodesToRead, limit);

    // Process each chunk and aggregate results
    std::vector<DataValue> allResults;
    allResults.reserve(nodesToRead.size());

    for (const auto& chunk : chunks) {
        // Create chunked request
        const ReadRequest chunkRequest(
            request.requestHeader(), request.maxAge(), request.timestampsToReturn(), chunk
        );

        // Process chunk
        auto chunkResponse = services::read(client_, chunkRequest);

        // Aggregate results
        const auto chunkResults = chunkResponse.results();
        for (const auto& result : chunkResults) {
            allResults.emplace_back(result);
        }
    }

    // Build aggregated response
    ReadResponse response;
    auto* handle = response.handle();
    handle->resultsSize = allResults.size();
    handle->results = static_cast<UA_DataValue*>(
        UA_Array_new(allResults.size(), &UA_TYPES[UA_TYPES_DATAVALUE])
    );
    
    for (size_t i = 0; i < allResults.size(); ++i) {
        UA_copy(allResults[i].handle(), &handle->results[i], &UA_TYPES[UA_TYPES_DATAVALUE]);
    }

    return response;
}

inline WriteResponse LimitRespectingClient::write(const WriteRequest& request) {
    const uint32_t limit = getLimit(
        NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERWRITE)
    );

    const auto nodesToWrite = request.nodesToWrite();

    // No limit enabled or nodes within limit - delegate directly
    if (limit == 0 || nodesToWrite.size() <= limit) {
        return services::write(client_, request);
    }

    // Chunk the request
    const auto chunks = chunkSpan(nodesToWrite, limit);

    // Process each chunk and aggregate results
    std::vector<StatusCode> allResults;
    allResults.reserve(nodesToWrite.size());

    for (const auto& chunk : chunks) {
        // Create chunked request
        const WriteRequest chunkRequest(request.requestHeader(), chunk);

        // Process chunk
        auto chunkResponse = services::write(client_, chunkRequest);

        // Aggregate results
        const auto chunkResults = chunkResponse.results();
        for (const auto& result : chunkResults) {
            allResults.emplace_back(result);
        }
    }

    // Build aggregated response
    WriteResponse response;
    auto* handle = response.handle();
    handle->resultsSize = allResults.size();
    handle->results = static_cast<UA_StatusCode*>(
        UA_Array_new(allResults.size(), &UA_TYPES[UA_TYPES_STATUSCODE])
    );
    
    for (size_t i = 0; i < allResults.size(); ++i) {
        UA_copy(allResults[i].handle(), &handle->results[i], &UA_TYPES[UA_TYPES_STATUSCODE]);
    }

    return response;
}

inline BrowseResponse LimitRespectingClient::browse(const BrowseRequest& request) {
    const uint32_t limit = getLimit(
        NodeId(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERBROWSE)
    );

    const auto nodesToBrowse = request.nodesToBrowse();

    // No limit enabled or nodes within limit - delegate directly
    if (limit == 0 || nodesToBrowse.size() <= limit) {
        return services::browse(client_, request);
    }

    // Chunk the request
    const auto chunks = chunkSpan(nodesToBrowse, limit);

    // Process each chunk and aggregate results
    std::vector<BrowseResult> allResults;
    allResults.reserve(nodesToBrowse.size());

    for (const auto& chunk : chunks) {
        // Create chunked request
        const BrowseRequest chunkRequest(
            request.requestHeader(),
            request.view(),
            request.requestedMaxReferencesPerNode(),
            chunk
        );

        // Process chunk
        auto chunkResponse = services::browse(client_, request);

        // Aggregate results
        const auto chunkResults = chunkResponse.results();
        for (const auto& result : chunkResults) {
            allResults.emplace_back(result);
        }
    }

    // Build aggregated response
    BrowseResponse response;
    auto* handle = response.handle();
    handle->resultsSize = allResults.size();
    handle->results = static_cast<UA_BrowseResult*>(
        UA_Array_new(allResults.size(), &UA_TYPES[UA_TYPES_BROWSERESULT])
    );
    
    for (size_t i = 0; i < allResults.size(); ++i) {
        UA_copy(allResults[i].handle(), &handle->results[i], &UA_TYPES[UA_TYPES_BROWSERESULT]);
    }

    return response;
}

}  // namespace opcua
