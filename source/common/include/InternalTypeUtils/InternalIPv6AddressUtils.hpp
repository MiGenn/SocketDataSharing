#pragma once
#include "IndirectIncludes/Types.hpp"
#include "InternalEndiannessConversions.hpp"
#include <cassert>

namespace InternalIPv6AddressUtils
{
	//Raw address must be at least 16 bytes long.
	//It is only used to copy a IPv6 address itself, not the additional information such as scope ID or flow information.
	inline void CopyFrom(const void* rawAddress, SDS::IPv6Address& address_out) noexcept
	{
		assert(rawAddress != nullptr);

		const auto* const addressToCopyFrom = reinterpret_cast<const uint64_t*>(rawAddress);
		auto* const addressToCopyTo = reinterpret_cast<uint64_t*>(address_out.hextets);

		addressToCopyTo[0] = addressToCopyFrom[0];
		addressToCopyTo[1] = addressToCopyFrom[1];
	}

	inline void CopyFrom(const SDS::IPv6Address& address, SDS::IPv6Address& address_out) noexcept
	{
		CopyFrom(reinterpret_cast<const void*>(address.hextets), address_out);
		reinterpret_cast<uint64_t&>(address_out.scopeID) = reinterpret_cast<const uint64_t&>(address.scopeID);
	}

	inline void ToHostBO(const SDS::IPv6Address& addressInNetworkBO, SDS::IPv6Address& addressInHostBO_out) noexcept
	{
		for (auto i = (size_t)0; i < (size_t)8; ++i)
			addressInHostBO_out.hextets[i] = HostToNetworkBO(addressInNetworkBO.hextets[i]);

		reinterpret_cast<uint64_t&>(addressInHostBO_out.scopeID) = reinterpret_cast<const uint64_t&>(addressInNetworkBO.scopeID);
	}

	inline void ToNetworkBO(const SDS::IPv6Address& addressInHostBO, SDS::IPv6Address& addressInNetworkBO_out) noexcept
	{
		for (auto i = (size_t)0; i < (size_t)8; ++i)
			addressInNetworkBO_out.hextets[i] = NetworkToHostBO(addressInHostBO.hextets[i]);

		reinterpret_cast<uint64_t&>(addressInNetworkBO_out.scopeID) = reinterpret_cast<const uint64_t&>(addressInHostBO.scopeID);
	}

	inline bool IsZero(const SDS::IPv6Address& address) noexcept
	{
		return reinterpret_cast<const uint64_t&>(address.hextets[0]) == (uint64_t)0 &&
			reinterpret_cast<const uint64_t&>(address.hextets[4]) == (uint64_t)0;
	}

	inline bool IsLoopback(const SDS::IPv6Address& address) noexcept
	{
		static constexpr uint16_t loopbackSecondPart[4]{ 0, 0, 0, 1 };
		return reinterpret_cast<const uint64_t&>(address.hextets[0]) == (uint64_t)0 &&
			reinterpret_cast<const uint64_t&>(address.hextets[4]) == reinterpret_cast<const uint64_t&>(*loopbackSecondPart);
	}

	inline bool IsLoopbackInNetworkBO(const SDS::IPv6Address& addressInNetworkBO) noexcept
	{
		static constexpr uint8_t loopbackSecondPart[8]{ 0, 0, 0, 0, 0, 0, 0, 1 };
		return reinterpret_cast<const uint64_t&>(addressInNetworkBO.hextets[0]) == (uint64_t)0 &&
			reinterpret_cast<const uint64_t&>(addressInNetworkBO.hextets[4]) == reinterpret_cast<const uint64_t&>(*loopbackSecondPart);
	}

	inline bool IsLinkLocal(const SDS::IPv6Address& address) noexcept
	{
		return (address.hextets[0] & (uint16_t)0xFFC0) == (uint16_t)0xFE80;
	}

	inline bool IsLinkLocalInNetworkBO(const SDS::IPv6Address& address) noexcept
	{
		static constexpr uint8_t linkLocalPrefix[2]{ 0xFE, 0x80 };
		static constexpr uint8_t linkLocalPrefixMask[2]{ 0xFF, 0xC0 };

		return (address.hextets[0] & reinterpret_cast<const uint16_t&>(*linkLocalPrefixMask)) == 
			reinterpret_cast<const uint16_t&>(*linkLocalPrefix);
	}

	inline bool IsPrivate(const SDS::IPv6Address& address) noexcept
	{
		return (address.hextets[0] & (uint16_t)0xFF00) == (uint16_t)0xFD00;
	}

	inline bool IsPrivateInNetworkBO(const SDS::IPv6Address& address) noexcept
	{
		static constexpr uint8_t privatePrefix[2]{ 0xFD, 0x00 };
		static constexpr uint8_t privatePrefixMask[2]{ 0xFF, 0x00 };

		return (address.hextets[0] & reinterpret_cast<const uint16_t&>(*privatePrefixMask)) ==
			reinterpret_cast<const uint16_t&>(*privatePrefix);
	}
}