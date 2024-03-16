#include "SocketDataSharing.hpp"
#include "WinAPI.hpp"
#include "State.hpp"
#include "ErrorHandler.hpp"
#include "InternalTypeUtils.hpp"
#include "InternalEndiannessConversions.hpp"
#include "Utilities/Buffer.hpp"
#include <utility>
#include <vector>
#include <cassert>

namespace SDS
{
    inline static std::pair<WSAPROTOCOL_INFOW*, int> _GetAvailableProtocols() noexcept;
    inline static std::pair<bool, IP_ADAPTER_ADDRESSES*> _GetIPAdapters() noexcept;
    inline static bool _SetNetworkIPAddressesFromIPAdapter(
        const IP_ADAPTER_ADDRESSES& ipAdapter, NetworkIPAddresses& networkIPAddresses_out) noexcept;
    inline static const void* _ChooseBestIPAddressInNetworkBO(const IPv4Address& ipv4Address, const IPv6Address& ipv6Address) noexcept;
    inline static SocketHandle _CreateAndBindIPv4Socket(int type, int protocol, 
        IPv4Address ipv4Address, uint16_t& portNumberInHostBO_inout) noexcept;
    inline static SocketHandle _CreateAndBindIPv6Socket(int type, int protocol,
        const IPv6Address& ipv6AddressInNetworkBO, uint16_t& portNumberInHostBO_inout) noexcept;
    inline static SocketHandle _CreateAndBindIPSocket(int type, int protocol, 
        sockaddr& socketAddressInNetworkBO_inout, int socketAddressSize, bool shouldUpdatePortNumber = false) noexcept;
    inline static bool _BindIPSocket(SOCKET socketToBind, sockaddr& socketAddressInNetworkBO_inout, 
        int socketAddressSize, bool shouldUpdatePortNumber = false) noexcept;
    inline static SocketHandle _CreateAndConnectIPTCPSocket(uint16_t portNumberToConnectFromInHostBO,
        const sockaddr& socketAddressInNetworkBO, int socketAddressSize) noexcept;

    inline static SocketHandle ToSocketHandle(SOCKET nativeSocketHandle) noexcept
    {
        return reinterpret_cast<SocketHandle>(++nativeSocketHandle);
    }

    inline static SOCKET ToNativeSocketHandle(SocketHandle socketHandle) noexcept
    {
        auto nativeSocketHandle = reinterpret_cast<SOCKET>(socketHandle);
        return --nativeSocketHandle;
    }

    ErrorIndicator Initialize() noexcept
    {
        if (State::isInitialized)
        {
            ErrorHandler::SignalError(Error::IsAlreadyInitialized);
            return ErrorIndicator::Error;
        }

        WSADATA wsaData;
        static constexpr auto requstedVersion = MAKEWORD(2, 2);
        if (const int errorCode = WSAStartup(requstedVersion, &wsaData);
            errorCode != 0)
        {
            ErrorHandler::Handle_WSAStartup(errorCode);
            return ErrorIndicator::Error;
        }
        
        //Should never occur.
        if (wsaData.wVersion != requstedVersion)
        {
            WSACleanup();
            ErrorHandler::SignalError(Error::NotSupportedMachine);
            return ErrorIndicator::Error;
        }

        State::isInitialized = true;
        return (ErrorIndicator)1;
    }

    ErrorIndicator Shutdown() noexcept
    {
        if (!State::isInitialized)
        {
            ErrorHandler::SignalError(Error::IsNotInitialized);
            ErrorIndicator::Error;
        }

        //WSACleanup automatically closes all sockets.
        if (WSACleanup() != 0)
        {
            ErrorHandler::Handle_WSACleanup();
            ErrorIndicator::Error;
        }

        State::isInitialized = false;
        return (ErrorIndicator)1;
    }

