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

void ErrorHandler::Handle_ioctlsocket() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEFAULT); //Invalid arguments

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

void ErrorHandler::Handle_bind() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEFAULT); //Invalid arguments or the passed address family doesn't match the address family of the socket.
    assert(errorCode != WSAEINVAL); //The socket is already bound.

    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAEADDRNOTAVAIL:
        error = Error::UnavailableIPAddress;
        break;

    //The socket address is taken by a socket with SO_EXCLUSIVEADDRUSE option enabled. 
    //It shouldn't happen because the library doesn't use SO_REUSEADDR option.    
    case WSAEACCES:
    case WSAEADDRINUSE:
        error = Error::SocketAddressIsTaken;
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

    assert(errorCode != WSAEWOULDBLOCK); //It's not an error.
    
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

    assert(errorCode != WSAEFAULT); //Invalid arguments.
    assert(errorCode != WSAENETRESET && errorCode != WSAENOTCONN); //SO_KEEPALIVE is set and something happened.
 
    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAEINVAL: //Invalid level for the socket. It can be either the library's fault or the user's fault.
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

void ErrorHandler::Handle_getsockopt() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEFAULT); //Invalid arguments.

    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAEINVAL: //Invalid level for the socket. It can be either the library's fault or the user's fault.
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

void ErrorHandler::Handle_listen() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEINVAL); //Socket isn't bound.

    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAEADDRINUSE: //Happens if the address was set to a zero one in the bind call.
        error = Error::SocketAddressIsTaken;
        break;

    case WSAENOBUFS:
        error = Error::NotEnoughMemory;
        break;

    case WSAEMFILE:
        error = Error::SystemSocketLimitIsReached;
        break;

    case WSAEOPNOTSUPP:
        error = Error::SocketDoesNotSupportListeningMode;
        break;

    case WSAEISCONN:
        error = Error::SocketIsAlreadyConnectedOrConnecting;
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

void ErrorHandler::Handle_accept() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEFAULT); //Invalid arguments.
    assert(errorCode != WSAEWOULDBLOCK); //It's not an error.
    assert(errorCode != WSAECONNRESET); //The other host terminated the connection prior to the accept call.

    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAENOBUFS:
        error = Error::NotEnoughMemory;
        break;

    case WSAEMFILE:
        error = Error::SystemSocketLimitIsReached;
        break;

    case WSAEOPNOTSUPP:
    case WSAEINVAL:
        error = Error::SocketMustBeInListeningMode;
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

void ErrorHandler::Handle_getpeername() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEFAULT); //Invalid arguments.

    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAENOTCONN:
        error = Error::SocketMustBeConnected;
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

void ErrorHandler::Handle_connect() noexcept
{
    const auto errorCode = WSAGetLastError();
    assert(errorCode != 0);

    assert(errorCode != WSAEFAULT); //Invalid arguments.
    assert(errorCode != WSAEWOULDBLOCK); //It's not an error.

    switch (errorCode)
    {
    case WSAENETDOWN:
        error = Error::NetworkSubsystemFailed;
        break;

    case WSAENETUNREACH:
        error = Error::CannotReachNetwork;
        break;

    case WSAEHOSTUNREACH:
        error = Error::CannotReachAnotherHost;
        break;

    case WSAECONNREFUSED:
        error = Error::AnotherHostRejectedConnection;
        break;

    case WSAETIMEDOUT:
        error = Error::CannotEstablishConnection;
        break;

    case WSAEADDRINUSE: //Happens if the address was set to a zero one in the bind call.
        error = Error::SocketAddressIsTaken;
        break;

    case WSAEADDRNOTAVAIL: //Happens if the address to connect to was set to a zero one.
    case WSAEAFNOSUPPORT:
        error = Error::InvalidIPAddress;
        break;

    case WSAEALREADY: //TODO: can probably be removed
    case WSAEISCONN: //TODO: can probably be removed
        error = Error::SocketIsAlreadyConnectedOrConnecting;
        break;

    case WSAEINVAL: //TODO: can probably be removed
        error = Error::SocketIsAlreadyInListeningMode;
        break;

    case WSAEACCES: //TODO: can probably be removed
        error = Error::BroadcastIsNotEnabled;
        break;

    case WSAENOBUFS:
        error = Error::NotEnoughMemory;
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
