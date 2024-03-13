#include "ErrorHandler.hpp"
#include <cstddef>
#include <stdexcept>
#include <cassert>

using namespace SDS;

void DoNothingWhenErrorOccured(Error, int64_t, void*) {};
ErrorOccuredCallback ErrorHandler::m_callback = DoNothingWhenErrorOccured;

void ErrorHandler::SignalError(Error error) noexcept
{
    assert(error != Error::Success && error != Error::UnexpectedSystemError);
    m_callback(error, (int64_t)0, m_callbackContext);
}

void ErrorHandler::SetCallback(ErrorOccuredCallback callback, void* callbackContext)
{
    if (callback != nullptr)
    {
        try
        {
            callback(Error::Success, (int64_t)0, callbackContext);

            m_callback = callback;
            m_callbackContext = callbackContext;

            return;
        }
        catch (...)
        {

        }
    }

    throw std::logic_error("Invalid ErrorOccuredCallback!");
}