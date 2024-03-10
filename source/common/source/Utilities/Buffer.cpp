#include "Utilities/Buffer.hpp"
#include <memory>

Buffer::Buffer(size_t size) :
	m_bufferSize(size)
{
	if (m_bufferSize != (size_t)0)
		m_buffer = new std::byte[m_bufferSize];
}

Buffer::Buffer(Buffer&& anotherBuffer) noexcept
{
	(*this) = std::move(anotherBuffer);
}

void Buffer::Resize(size_t newSize)
{
	Destruct();

	m_bufferSize = newSize;
	if (m_bufferSize != (size_t)0)
		m_buffer = new std::byte[m_bufferSize];
}

Buffer& Buffer::operator=(Buffer&& anotherBuffer) noexcept
{
	if (this != &anotherBuffer)
	{
		Destruct();

		m_bufferSize = anotherBuffer.m_bufferSize;
		m_buffer = anotherBuffer.m_buffer;

		anotherBuffer.m_bufferSize = (size_t)0;
		anotherBuffer.m_buffer = nullptr;
	}

	return *this;
}

void Buffer::Destruct() noexcept
{
	delete[] m_buffer;
	m_buffer = nullptr;
}