#include "SocketDataSharing.hpp"
#include "WinAPI.hpp"
#include "State.hpp"
#include "ErrorHandler.hpp"
#include "InternalTypeUtils.hpp"
#include "InternalEndiannessConversions.hpp"
#include "Utilities/Buffer.hpp"
#include "PortConstants.hpp"
#include <utility>
#include <vector>
#include <cassert>

namespace SDS
{
    std::pair<WSAPROTOCOL_INFOW*, int> _GetAvailableProtocols() noexcept;
    std::pair<bool, IP_ADAPTER_ADDRESSES*> _GetIPAdapters() noexcept;
    bool _SetNetworkIPAddressesFromIPAdapter(const IP_ADAPTER_ADDRESSES& ipAdapter, NetworkIPAddresses& networkIPAddresses_out) noexcept;
    const void* _ChooseBestIPAddressInNetworkBO(const IPv4Address& ipv4Address, const IPv6Address& ipv6Address) noexcept;
    bool _IsTCPUDPPortInvalid(const uint16_t* tcpudpPortNumberInHostBO) noexcept;
    SocketHandle _CreateAndBindIPv4Socket(int type, int protocol, IPv4Address ipv4Address, uint16_t& portNumberInHostBO_inout) noexcept;
    SocketHandle _CreateAndBindIPv6Socket(int type, int protocol, 
        const IPv6Address& ipv4AddressInNetworkBO, uint16_t& portNumberInHostBO_inout) noexcept;
    SocketHandle _CreateAndBindIPSocket(int type, int protocol, sockaddr& socketNameInNetworkBO_inout, int socketNameSizeInBytes) noexcept;
    bool _BindSocket(SOCKET socketToBind, sockaddr& socketNameInNetworkBO_inout, int socketNameSizeInBytes) noexcept;

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

    SocketHandle CreateIPv4TCPSocket(IPv4Address ipv4Address, uint16_t* portNumberInHostBO_inout) noexcept
    {
        if (InternalIPv4AddressUtils::IsZero(ipv4Address))
        {
            ErrorHandler::SignalError(Error::UnavailableIPAddress);
            return nullptr;
        }

        if (_IsTCPUDPPortInvalid(portNumberInHostBO_inout))
            return nullptr;

        return _CreateAndBindIPv4Socket(SOCK_STREAM, IPPROTO_TCP, ipv4Address, *portNumberInHostBO_inout);
    }

    SocketHandle CreateIPv4UDPSocket(IPv4Address ipv4Address, uint16_t* portNumberInHostBO_inout) noexcept
    {
        if (InternalIPv4AddressUtils::IsZero(ipv4Address))
        {
            ErrorHandler::SignalError(Error::UnavailableIPAddress);
            return nullptr;
        }

        if (_IsTCPUDPPortInvalid(portNumberInHostBO_inout))
            return nullptr;

        return _CreateAndBindIPv4Socket(SOCK_DGRAM, IPPROTO_UDP, ipv4Address, *portNumberInHostBO_inout);
    }

    SocketHandle CreateIPv6TCPSocket(IPv6Address ipv6AddressInNetworkBO, uint16_t* portNumberInHostBO_inout) noexcept
    {
        if (InternalIPv6AddressUtils::IsZero(ipv6AddressInNetworkBO))
        {
            ErrorHandler::SignalError(Error::UnavailableIPAddress);
            return nullptr;
        }

        if (_IsTCPUDPPortInvalid(portNumberInHostBO_inout))
            return nullptr;

        return _CreateAndBindIPv6Socket(SOCK_STREAM, IPPROTO_TCP, ipv6AddressInNetworkBO, *portNumberInHostBO_inout);
    }

