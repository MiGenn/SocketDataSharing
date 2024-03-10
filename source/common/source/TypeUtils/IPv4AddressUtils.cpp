#include "IndirectIncludes/TypeUtils/IPv4AddressUtils.hpp"
#include "InternalTypeUtils/InternalIPv4AddressUtils.hpp"
#include "ErrorHandler.hpp"

namespace SDS::IPv4AddressUtils
{
    ErrorBool IsZero(const IPv4Address* address) noexcept
    {
        if (address == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }

        auto isZero = (uint8_t)(InternalIPv4AddressUtils::IsZero(*address));
        return reinterpret_cast<ErrorBool&>(++isZero);
    }

    ErrorBool IsLoopback(const IPv4Address* address) noexcept
    {
        if (address == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }
        
        auto isLoopback = (uint8_t)(InternalIPv4AddressUtils::IsLoopback(*address));
        return reinterpret_cast<ErrorBool&>(++isLoopback);
    }

    ErrorBool IsLinkLocal(const IPv4Address* address) noexcept
    {
        if (address == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }

        auto isLinkLocal = (uint8_t)(InternalIPv4AddressUtils::IsLinkLocal(*address));
        return reinterpret_cast<ErrorBool&>(++isLinkLocal);
    }

    ErrorBool IsPrivate(const IPv4Address* address) noexcept
    {
        if (address == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }

        auto isPrivate = (uint8_t)(InternalIPv4AddressUtils::IsPrivate(*address));
        return reinterpret_cast<ErrorBool&>(++isPrivate);
    }
}