    ErrorSupportedProtocols EnumerateSupportedProtocols() noexcept
    {
        ErrorSupportedProtocols supportedProtocols{};

        auto [protocolInfoArray, protocolInfoArraySize] = _GetAvailableProtocols();
        if (protocolInfoArray != nullptr)
        {
            for (auto i = 0; i < protocolInfoArraySize; ++i)
            {
                if (protocolInfoArray[i].iAddressFamily == AF_INET && protocolInfoArray[i].iSocketType == SOCK_STREAM &&
                    protocolInfoArray[i].iProtocol == IPPROTO_TCP)
                {
                    supportedProtocols.isIPv4TCPSupported = Bool::True;
                }
                else if (protocolInfoArray[i].iAddressFamily == AF_INET && protocolInfoArray[i].iSocketType == SOCK_DGRAM &&
                    protocolInfoArray[i].iProtocol == IPPROTO_UDP)
                {
                    supportedProtocols.isIPv4UDPSupported = Bool::True;
                }
                else if (protocolInfoArray[i].iAddressFamily == AF_INET6 && protocolInfoArray[i].iSocketType == SOCK_STREAM &&
                    protocolInfoArray[i].iProtocol == IPPROTO_TCP)
                {
                    supportedProtocols.isIPv6TCPSupported = Bool::True;
                }
                else if (protocolInfoArray[i].iAddressFamily == AF_INET6 && protocolInfoArray[i].iSocketType == SOCK_DGRAM &&
                    protocolInfoArray[i].iProtocol == IPPROTO_UDP)
                {
                    supportedProtocols.isIPv6UDPSupported = Bool::True;
                }
            }

            supportedProtocols.errorIndicator = (ErrorIndicator)1;
        }

        return supportedProtocols;
    }    

    NetworkIPAddresses* GetNetworkIPAddressesArray(int32_t* size_out) noexcept
    {
        if (!State::isInitialized) //It's not necessary to do this check.
        {
            ErrorHandler::SignalError(Error::IsNotInitialized);
            return nullptr;
        }

        if (size_out == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return nullptr;
        }

        auto [hasGetIPAdaptersSucceeded, ipAdapters] = _GetIPAdapters();
        if (hasGetIPAdaptersSucceeded)
        {
            try
            {
                static std::vector<NetworkIPAddresses> networkIPAddresses(2);
                networkIPAddresses.clear();

                NetworkIPAddresses networkIPAddress;

                IP_ADAPTER_ADDRESSES* nextIPAdapter = ipAdapters;
                while (nextIPAdapter != nullptr)
                {
                    if (nextIPAdapter->IfType != (IFTYPE)24) //Ignore loopback adapters.
                        if (_SetNetworkIPAddressesFromIPAdapter(*nextIPAdapter, networkIPAddress))
                            networkIPAddresses.emplace_back(networkIPAddress);

                    nextIPAdapter = nextIPAdapter->Next;
                }

                *size_out = networkIPAddresses.size();
                return networkIPAddresses.data();
            }
            catch (...)
            {
                ErrorHandler::SignalError(Error::NotEnoughMemory);
            }
        }

        *size_out = (int32_t)0;
        return nullptr;
    }

    ErrorBool IsIPv4AddressPreferred(const NetworkIPAddresses* networkIPAddressesInNetworkBO) noexcept
    {
        if (!State::isInitialized) //It's not necessary to do this check.
        {
            ErrorHandler::SignalError(Error::IsNotInitialized);
            return ErrorBool::Error;
        }

        if (networkIPAddressesInNetworkBO == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorBool::Error;
        }

        const auto* const bestIPAddressPointer = _ChooseBestIPAddressInNetworkBO(
            networkIPAddressesInNetworkBO->v4, networkIPAddressesInNetworkBO->v6);
        if (bestIPAddressPointer == &networkIPAddressesInNetworkBO->v4)
            return ErrorBool::True;

        return ErrorBool::False;
    }

