#include "IndirectIncludes/TypeUtils/IPv6AddressUtils.hpp"
#include "InternalTypeUtils/InternalIPv6AddressUtils.hpp"
#include "ErrorHandler.hpp"

namespace SDS::IPv6AddressUtils
{
    IPv6Address ToHostBO(const IPv6Address* addressInNetworkBO) noexcept
    {
        if (addressInNetworkBO == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return {};
        }

        IPv6Address addressInHostBO;
        InternalIPv6AddressUtils::ToHostBO(*addressInNetworkBO, addressInHostBO);

        return addressInHostBO;
    }

    IPv6Address ToNetworkBO(const IPv6Address* addressInHostBO) noexcept
    {
        if (addressInHostBO == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return {};
        }

        IPv6Address addressInNetworkBO;
        InternalIPv6AddressUtils::ToNetworkBO(*addressInHostBO, addressInNetworkBO);

        return addressInNetworkBO;
    }

    ErrorBool IsZero(const IPv6Address* address) noexcept
    {
        if (address == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }

        auto isZero = (uint8_t)(InternalIPv6AddressUtils::IsZero(*address));
        return reinterpret_cast<ErrorBool&>(++isZero);
    }

    ErrorBool IsLoopback(const IPv6Address* address) noexcept
    {
        if (address == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }
        
        auto isLoopback = (uint8_t)(InternalIPv6AddressUtils::IsLoopback(*address));
        return reinterpret_cast<ErrorBool&>(++isLoopback);
    }

    ErrorBool IsLoopbackInNetworkBO(const IPv6Address* addressInNetworkBO) noexcept
    {
        if (addressInNetworkBO == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }

        auto isLoopback = (uint8_t)(InternalIPv6AddressUtils::IsLoopbackInNetworkBO(*addressInNetworkBO));
        return reinterpret_cast<ErrorBool&>(++isLoopback);
    }

    ErrorBool IsLinkLocal(const IPv6Address* address) noexcept
    {
        if (address == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }

        auto isLinkLocal = (uint8_t)(InternalIPv6AddressUtils::IsLinkLocal(*address));
        return reinterpret_cast<ErrorBool&>(++isLinkLocal);
    }

    ErrorBool IsLinkLocalInNetworkBO(const IPv6Address* addressInNetworkBO) noexcept
    {
        if (addressInNetworkBO == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }

        auto isLinkLocal = (uint8_t)(InternalIPv6AddressUtils::IsLinkLocalInNetworkBO(*addressInNetworkBO));
        return reinterpret_cast<ErrorBool&>(++isLinkLocal);
    }

    ErrorBool IsPrivate(const IPv6Address* address) noexcept
    {
        if (address == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }

        auto isPrivate = (uint8_t)(InternalIPv6AddressUtils::IsPrivate(*address));
        return reinterpret_cast<ErrorBool&>(++isPrivate);
    }
    
    ErrorBool IsPrivateInNetworBO(const IPv6Address* addressInNetworkBO) noexcept
    {
        if (addressInNetworkBO == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }

        auto isPrivate = (uint8_t)(InternalIPv6AddressUtils::IsPrivateInNetworkBO(*addressInNetworkBO));
        return reinterpret_cast<ErrorBool&>(++isPrivate);
    }
}