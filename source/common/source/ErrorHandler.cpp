#include "ErrorHandler.hpp"
#include <stdexcept>
#include <cassert>

using namespace SDS;

void DoNothingWhenErrorOccured(Error, intmax_t, void*) {};
ErrorOccuredCallback ErrorHandler::m_callback = DoNothingWhenErrorOccured;

void ErrorHandler::SignalError(Error error) noexcept
{
    assert(error != Error::Success && error != Error::UnexpectedSystemError);

    m_callback(error, (intmax_t)0, m_callbackContext);
}

void ErrorHandler::SetCallback(ErrorOccuredCallback callback, void* callbackContext)
{
    if (callback != nullptr)
    {
        try
        {
            callback(Error::Success, (intmax_t)0, callbackContext);

            m_callback = callback;
            m_callbackContext = callbackContext;
        }
        catch (...)
        {

        }
    }

    throw std::logic_error("Invalid error occured callback!");
}