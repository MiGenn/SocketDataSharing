#pragma once
#include <cstddef>

//Dynamic array of bytes.
class Buffer final
{
public:
	Buffer() noexcept = default;

	//Passing zero will make the constructor behave like the default constructor.
	Buffer(size_t size);
	Buffer(const Buffer&) = delete;
	Buffer(Buffer&& anotherBuffer) noexcept;
	~Buffer() noexcept { Destruct(); }

	//Passing zero will deallocate the memory.
	void Resize(size_t newSize);

	size_t GetSize() const noexcept { return m_bufferSize; }

	void* GetData() noexcept { return m_buffer; }
	void* GetData() const noexcept { return m_buffer; }

	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&& anotherBuffer) noexcept;

private:
	size_t m_bufferSize = (size_t)0;
	std::byte* m_buffer = nullptr;

	void Destruct() noexcept;
};