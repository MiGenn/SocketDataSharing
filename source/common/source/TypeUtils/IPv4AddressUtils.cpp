#include "IndirectIncludes/TypeUtils/IPv4AddressUtils.hpp"
#include "InternalTypeUtils/InternalIPv4AddressUtils.hpp"
#include "ErrorHandler.hpp"

namespace SDS
{
    ErrorBool IsIPv4AddressZero(IPv4Address address) noexcept
    {
        auto isZero = (uint8_t)(InternalIPv4AddressUtils::IsZero(address));
        return reinterpret_cast<ErrorBool&>(++isZero);
    }

    ErrorBool IsIPv4AddressLoopback(IPv4Address address) noexcept
    {
        auto isLoopback = (uint8_t)(InternalIPv4AddressUtils::IsLoopback(address));
        return reinterpret_cast<ErrorBool&>(++isLoopback);
    }

    ErrorBool IsIPv4AddressLinkLocal(IPv4Address address) noexcept
    {
        auto isLinkLocal = (uint8_t)(InternalIPv4AddressUtils::IsLinkLocal(address));
        return reinterpret_cast<ErrorBool&>(++isLinkLocal);
    }

    ErrorBool IsIPv4AddressPrivate(IPv4Address address) noexcept
    {
        auto isPrivate = (uint8_t)(InternalIPv4AddressUtils::IsPrivate(address));
        return reinterpret_cast<ErrorBool&>(++isPrivate);
    }
}
