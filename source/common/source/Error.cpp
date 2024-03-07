#include "Interface/Error.hpp"
#include "ErrorHandler.hpp"
#include <stdexcept>

void SDS::SetErrorOccuredCallback(ErrorOccuredCallback callback, void* callbackContext)
{
	ErrorHandler::SetCallback(callback, callbackContext);
}