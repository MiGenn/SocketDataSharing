#include "ErrorHandler.hpp"
#include <cassert>

#define CALL_CALLBACK m_callback(error, (int64_t)errorCode, m_callbackContext)

using namespace SDS;

static Error error = Error::Success;

void ErrorHandler::Handle_WSAStartup(int errorCode) noexcept
{
    assert(errorCode != 0);
    assert(errorCode != WSAEFAULT); //Invalid arguments

    switch (errorCode)
    {
    case WSASYSNOTREADY:
        error = Error::NetworkSubsystemIsUnavailable;
        break;

    case WSAEPROCLIM:
        error = Error::TooManyApplicationsAreUsingSystemLibrary;
        break;

    case WSAVERNOTSUPPORTED:
        error = Error::NotSupportedMachine;
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
        error = Error::NetworkSubsystemFailed;
        break;

    case WSANOTINITIALISED:
        error = Error::IsNotInitialized;
        break;

    default:
        error = Error::UnexpectedSystemError;
    }

    WSASetLastError(0);
    CALL_CALLBACK;
}

void ErrorHandler::Handle_WSAEnumProtocols() noexcept
{
    const int errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEFAULT && errorCode != WSAEINVAL); //Invalid arguments.
    assert(errorCode != WSAENOBUFS); //The buffer is too small.

    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSANOTINITIALISED:
        error = Error::IsNotInitialized;
        break;

    default:
        error = Error::UnexpectedSystemError;
    }

    WSASetLastError(0);
    CALL_CALLBACK;
}

void ErrorHandler::Handle_GetAdaptersAddresses(ULONG errorCode) noexcept
{
    assert(errorCode != NO_ERROR);
    assert(errorCode != ERROR_BUFFER_OVERFLOW);
    assert(errorCode != ERROR_INVALID_PARAMETER); //Invalid arguments
    assert(errorCode != ERROR_NO_DATA);
    assert(errorCode != ERROR_ADDRESS_NOT_ASSOCIATED); //Treat the same as ERROR_NO_DATA.

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

void ErrorHandler::Handle_socket(int addressFamily, int socketType, int protocol) noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEINVAL); //Invalid arguments
    assert(errorCode != WSAEPROTOTYPE); //Invalid arguments

    assert(addressFamily == AF_INET && socketType == SOCK_STREAM && protocol == IPPROTO_TCP ||
        addressFamily == AF_INET && socketType == SOCK_DGRAM && protocol == IPPROTO_UDP ||
        addressFamily == AF_INET6 && socketType == SOCK_STREAM && protocol == IPPROTO_TCP ||
        addressFamily == AF_INET6 && socketType == SOCK_DGRAM && protocol == IPPROTO_UDP);

    switch (errorCode)
    {
    case WSAENETDOWN: 
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAEPROVIDERFAILEDINIT: 
    case WSAEINVALIDPROCTABLE: 
    case WSAEINVALIDPROVIDER:
        error = Error::ServiceProviderFailed;
        break;

    case WSAEMFILE:
        error = Error::SystemSocketLimitIsReached;
        break;

    case WSAENOBUFS:
        error = Error::NotEnoughMemory;
        break;

    case WSAEAFNOSUPPORT:
        switch (addressFamily)
        {
        case AF_INET6:
            error = Error::IPv6IsNotSupported;
            break;

        default:
            error = Error::IPv4IsNotSupported;
        }
        break;

    case WSAESOCKTNOSUPPORT:
    case WSAEPROTONOSUPPORT:
        if (addressFamily == AF_INET && socketType == SOCK_STREAM && protocol == IPPROTO_TCP)
            error = Error::IPv4TCPIsNotSupported;
        else if (addressFamily == AF_INET && socketType == SOCK_DGRAM && protocol == IPPROTO_UDP)
            error = Error::IPv4UDPIsNotSupported;
        else if (addressFamily == AF_INET6 && socketType == SOCK_STREAM && protocol == IPPROTO_TCP)
            error = Error::IPv6TCPIsNotSupported;
        else
            error = Error::IPv6UDPIsNotSupported;
        break;

    case WSANOTINITIALISED:
        error = Error::IsNotInitialized;
        break;

    default:
        error = Error::UnexpectedSystemError;
    }

    WSASetLastError(0);
    CALL_CALLBACK;
}

void ErrorHandler::Handle_bind() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEFAULT); //Invalid arguments or the passed address family doesn't match the address family of the socket.
    assert(errorCode != WSAEACCES); //TODO: write a clarifying comment.
    assert(errorCode != WSAEINVAL); //The socket is already bound.

    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAEADDRNOTAVAIL:
        error = Error::UnavailableIPAddress;
        break;

    case WSAEADDRINUSE:
        error = Error::TCPEndpointIsTakenOrInWaitState;
        break;

    case WSAENOBUFS:
        error = Error::AllDynamicPortsAreTaken;
        break;

    case WSAENOTSOCK:
        error = Error::InvalidSocketHandle;
        break;

    case WSANOTINITIALISED:
        error = Error::IsNotInitialized;
        break;

    default:
        error = Error::UnexpectedSystemError;
    }

    WSASetLastError(0);
    CALL_CALLBACK;
}

void ErrorHandler::Handle_getsockname() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEFAULT); //Invalid arguments.
    assert(errorCode != WSAEINVAL); //Socket isn't bound or bound with a zero address.

    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAENOTSOCK:
        error = Error::InvalidSocketHandle;
        break;

    case WSANOTINITIALISED:
        error = Error::IsNotInitialized;
        break;

    default:
        error = Error::UnexpectedSystemError;
    }

    WSASetLastError(0);
    CALL_CALLBACK;
}

void ErrorHandler::Handle_closesocket() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEWOULDBLOCK); //TODO: write a clarifying comment.
    
    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAENOTSOCK:
        error = Error::InvalidSocketHandle;
        break;

    case WSANOTINITIALISED:
        error = Error::IsNotInitialized;
        break;

    default:
        error = Error::UnexpectedSystemError;
    }

    WSASetLastError(0);
    CALL_CALLBACK;
}

void ErrorHandler::Handle_setsockopt() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEFAULT && errorCode != WSAEINVAL); //Invalid arguments.
    assert(errorCode != WSAENETRESET && errorCode != WSAENOTCONN); //SO_KEEPALIVE is set and somethin happened.
 
    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAENOPROTOOPT:
        error = Error::UnsupportedSocketOption;
        break;

    case WSAENOTSOCK:
        error = Error::InvalidSocketHandle;
        break;

    case WSANOTINITIALISED:
        error = Error::IsNotInitialized;
        break;

    default:
        error = Error::UnexpectedSystemError;
    }

    WSASetLastError(0);
    CALL_CALLBACK;
}
