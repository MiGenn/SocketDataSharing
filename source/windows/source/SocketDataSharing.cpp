#include "SocketDataSharing.hpp"
#include "WinAPI.hpp"
#include "State.hpp"
#include "ErrorHandler.hpp"
#include "InternalTypeUtils.hpp"
#include "Utilities/Buffer.hpp"
#include <utility>
#include <vector>
#include <cassert>

namespace SDS
{
    std::pair<bool, IP_ADAPTER_ADDRESSES*> _GetIPAdapters() noexcept;
    bool _SetNetworkIPAddressesFromIPAdapter(const IP_ADAPTER_ADDRESSES& ipAdapter, NetworkIPAddresses& networkIPAddresses_out) noexcept;
    const void* _ChooseBestIPAddressInNetworkBO(const IPv4Address& ipv4Address, const IPv6Address& ipv6Address) noexcept;

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

    NetworkIPAddresses* GetNetworkIPAddressesArray(int32_t* size_out) noexcept
    {
        if (!State::isInitialized)
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
        if (!State::isInitialized)
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

    //The bool value is set to false if the function failed.
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
        IP_ADAPTER_UNICAST_ADDRESS* nextUnicastAddress = ipAdapter.FirstUnicastAddress;
        while (nextUnicastAddress != nullptr)
        {
            if (nextUnicastAddress->DadState == IpDadStatePreferred)
            {
                if (nextUnicastAddress->Address.lpSockaddr->sa_family == AF_INET)
                {
                    networkIPAddresses_out.v4NetworkPrefixLength = (uint8_t)nextUnicastAddress->OnLinkPrefixLength;
                    InternalIPv4AddressUtils::CopyFrom(
                        &reinterpret_cast<sockaddr_in*>(nextUnicastAddress->Address.lpSockaddr)->sin_addr, 
                        networkIPAddresses_out.v4);
                }
                else
                {
                    networkIPAddresses_out.v6NetworkPrefixLength = (uint8_t)nextUnicastAddress->OnLinkPrefixLength;
                    InternalIPv6AddressUtils::CopyFrom(
                        &reinterpret_cast<sockaddr_in6*>(nextUnicastAddress->Address.lpSockaddr)->sin6_addr,
                        networkIPAddresses_out.v6);
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
}