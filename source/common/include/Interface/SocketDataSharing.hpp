#pragma once
#include <cstdint>
#include "IndirectIncludes/TypeUtils.hpp"

#include "IndirectIncludes/SocketDataSharingAPIDefine.hpp"

//The library is not thread safe!
//In order to use static version of the library you need to define SOCKETDATASHARING_STATIC.

namespace SDS
{
	extern "C"
	{
		using SocketHandle = void*;

		//The second function to be called if you have called SetErrorOccuredCallback.
		//Calling this function is required if you want to use any function from this header.
		SOCKETDATASHARING_API ErrorIndicator Initialize() noexcept;

		//The last function to be called. Must be called only after the Initialize function has been called.
		//It automatically destroys all created sockets.
		SOCKETDATASHARING_API ErrorIndicator Shutdown() noexcept;

		//Host can be connected to multiple networks and have more than one IP address per network.
		//If the host isn't connected to any network the returned IP address count is 0 but the data pointer isn't null.
		//The data pointer is null only if an error occured. Also you don't need to deallocate the memory.
		//The returned IPv6 addresses are in network byte order.
		SOCKETDATASHARING_API NetworkIPAddresses* GetNetworkIPAddressesArray(int32_t* size_out) noexcept;

		//This function accepts any addresses. BO means byte order.
		SOCKETDATASHARING_API ErrorBool IsIPv4AddressPreferred(const NetworkIPAddresses* networkIPAddressesInNetworkBO) noexcept;
	}
}