    SocketHandle CreateIPv6UDPSocket(IPv6Address ipv6AddressInNetworkBO, uint16_t* portNumberInHostBO_inout) noexcept
    {
        if (InternalIPv6AddressUtils::IsZero(ipv6AddressInNetworkBO))
        {
            ErrorHandler::SignalError(Error::UnavailableIPAddress);
            return nullptr;
        }

        if (_IsTCPUDPPortInvalid(portNumberInHostBO_inout))
            return nullptr;

        return _CreateAndBindIPv6Socket(SOCK_DGRAM, IPPROTO_UDP, ipv6AddressInNetworkBO, *portNumberInHostBO_inout);
    }

    ErrorIndicator DestroySocket(SocketHandle socketHandle) noexcept
    {
        if (closesocket(reinterpret_cast<SOCKET>(socketHandle)) != 0)
        {
            ErrorHandler::Handle_closesocket();
            return ErrorIndicator::Error;
        }

        return (ErrorIndicator)1;
    }

    ErrorIndicator SetIPTCPSocketDestructionTimeout(SocketHandle socketHandle, Bool isEnabled, uint16_t timeInSeconds) noexcept
    {
        linger optionValue{ (u_short)isEnabled, (u_short)timeInSeconds };
        if (setsockopt(reinterpret_cast<SOCKET>(socketHandle), SOL_SOCKET, SO_LINGER,
            reinterpret_cast<char*>(&optionValue), sizeof(linger)) != 0)
        {
            ErrorHandler::Handle_setsockopt();
            return ErrorIndicator::Error;
        }

        return (ErrorIndicator)1;
    }

    ErrorIndicator SetIPUDPSocketBroadcast(SocketHandle socketHandle, Bool isEnabled) noexcept
    {
        auto optionValue = (BOOL)isEnabled;
        if (setsockopt(reinterpret_cast<SOCKET>(socketHandle), SOL_SOCKET, SO_BROADCAST, 
                reinterpret_cast<char*>(&optionValue), sizeof(BOOL)) != 0)
        {
            ErrorHandler::Handle_setsockopt();
            return ErrorIndicator::Error;
        }

        return (ErrorIndicator)1;
    }

    //The pointer is null only if an error occured.
    //The int value is used to store the protocol info array's size.
    std::pair<WSAPROTOCOL_INFOW*, int> _GetAvailableProtocols() noexcept
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

    //The bool value is set to false if the function failed.
    //The pointer can be null if no data was found.
    std::pair<bool, IP_ADAPTER_ADDRESSES*> _GetIPAdapters() noexcept
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

    //The bool value is set to true if at least one IP address was assigned.
    bool _SetNetworkIPAddressesFromIPAdapter(const IP_ADAPTER_ADDRESSES& ipAdapter, NetworkIPAddresses& networkIPAddresses_out) noexcept
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
                    const auto& socketName = *reinterpret_cast<const sockaddr_in6*>(nextUnicastAddress->Address.lpSockaddr);

