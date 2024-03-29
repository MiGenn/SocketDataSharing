#pragma once
#include "IndirectIncludes/Types.hpp"
#include <cassert>

namespace InternalIPv4AddressUtils
{
	//The raw address must be at least 4 bytes long.
	inline void CopyFrom(const void* rawAddress, SDS::IPv4Address& address_out) noexcept
	{
		assert(rawAddress != nullptr);
		reinterpret_cast<uint32_t&>(*address_out.octets) = *reinterpret_cast<const uint32_t*>(rawAddress);
	}

	//The raw address must be at least 4 bytes long.
	inline void CopyTo(void* rawAddress_out, const SDS::IPv4Address& address) noexcept
	{
		assert(rawAddress_out != nullptr);
		*reinterpret_cast<uint32_t*>(rawAddress_out) = reinterpret_cast<const uint32_t&>(*address.octets);
	}

	inline bool IsZero(SDS::IPv4Address address) noexcept
	{
		return reinterpret_cast<const uint32_t&>(*address.octets) == (uint32_t)0;
	}

	inline bool IsLoopback(SDS::IPv4Address address) noexcept
	{
		return address.octets[0] == (uint8_t)127;
	}

	inline bool IsLinkLocal(SDS::IPv4Address address) noexcept
	{
		static constexpr uint8_t linkLocalPrefix[]{ 169, 254 };
		return reinterpret_cast<const uint16_t&>(*address.octets) == reinterpret_cast<const uint16_t&>(*linkLocalPrefix);
	}

	inline bool IsPrivate(SDS::IPv4Address address) noexcept
	{
		static constexpr uint8_t privatePrefix1[]{ 192, 168 };
		static constexpr uint8_t privatePrefix2[]{ 172, 16 };
		static constexpr uint8_t privatePrefix2Mask[]{ 0xFF, 0xF0 };

		return reinterpret_cast<const uint16_t&>(*address.octets) == reinterpret_cast<const uint16_t&>(*privatePrefix1) ||
			(reinterpret_cast<const uint16_t&>(*address.octets) & reinterpret_cast<const uint16_t&>(*privatePrefix2Mask)) == 
			reinterpret_cast<const uint16_t&>(*privatePrefix2) ||
			address.octets[0] == (uint8_t)10;
	}
}