    SocketHandle CreateIPv4UDPSocket(IPv4Address ipv4Address, uint16_t* portNumberInHostBO_inout) noexcept
    {
        if (InternalIPv4AddressUtils::IsZero(ipv4Address))
        {
            ErrorHandler::SignalError(Error::InvalidIPAddress);
            return nullptr;
        }

        if (portNumberInHostBO_inout == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return nullptr;
        }

        return _CreateAndBindIPv4Socket(SOCK_DGRAM, IPPROTO_UDP, ipv4Address, *portNumberInHostBO_inout);
    }
   
    SocketHandle CreateIPv6UDPSocket(IPv6Address ipv6AddressInNetworkBO, uint16_t* portNumberInHostBO_inout) noexcept
    {
        if (InternalIPv6AddressUtils::IsZero(ipv6AddressInNetworkBO))
        {
            ErrorHandler::SignalError(Error::InvalidIPAddress);
            return nullptr;
        }

        if (portNumberInHostBO_inout == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return nullptr;
        }

        return _CreateAndBindIPv6Socket(SOCK_DGRAM, IPPROTO_UDP, ipv6AddressInNetworkBO, *portNumberInHostBO_inout);
    }

    SocketHandle CreateListeningIPv4TCPSocket(IPv4Address ipv4Address, 
        uint16_t* portNumberInHostBO_inout, uint32_t pendingConnectionQueueSize) noexcept
    {
        if (InternalIPv4AddressUtils::IsZero(ipv4Address))
        {
            ErrorHandler::SignalError(Error::InvalidIPAddress);
            return nullptr;
        }

        if (portNumberInHostBO_inout == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return nullptr;
        }

        auto listeningSocketHandle = _CreateAndBindIPv4Socket(SOCK_STREAM, IPPROTO_TCP, ipv4Address, *portNumberInHostBO_inout);
        if (listeningSocketHandle != nullptr)
        {
            pendingConnectionQueueSize &= 0x7FFFFFFF;
            if (listen(ToNativeSocketHandle(listeningSocketHandle), (int)pendingConnectionQueueSize) != 0)
            {
                ErrorHandler::Handle_listen();
                closesocket(ToNativeSocketHandle(listeningSocketHandle)); //In this context, it doesn't matter if it fails. 
                WSASetLastError(0);
                listeningSocketHandle = nullptr;
            }
        }

        return listeningSocketHandle;
    }

    SocketHandle CreateListeningIPv6TCPSocket(IPv6Address ipv6AddressInNetworkBO, 
        uint16_t* portNumberInHostBO_inout, uint32_t pendingConnectionQueueSize) noexcept
    {
        if (InternalIPv6AddressUtils::IsZero(ipv6AddressInNetworkBO))
        {
            ErrorHandler::SignalError(Error::InvalidIPAddress);
            return nullptr;
        }

        if (portNumberInHostBO_inout == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return nullptr;
        }

        auto listeningSocketHandle = _CreateAndBindIPv6Socket(SOCK_STREAM, IPPROTO_TCP, ipv6AddressInNetworkBO, *portNumberInHostBO_inout);
        if (listeningSocketHandle != nullptr)
        {
            pendingConnectionQueueSize &= 0x7FFFFFFF;
            if (listen(ToNativeSocketHandle(listeningSocketHandle), (int)pendingConnectionQueueSize) != 0)
            {
                ErrorHandler::Handle_listen();
                closesocket(ToNativeSocketHandle(listeningSocketHandle)); //In this context, it doesn't matter if it fails.
                WSASetLastError(0);
                listeningSocketHandle = nullptr;
            }
        }

        return listeningSocketHandle;
    }

