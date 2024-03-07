#include "ErrorHandler.hpp"
#include <cassert>

#define CALL_CALLBACK m_callback(error, (intmax_t)errorCode, m_callbackContext)

using namespace SDS;

static Error error = Error::Success;

void ErrorHandler::Handle_WSAStartup(int errorCode) noexcept
{
    assert(errorCode != 0);

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