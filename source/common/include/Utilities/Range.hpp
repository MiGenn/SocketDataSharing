#pragma once
#include <type_traits>
#include <stdexcept>

template<typename Number>
class Range final
{
public:
	static_assert(std::is_arithmetic_v<Number> && !std::is_same_v<bool, Number> &&
		!std::is_same_v<wchar_t, Number> && !std::is_same_v<char16_t, Number> && !std::is_same_v<char32_t, Number>);

	enum class Type : uint8_t
	{
		Inclusive = 0,
		Exclusive = 1,
		InclusiveLeftExclusiveRight = 2,
		ExclusiveLeftInclusiveRight = 3
	};

	Type type;

	constexpr Range() noexcept;

	//rangeStart must be less than or equal to rangeEnd.
	constexpr Range(Number rangeStart, Number rangeEnd, Type type = Type::Inclusive);

	//newRangeStart must be less than or equal to rangeEnd.
	constexpr void SetRangeStart(Number newRangeStart);

	//newRangeEndt must be greater than or equal to rangeStart.
	constexpr void SetRangeEnd(Number newRangeEnd);

	constexpr Number GetRangeStart() const noexcept { return m_rangeStart; }
	constexpr Number GetRangeEnd() const noexcept { return m_rangeEnd; }

	constexpr bool IsWithinRange(Number number) const noexcept;
	constexpr bool IsOutsideRange(Number number) const noexcept { return !IsWithinRange(number); }

private:
	Number m_rangeStart;
	Number m_rangeEnd;
};

template<typename Number>
inline constexpr Range<Number>::Range() noexcept :
	m_rangeStart((Number)0), m_rangeEnd((Number)0), type(Type::Inclusive)
{

}

template<typename Number>
inline constexpr Range<Number>::Range(Number rangeStart, Number rangeEnd, Type type) :
	m_rangeStart(rangeStart), m_rangeEnd(rangeEnd), type(type)
{
	if (rangeStart > rangeEnd)
		throw std::logic_error("Invalid range!");
}

template<typename Number>
inline constexpr void Range<Number>::SetRangeStart(Number newRangeStart)
{
	if (newRangeStart > m_rangeEnd)
		throw std::logic_error("Invalid new range start!");

	m_rangeStart = newRangeStart;
}

template<typename Number>
inline constexpr void Range<Number>::SetRangeEnd(Number newRangeEnd)
{
	if (newRangeEnd < m_rangeStart)
		throw std::logic_error("Invalid new range end!");

	m_rangeEnd = newRangeEnd;
}

template<typename Number>
inline constexpr bool Range<Number>::IsWithinRange(Number number) const noexcept
{
	if (type == Type::Inclusive)
		return m_rangeStart <= number && number <= m_rangeEnd;

	if (type == Type::InclusiveLeftExclusiveRight)
		return m_rangeStart <= number && number < m_rangeEnd;

	if (type == Type::Exclusive)
		return m_rangeStart < number && number < m_rangeEnd;

	return m_rangeStart < number && number <= m_rangeEnd;
}