    SocketHandle CreateConnectedIPv4TCPSocket(uint16_t portNumberToConnectFromInHostBO, 
        IPv4Address ipv4AddressToConnectTo, uint16_t portNumberToConnectToInHostBO) noexcept
    {
        if (portNumberToConnectToInHostBO == (uint16_t)0)
        {
            ErrorHandler::SignalError(Error::PortNumberIsInvalid);
            return nullptr;
        }

        sockaddr_in socketAddressToConnectTo;
        socketAddressToConnectTo.sin_family = AF_INET;
        socketAddressToConnectTo.sin_port = HostToNetworkBO(portNumberToConnectToInHostBO);
        InternalIPv4AddressUtils::CopyTo(&socketAddressToConnectTo.sin_addr, ipv4AddressToConnectTo);

        return _CreateAndConnectIPTCPSocket(portNumberToConnectFromInHostBO, 
            reinterpret_cast<sockaddr&>(socketAddressToConnectTo), sizeof(sockaddr_in));
    }

    SocketHandle CreateConnectedIPv6TCPSocket(uint16_t portNumberToConnectFromInHostBO,
        IPv6Address ipv6AddressToConnectToInHostBO, uint16_t portNumberToConnectToInHostBO) noexcept
    {
        if (portNumberToConnectToInHostBO == (uint16_t)0)
        {
            ErrorHandler::SignalError(Error::PortNumberIsInvalid);
            return nullptr;
        }

        InternalIPv6AddressUtils::ToNetworkBO(ipv6AddressToConnectToInHostBO, ipv6AddressToConnectToInHostBO);

        sockaddr_in6 socketAddressToConnectTo;
        socketAddressToConnectTo.sin6_family = AF_INET6;
        socketAddressToConnectTo.sin6_port = HostToNetworkBO(portNumberToConnectToInHostBO);
        socketAddressToConnectTo.sin6_flowinfo = ipv6AddressToConnectToInHostBO.flowInfo;
        InternalIPv6AddressUtils::CopyTo(&socketAddressToConnectTo.sin6_addr, ipv6AddressToConnectToInHostBO);
        socketAddressToConnectTo.sin6_scope_id = (ULONG)0;

        return _CreateAndConnectIPTCPSocket(portNumberToConnectFromInHostBO, 
            reinterpret_cast<sockaddr&>(socketAddressToConnectTo), sizeof(sockaddr_in6));
    }

    ErrorIndicator AcceptNewConnection(SocketHandle listeningSocketHandle, SocketHandle* connectedSocketHandle_out) noexcept
    {
        if (connectedSocketHandle_out == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return ErrorIndicator::Error;
        }

        auto newConnection = accept(ToNativeSocketHandle(listeningSocketHandle), nullptr, nullptr);
        if (newConnection == INVALID_SOCKET)
        {
            const int errorCode = WSAGetLastError();
            if (errorCode != WSAEWOULDBLOCK && errorCode != WSAECONNRESET)
            {
                ErrorHandler::Handle_accept();
                return ErrorIndicator::Error;
            }
        }

        *connectedSocketHandle_out = ToSocketHandle(newConnection);
        return (ErrorIndicator)1;
    }

    ErrorIPSocketAddress GetAnotherHostIPSocketAddress(SocketHandle connectedSocketHandle) noexcept
    {
        ErrorIPSocketAddress errorIPSocketAddress{};

        sockaddr_in6 socketAddress; //Used as a buffer for any IP address family.
        auto socketAddressSize = (int)sizeof(sockaddr_in6);
        if (getpeername(ToNativeSocketHandle(connectedSocketHandle), reinterpret_cast<sockaddr*>(&socketAddress), &socketAddressSize) != 0)
        {
            if (WSAGetLastError() != WSAEFAULT)
                ErrorHandler::Handle_getpeername();
            else
                ErrorHandler::SignalError(Error::AnotherHostUsesIncompatibleSocketAddress);

            return errorIPSocketAddress;
        }

        errorIPSocketAddress.errorIndicator = (ErrorIndicator)1;
        errorIPSocketAddress.port = socketAddress.sin6_port;
        
        if (socketAddress.sin6_family == AF_INET)
        {
            InternalIPv4AddressUtils::CopyFrom(&socketAddress.sin6_flowinfo, errorIPSocketAddress.v4);
        }
        else if (socketAddress.sin6_family == AF_INET6)
        {
            InternalIPv6AddressUtils::CopyFrom(&socketAddress.sin6_addr, errorIPSocketAddress.v6);
            errorIPSocketAddress.v6.flowInfo = socketAddress.sin6_flowinfo;
            errorIPSocketAddress.v6.scopeID = socketAddress.sin6_scope_id;
        }
        else
        {
            ErrorHandler::SignalError(Error::AnotherHostUsesIncompatibleSocketAddress);
            errorIPSocketAddress.errorIndicator = ErrorIndicator::Error;
        }

        return errorIPSocketAddress;
    }

