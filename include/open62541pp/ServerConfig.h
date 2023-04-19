#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "open62541/server.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeConverter.h"
// FIXME
#include "../../src/open62541_impl.h"

namespace opcua {

template <typename T>
class Builder {
public:
    static T builder() { return {}; }
    T & build() { return static_cast<T&>(*this); }
};

class ServerConfig : public Builder<ServerConfig> {
    friend class Builder<ServerConfig>;

public:
    ServerConfig&& defaults() && {
        port_ = {};
        certificate_ = {};
        sendBufferSize_ = 0;
        recvBufferSize_ = 0;
        return std::move(static_cast<ServerConfig&>(*this));
    }

     ServerConfig&& minimal(int16_t port, std::string_view certificate = {}) && {
        port_ = port;
        certificate_ = certificate;
        sendBufferSize_ = 0;
        recvBufferSize_ = 0;
        return std::move(static_cast<ServerConfig&>(*this));
     }

     ServerConfig&& minimalCustomBuffer(int16_t port, std::string_view certificate = {}, uint32_t sendBufferSize = 0, uint32_t recvBufferSize = 0) && {
        port_ = port;
        certificate_ = certificate;
        sendBufferSize_ = sendBufferSize;
        recvBufferSize_ = recvBufferSize;
        return std::move(static_cast<ServerConfig&>(*this));
     }

    ServerConfig&& port(int16_t port) && {
        port_ = port;
        return std::move(static_cast<ServerConfig&>(*this));
    }

    ServerConfig&& hostname(std::string_view hostname) && {
        hostname_ = hostname;
        return std::move(static_cast<ServerConfig&>(*this));
    }

    ServerConfig&& applicationname(std::string_view applicationname) && {
        applicationname_ = applicationname;
        return std::move(static_cast<ServerConfig&>(*this));
    }

public:
    [[nodiscard]] UA_ServerConfig* toNative() const {
        UA_ServerConfig* serverconfig_ = new UA_ServerConfig();
        UA_ByteString nativeCert{};
        // Extend TypeConverter?
        TypeConverter<std::string>::toNative(std::string(certificate_), nativeCert);
        UA_ServerConfig_setMinimalCustomBuffer(serverconfig_, port_, &nativeCert, sendBufferSize_, recvBufferSize_);

        auto& ref = serverconfig_->customHostname;
        UA_String_clear(&ref);
        ref = detail::allocUaString(hostname_);

        //UA_ServerConfig_addAllEndpoints();
        //UA_ServerConfig_addEndpoint();
        //UA_ServerConfig_addNetworkLayerTCP();
        //UA_ServerConfig_addSecurityPolicyNone();
        //serverconfig_->applicationDescription.applicationName = applicationname_;

        return serverconfig_;
    }

private:
    ServerConfig() = default;

    bool defaults_={false};

    static constexpr const int16_t DEFAULT_PORT{4840};

    int16_t port_{DEFAULT_PORT};

    std::string_view certificate_{};

    uint32_t sendBufferSize_{0};
    uint32_t recvBufferSize_{0};

    std::string_view hostname_;
    std::string_view applicationname_;

    std::string_view locale_{"en-US"};

    // TODO: Deal with open62541 compile options i.e. UA_ENABLE_ENCRYPTION
};

}  // namespace opcua