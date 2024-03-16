#pragma once
#include <cstdint>

#include "IndirectIncludes/SocketDataSharingAPIDefine.hpp"

namespace SDS
{
	extern "C"
	{
		//BO means byte order.
		//You actually can use any type you want. Only the bit width must match.

		SOCKETDATASHARING_API uint64_t HostToNetworkBO_64(uint64_t value) noexcept;
		SOCKETDATASHARING_API uint32_t HostToNetworkBO_32(uint32_t value) noexcept;
		SOCKETDATASHARING_API uint16_t HostToNetworkBO_16(uint16_t value) noexcept;

		SOCKETDATASHARING_API uint64_t NetworkToHostBO_64(uint64_t value) noexcept;
		SOCKETDATASHARING_API uint32_t NetworkToHostBO_32(uint32_t value) noexcept;
		SOCKETDATASHARING_API uint16_t NetworkToHostBO_16(uint16_t value) noexcept;
	}
}