    ErrorIndicator DestroySocket(SocketHandle socketHandle) noexcept
    {
        if (closesocket(ToNativeSocketHandle(socketHandle)) != 0 && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            ErrorHandler::Handle_closesocket();
            return ErrorIndicator::Error;
        }

        return (ErrorIndicator)1;
    }

    ErrorIndicator SetTCPSocketNaglesAlgorithm(SocketHandle socketHandle, Bool isEnabled) noexcept
    {
        if (isEnabled == Bool::False)
            isEnabled = Bool::True;
        else
            isEnabled = Bool::False;

        const auto optionValue = (DWORD)isEnabled;
        if (setsockopt(ToNativeSocketHandle(socketHandle), IPPROTO_TCP, TCP_NODELAY,
                reinterpret_cast<const char*>(&isEnabled), (int)sizeof(DWORD)) != 0)
        {
            ErrorHandler::Handle_setsockopt();
            return ErrorIndicator::Error;
        }

        return (ErrorIndicator)1;
    }
    
    ErrorIndicator SetSocketDestructionTimeout(SocketHandle socketHandle, Bool isEnabled, uint16_t timeInSeconds) noexcept
    {
        const linger optionValue{ (u_short)isEnabled, (u_short)timeInSeconds };
        if (setsockopt(ToNativeSocketHandle(socketHandle), SOL_SOCKET, SO_LINGER,
                reinterpret_cast<const char*>(&optionValue), (int)sizeof(linger)) != 0)
        {
            ErrorHandler::Handle_setsockopt();
            return ErrorIndicator::Error;
        }

        return (ErrorIndicator)1;
    }

    ErrorIndicator SetSocketBroadcast(SocketHandle socketHandle, Bool isEnabled) noexcept
    {
        const auto optionValue = (BOOL)isEnabled;
        if (setsockopt(ToNativeSocketHandle(socketHandle), SOL_SOCKET, SO_BROADCAST,
                reinterpret_cast<const char*>(&optionValue), (int)sizeof(BOOL)) != 0)
        {
            ErrorHandler::Handle_setsockopt();
            return ErrorIndicator::Error;
        }

        return (ErrorIndicator)1;
    }

    //The returned pointer is null only if an error occured.
    //The returned int value is used to store the protocol info array's size.
    inline std::pair<WSAPROTOCOL_INFOW*, int> _GetAvailableProtocols() noexcept
    {
        try
        {
            static Buffer memoryForProtocolInfos(8192);
            WSAPROTOCOL_INFOW* protocolInfos;
            DWORD errorCodeOrProtocolInfoCount;

            while (true)
            {
                protocolInfos = reinterpret_cast<WSAPROTOCOL_INFOW*>(memoryForProtocolInfos.GetData());
                auto memoryForProtocolInfosSize = (DWORD)memoryForProtocolInfos.GetSize();

                errorCodeOrProtocolInfoCount = WSAEnumProtocolsW(nullptr, protocolInfos, &memoryForProtocolInfosSize);
                if (memoryForProtocolInfosSize > memoryForProtocolInfos.GetSize())
                    memoryForProtocolInfos.Resize(memoryForProtocolInfosSize);
                else
                    break;
            }

            if (errorCodeOrProtocolInfoCount != SOCKET_ERROR)
                return { protocolInfos, errorCodeOrProtocolInfoCount };

            ErrorHandler::Handle_WSAEnumProtocols();
        }
        catch (...)
        {
            ErrorHandler::SignalError(Error::NotEnoughMemory);
        }

        return { nullptr, 0 };
    }

