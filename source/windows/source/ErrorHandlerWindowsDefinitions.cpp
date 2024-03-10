#include "ErrorHandler.hpp"
#include <cassert>

#define CALL_CALLBACK m_callback(error, (int64_t)errorCode, m_callbackContext)

using namespace SDS;

static Error error = Error::Success;

void ErrorHandler::Handle_WSAStartup(int errorCode) noexcept
{
    assert(errorCode != 0);
    assert(errorCode != WSAEFAULT);

    switch (errorCode)
    {
    case WSASYSNOTREADY:
        error = Error::NetworkSubsystemIsUnavailable;
        break;

    case WSAEPROCLIM:
        error = Error::TooManyApplicationsAreUsingSystemLibrary;
        break;

    default:
        error = Error::UnexpectedSystemError;
    }

    CALL_CALLBACK;
}

void ErrorHandler::Handle_WSACleanup() noexcept
{
    const int errorCode = WSAGetLastError();
    assert(errorCode != 0);

    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkIsDown;
        break;

    default:
        error = Error::UnexpectedSystemError;
    }

    CALL_CALLBACK;
}

void ErrorHandler::Handle_GetAdaptersAddresses(ULONG errorCode) noexcept
{
    assert(errorCode != NO_ERROR);
    assert(errorCode != ERROR_BUFFER_OVERFLOW);
    assert(errorCode != ERROR_INVALID_PARAMETER);
    assert(errorCode != ERROR_NO_DATA);
    assert(errorCode != ERROR_ADDRESS_NOT_ASSOCIATED);

    switch (errorCode)
    {
    case ERROR_NOT_ENOUGH_MEMORY:
        error = Error::NotEnoughMemory;
        break;

    default:
        error = Error::UnexpectedSystemError;
    }

    CALL_CALLBACK;
}