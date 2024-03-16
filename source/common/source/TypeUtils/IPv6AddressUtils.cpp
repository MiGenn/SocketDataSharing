#include "IndirectIncludes/TypeUtils/IPv6AddressUtils.hpp"
#include "InternalTypeUtils/InternalIPv6AddressUtils.hpp"
#include "ErrorHandler.hpp"

namespace SDS
{
    IPv6Address ToIPv6AddressInHostBO(IPv6Address addressInNetworkBO) noexcept
    {
        IPv6Address addressInHostBO;
        InternalIPv6AddressUtils::ToHostBO(addressInNetworkBO, addressInHostBO);

        return addressInHostBO;
    }

    IPv6Address ToIPv6AddressInNetworkBO(IPv6Address addressInHostBO) noexcept
    {
        IPv6Address addressInNetworkBO;
        InternalIPv6AddressUtils::ToNetworkBO(addressInHostBO, addressInNetworkBO);

        return addressInNetworkBO;
    }

    ErrorBool IsIPv6AddressZero(IPv6Address address) noexcept
    {
        auto isZero = (uint8_t)(InternalIPv6AddressUtils::IsZero(address));
        return reinterpret_cast<ErrorBool&>(++isZero);
    }

    ErrorBool IsIPv6AddressLoopback(IPv6Address address) noexcept
    {
        auto isLoopback = (uint8_t)(InternalIPv6AddressUtils::IsLoopback(address));
        return reinterpret_cast<ErrorBool&>(++isLoopback);
    }

    ErrorBool IsIPv6AddressLoopbackInNetworkBO(IPv6Address addressInNetworkBO) noexcept
    {
        auto isLoopback = (uint8_t)(InternalIPv6AddressUtils::IsLoopbackInNetworkBO(addressInNetworkBO));
        return reinterpret_cast<ErrorBool&>(++isLoopback);
    }

    ErrorBool IsIPv6AddressLinkLocal(IPv6Address address) noexcept
    {
        auto isLinkLocal = (uint8_t)(InternalIPv6AddressUtils::IsLinkLocal(address));
        return reinterpret_cast<ErrorBool&>(++isLinkLocal);
    }

    ErrorBool IsIPv6AddressLinkLocalInNetworkBO(IPv6Address addressInNetworkBO) noexcept
    {
        auto isLinkLocal = (uint8_t)(InternalIPv6AddressUtils::IsLinkLocalInNetworkBO(addressInNetworkBO));
        return reinterpret_cast<ErrorBool&>(++isLinkLocal);
    }

    ErrorBool IsIPv6AddressPrivate(IPv6Address address) noexcept
    {
        auto isPrivate = (uint8_t)(InternalIPv6AddressUtils::IsPrivate(address));
        return reinterpret_cast<ErrorBool&>(++isPrivate);
    }
    
    ErrorBool IsIPv6AddressPrivateInNetworBO(IPv6Address addressInNetworkBO) noexcept
    {
        auto isPrivate = (uint8_t)(InternalIPv6AddressUtils::IsPrivateInNetworkBO(addressInNetworkBO));
        return reinterpret_cast<ErrorBool&>(++isPrivate);
    }
}