    //The returned bool value is set to false if the function failed.
    //The returned pointer can be null if no data was found.
    inline std::pair<bool, IP_ADAPTER_ADDRESSES*> _GetIPAdapters() noexcept
    {
        static constexpr ULONG flags = GAA_FLAG_SKIP_ANYCAST & GAA_FLAG_SKIP_MULTICAST &
            GAA_FLAG_SKIP_DNS_SERVER & GAA_FLAG_SKIP_FRIENDLY_NAME;

        try
        {
            static Buffer memoryForIPAdapters(16384);
            IP_ADAPTER_ADDRESSES* ipAdapters;
            ULONG errorCode;

            while (true)
            {
                ipAdapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(memoryForIPAdapters.GetData());
                auto memoryForAdaptersSize = (ULONG)memoryForIPAdapters.GetSize();

                errorCode = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, ipAdapters, &memoryForAdaptersSize);
                if (memoryForAdaptersSize > memoryForIPAdapters.GetSize())
                    memoryForIPAdapters.Resize(memoryForAdaptersSize);
                else
                    break;
            }

            if (errorCode == NO_ERROR)
                return { true, ipAdapters };

            if (errorCode == ERROR_NO_DATA || errorCode == ERROR_ADDRESS_NOT_ASSOCIATED)
                return { true, nullptr };

            ErrorHandler::Handle_GetAdaptersAddresses(errorCode);
        }
        catch (...)
        {
            ErrorHandler::SignalError(Error::NotEnoughMemory);
        }