                    networkIPAddresses_out.v6NetworkPrefixLength = (uint8_t)nextUnicastAddress->OnLinkPrefixLength;
                    InternalIPv6AddressUtils::CopyFrom(&socketName.sin6_addr, networkIPAddresses_out.v6);
                    networkIPAddresses_out.v6.scopeID = socketName.sin6_scope_id;
                    networkIPAddresses_out.v6.flowInfo = socketName.sin6_flowinfo;
                }

                isNetworkIPAddressesNotEmpty = true;
            }

            nextUnicastAddress = nextUnicastAddress->Next;
        }

        return isNetworkIPAddressesNotEmpty;
    }

    //This function returns a poiter to the best IPAddress.
    const void* _ChooseBestIPAddressInNetworkBO(const IPv4Address& ipv4Address, const IPv6Address& ipv6AddressInNetworkBO) noexcept
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

    bool _IsTCPUDPPortInvalid(const uint16_t* tcpudpPortNumberInHostBO) noexcept
    {
        if (tcpudpPortNumberInHostBO == nullptr)
        {
            ErrorHandler::SignalError(Error::PassedPointerIsNull);
            return true;
        }

        if (*tcpudpPortNumberInHostBO != (uint16_t)0 &&
            Port::userRange.IsOutsideRange(*tcpudpPortNumberInHostBO))
        {
            ErrorHandler::SignalError(Error::TCPUDPPortNumberIsInvalid);
            return true;
        }

        return false;
    }

    //The returned socket handle can only be nullptr if an error occured.
    //If the passed port number is 0, it will be updated.
    SocketHandle _CreateAndBindIPv4Socket(int type, int protocol, IPv4Address ipv4Address, uint16_t& portNumberInHostBO_inout) noexcept
    {
        sockaddr_in socketName;
        socketName.sin_family = AF_INET;
        socketName.sin_port = HostToNetworkBO(portNumberInHostBO_inout);
        InternalIPv4AddressUtils::CopyFrom(ipv4Address.octets, reinterpret_cast<IPv4Address&>(socketName.sin_addr));

        auto* const socketHandle = _CreateAndBindIPSocket(type, protocol, reinterpret_cast<sockaddr&>(socketName), (int)sizeof(sockaddr_in));
        if (socketHandle != nullptr)
            portNumberInHostBO_inout = NetworkToHostBO(socketName.sin_port);

        return socketHandle;
    }

    //The returned socket handle can only be nullptr if an error occured.
    //If the passed port number is 0, it will be updated.
    SocketHandle _CreateAndBindIPv6Socket(int type, int protocol, 
        const IPv6Address& ipv4AddressInNetworkBO, uint16_t& portNumberInHostBO_inout) noexcept
    {
        sockaddr_in6 socketName;
        socketName.sin6_family = AF_INET6;
        socketName.sin6_port = HostToNetworkBO(portNumberInHostBO_inout);
        socketName.sin6_flowinfo = ipv4AddressInNetworkBO.flowInfo;
        InternalIPv6AddressUtils::CopyFrom(ipv4AddressInNetworkBO.hextets, reinterpret_cast<IPv6Address&>(socketName.sin6_addr));
        socketName.sin6_scope_id = ipv4AddressInNetworkBO.scopeID;

        auto* const socketHandle = _CreateAndBindIPSocket(type, protocol, reinterpret_cast<sockaddr&>(socketName), (int)sizeof(sockaddr_in6));
        if (socketHandle != nullptr)
            portNumberInHostBO_inout = NetworkToHostBO(socketName.sin6_port);

        return socketHandle;
    }

    //The returned socket handle can only be nullptr if an error occured.
    //If the passed port number is 0, it will be updated.
    SocketHandle _CreateAndBindIPSocket(int type, int protocol, sockaddr& socketNameInNetworkBO_inout, int socketNameSizeInBytes) noexcept
    {
        assert(socketNameInNetworkBO_inout.sa_family != AF_UNSPEC);
        assert(type == SOCK_STREAM && protocol == IPPROTO_TCP ||
            type == SOCK_DGRAM && protocol == IPPROTO_UDP);

        if (const auto socketHandle = socket(socketNameInNetworkBO_inout.sa_family, type, protocol); socketHandle != INVALID_SOCKET)
        {
            if (_BindSocket(socketHandle, socketNameInNetworkBO_inout, socketNameSizeInBytes))
                return reinterpret_cast<SocketHandle>(socketHandle);

            //In this context, it doesn't matter if it fails.
            closesocket(socketHandle);
            return nullptr;
        }

        ErrorHandler::Handle_socket(socketNameInNetworkBO_inout.sa_family, type, protocol);
        return nullptr;
    }
    
    //The bool value is set to false if the function failed.
    //If the passed port number is 0, it will be updated.
    bool _BindSocket(SOCKET socketToBind, sockaddr& socketNameInNetworkBO_inout, int socketNameSizeInBytes) noexcept
    {
        if (bind(socketToBind, &socketNameInNetworkBO_inout, socketNameSizeInBytes) == 0)
        {
            if (reinterpret_cast<uint16_t*>(&socketNameInNetworkBO_inout)[1] == (uint16_t)0) //Checks for the port number being zero.
            {
                if (getsockname(socketToBind, &socketNameInNetworkBO_inout, &socketNameSizeInBytes) != 0)
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
}