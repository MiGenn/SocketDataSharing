#include "EndiannessConversions.hpp"
#include "InternalEndiannessConversions.hpp"

uint64_t SDS::HostToNetworkBO_64(uint64_t value) noexcept
{
    return HostToNetworkBO(value);
}

uint32_t SDS::HostToNetworkBO_32(uint32_t value) noexcept
{
    return HostToNetworkBO(value);
}

uint16_t SDS::HostToNetworkBO_16(uint16_t value) noexcept
{
    return HostToNetworkBO(value);
}

uint64_t SDS::NetworkToHostBO_64(uint64_t value) noexcept
{
    return NetworkToHostBO(value);
}

uint32_t SDS::NetworkToHostBO_32(uint32_t value) noexcept
{
    return NetworkToHostBO(value);
}

uint16_t SDS::NetworkToHostBO_16(uint16_t value) noexcept
{
    return NetworkToHostBO(value);
}