        return { false, nullptr };
    }

    //The returned bool value is set to true if at least one IP address was assigned.
    inline bool _SetNetworkIPAddressesFromIPAdapter(const IP_ADAPTER_ADDRESSES& ipAdapter, NetworkIPAddresses& networkIPAddresses_out) noexcept
    {
        networkIPAddresses_out = {};

        bool isNetworkIPAddressesNotEmpty = false;
        const IP_ADAPTER_UNICAST_ADDRESS* nextUnicastAddress = ipAdapter.FirstUnicastAddress;
        while (nextUnicastAddress != nullptr)
        {
            if (nextUnicastAddress->DadState == IpDadStatePreferred)
            {
                if (nextUnicastAddress->Address.lpSockaddr->sa_family == AF_INET)
                {
                    networkIPAddresses_out.v4NetworkPrefixLength = (uint8_t)nextUnicastAddress->OnLinkPrefixLength;
                    InternalIPv4AddressUtils::CopyFrom(
                        &reinterpret_cast<const sockaddr_in*>(nextUnicastAddress->Address.lpSockaddr)->sin_addr, networkIPAddresses_out.v4);

                }
                else
                {
                    const auto& socketAddress = *reinterpret_cast<const sockaddr_in6*>(nextUnicastAddress->Address.lpSockaddr);

                    networkIPAddresses_out.v6NetworkPrefixLength = (uint8_t)nextUnicastAddress->OnLinkPrefixLength;
                    InternalIPv6AddressUtils::CopyFrom(&socketAddress.sin6_addr, networkIPAddresses_out.v6);
                    networkIPAddresses_out.v6.scopeID = socketAddress.sin6_scope_id;
                    networkIPAddresses_out.v6.flowInfo = socketAddress.sin6_flowinfo;
                }

                isNetworkIPAddressesNotEmpty = true;
            }

            nextUnicastAddress = nextUnicastAddress->Next;
        }

        return isNetworkIPAddressesNotEmpty;
    }

    //This function returns a pointer to the best IPAddress.
    inline const void* _ChooseBestIPAddressInNetworkBO(const IPv4Address& ipv4Address, const IPv6Address& ipv6AddressInNetworkBO) noexcept
    {
        auto ipv4PriorityLevel = (uint8_t)1;
        if (InternalIPv4AddressUtils::IsPrivate(ipv4Address))
        {
            ipv4PriorityLevel = 3;
            return &ipv4Address;
        }
        else if (InternalIPv4AddressUtils::IsLinkLocal(ipv4Address))
        {
            ipv4PriorityLevel = 2;
        }
        else if (InternalIPv4AddressUtils::IsZero(ipv4Address))
        {
            ipv4PriorityLevel = 0;
        }

        auto ipv6PriorityLevel = (uint8_t)1;
        if (InternalIPv6AddressUtils::IsLinkLocalInNetworkBO(ipv6AddressInNetworkBO))
            ipv6PriorityLevel = 2;
        else if (InternalIPv6AddressUtils::IsPrivateInNetworkBO(ipv6AddressInNetworkBO))
            ipv6PriorityLevel = 3;
        else if (InternalIPv6AddressUtils::IsZero(ipv6AddressInNetworkBO))
            ipv6PriorityLevel = 0;

        if (ipv4PriorityLevel >= ipv6PriorityLevel)
            return &ipv4Address;

        return &ipv6AddressInNetworkBO;
    }

    //The returned socket handle can only be nullptr if an error occured.
    //The port number will be updated only if the address isn't zero.
    inline SocketHandle _CreateAndBindIPv4Socket(int type, int protocol, 
        IPv4Address ipv4Address, uint16_t& portNumberInHostBO_inout) noexcept
    {
        sockaddr_in socketAddress;
        socketAddress.sin_family = AF_INET;
        socketAddress.sin_port = HostToNetworkBO(portNumberInHostBO_inout);
        InternalIPv4AddressUtils::CopyTo(&socketAddress.sin_addr, ipv4Address);

        auto* const socketHandle = _CreateAndBindIPSocket(type, protocol, reinterpret_cast<sockaddr&>(socketAddress), 
            (int)sizeof(sockaddr_in), !InternalIPv4AddressUtils::IsZero(ipv4Address));
        if (socketHandle != nullptr)
            portNumberInHostBO_inout = NetworkToHostBO(socketAddress.sin_port);

        return socketHandle;
    }

    //The returned socket handle can only be nullptr if an error occured.
    //The port number will be updated only if the address isn't zero.
    inline SocketHandle _CreateAndBindIPv6Socket(int type, int protocol, 
        const IPv6Address& ipv6AddressInNetworkBO, uint16_t& portNumberInHostBO_inout) noexcept
    {
        sockaddr_in6 socketAddress;
        socketAddress.sin6_family = AF_INET6;
        socketAddress.sin6_port = HostToNetworkBO(portNumberInHostBO_inout);
        socketAddress.sin6_flowinfo = ipv6AddressInNetworkBO.flowInfo;
        InternalIPv6AddressUtils::CopyTo(&socketAddress.sin6_addr, ipv6AddressInNetworkBO);
        socketAddress.sin6_scope_id = ipv6AddressInNetworkBO.scopeID;

        auto* const socketHandle = _CreateAndBindIPSocket(type, protocol, reinterpret_cast<sockaddr&>(socketAddress), 
            (int)sizeof(sockaddr_in6), !InternalIPv6AddressUtils::IsZero(ipv6AddressInNetworkBO));
        if (socketHandle != nullptr)
            portNumberInHostBO_inout = NetworkToHostBO(socketAddress.sin6_port);

        return socketHandle;
    }

    //The returned socket handle can only be nullptr if an error occured.
    //If the passed port number is zero and shouldUpdatePortNumber is true, it will updated the port number.
    //Don't set the shouldUpdatePortNumber parameter to true if the address may be zero.
    inline SocketHandle _CreateAndBindIPSocket(int type, int protocol, 
        sockaddr& socketAddressInNetworkBO_inout, int socketAddressSize, bool shouldUpdatePortNumber) noexcept
    {
        assert(socketAddressInNetworkBO_inout.sa_family == AF_INET || socketAddressInNetworkBO_inout.sa_family == AF_INET6);
        assert(type == SOCK_STREAM && protocol == IPPROTO_TCP ||
            type == SOCK_DGRAM && protocol == IPPROTO_UDP);

        if (auto socketHandle = socket(socketAddressInNetworkBO_inout.sa_family, type, protocol); socketHandle != INVALID_SOCKET)
        {
            auto isNonBlockingModeEnabled = (u_long)1;
            if (ioctlsocket(socketHandle, FIONBIO, &isNonBlockingModeEnabled) == 0)
            {
                if (_BindIPSocket(socketHandle, socketAddressInNetworkBO_inout, socketAddressSize, shouldUpdatePortNumber))
                    return ToSocketHandle(socketHandle);
            }
            else
            {
                ErrorHandler::Handle_ioctlsocket();
            }

            closesocket(socketHandle); //In this context, it doesn't matter if it fails.
            WSASetLastError(0);
            return nullptr;
        }

        ErrorHandler::Handle_socket(socketAddressInNetworkBO_inout.sa_family, type, protocol);
        return nullptr;
    }
    
    //The returned bool value is set to false if the function failed.
    //If the passed port number is zero and shouldUpdatePortNumber is true, it will updated the port number.
    //Don't set the shouldUpdatePortNumber parameter to true if the address may be zero.
    inline bool _BindIPSocket(SOCKET ipSocketToBind, sockaddr& ipSocketAddressInNetworkBO_inout, 
        int ipSocketAddressSize, bool shouldUpdatePortNumber) noexcept
    {
        if (bind(ipSocketToBind, &ipSocketAddressInNetworkBO_inout, ipSocketAddressSize) == 0)
        {
            if (shouldUpdatePortNumber &&
                reinterpret_cast<uint16_t*>(&ipSocketAddressInNetworkBO_inout)[1] == (uint16_t)0) //Checks for the port number being zero.
            {
                if (getsockname(ipSocketToBind, &ipSocketAddressInNetworkBO_inout, &ipSocketAddressSize) != 0)
                {
                    ErrorHandler::Handle_getsockname();
                    return false;
                }
            }

            return true;
        }

        ErrorHandler::Handle_bind();
        return false;
    }
    
    inline SocketHandle _CreateAndConnectIPTCPSocket(uint16_t portNumberToConnectFromInHostBO,
        const sockaddr& socketAddressToConnectToInNetworkBO, int socketAddressToConnectToSize) noexcept
    {
        sockaddr_in6 socketAddress{}; //Used as a buffer for any IP address family.
        socketAddress.sin6_family = socketAddressToConnectToInNetworkBO.sa_family;
        socketAddress.sin6_port = HostToNetworkBO(portNumberToConnectFromInHostBO);

        while (true)
        {
            auto connectingSocketHandle = _CreateAndBindIPSocket(SOCK_STREAM, IPPROTO_TCP,
                reinterpret_cast<sockaddr&>(socketAddress), sizeof(sockaddr_in6));
            if (connectingSocketHandle != nullptr)
            {
                if (connect(ToNativeSocketHandle(connectingSocketHandle),
                    reinterpret_cast<const sockaddr*>(&socketAddressToConnectToInNetworkBO), socketAddressToConnectToSize) != 0)
                {
                    const int errorCode = WSAGetLastError();

                    if (errorCode == WSAEADDRINUSE && portNumberToConnectFromInHostBO == (uint16_t)0)
                    {
                        if (closesocket(ToNativeSocketHandle(connectingSocketHandle)) != 0)
                        {
                            ErrorHandler::Handle_closesocket();
                            return nullptr;
                        }

                        continue;
                    }

                    if (errorCode != WSAEWOULDBLOCK)
                    {
                        ErrorHandler::Handle_connect();
                        closesocket(ToNativeSocketHandle(connectingSocketHandle)); //In this context, it doesn't matter if it fails. 
                        WSASetLastError(0);
                        connectingSocketHandle = nullptr;
                    }
                }
            }

            return connectingSocketHandle;
        }    
    }
}