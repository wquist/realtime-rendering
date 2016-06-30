#pragma once

#include <limits>

#include <glm/glm.hpp>

template<typename T>
class aabb
{
public:
	aabb() { this->clear(); }
	aabb(const T& min, const T& max) : m_min(min), m_max(max) {}
	~aabb() = default;

public:
	void expand(const T& pt)
	{
		m_min = glm::min(m_min, pt);
		m_max = glm::max(m_max, pt);
	}

	void expand(const aabb<T>& other)
	{
		m_min = glm::min(m_min, other.m_min);
		m_max = glm::max(m_max, other.m_max);
	}

	void clear()
	{
		m_min = T(std::numeric_limits<typename T::value_type>::max());
		m_max = T(std::numeric_limits<typename T::value_type>::min());
	}

	const T& min() const { return m_min; }
	const T& max() const { return m_max; }

private:
	T m_min, m_max;
};
