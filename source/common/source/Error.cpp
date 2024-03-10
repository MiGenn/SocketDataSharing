#include "Error.hpp"
#include "ErrorHandler.hpp"

void SDS::SetErrorOccuredCallback(ErrorOccuredCallback callback, void* callbackContext)
{
	ErrorHandler::SetCallback(callback, callbackContext);
}