#pragma once

#include <vector>
#include <type_traits>
#include <cassert>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../common/glref.h"

class buffer
{
public:
	template<typename T>
	buffer(GLenum mode, const std::vector<T>& buf);
	~buffer() = default;

public:
	void bind(bool on = true) const { glBindBuffer(m_mode, on?m_bo:0); }

	template<typename T>
	void update(const std::vector<T>& buf);

	GLenum mode() const { return m_mode; }
	GLenum type() const { return m_type; }

	size_t size() const { return m_elements; }
	size_t element_size()   const { return m_esize; }
	size_t element_stride() const { return m_stride; }

	GLuint raw() const { return m_bo; }

private:
	template<typename T, std::enable_if_t<std::is_class<T>::value, int> = 0>
	size_t component_size() const { return sizeof(typename T::value_type); }
	template<typename T, std::enable_if_t<!std::is_class<T>::value, int> = 0>
	size_t component_size() const { return sizeof(T); }

	template<typename T>
	GLenum element_type() const;

private:
	GLenum m_mode;
	glref  m_bo;

	GLenum m_type;
	size_t m_elements;
	size_t m_stride, m_esize;
};

template<typename T>
buffer::buffer(GLenum mode, const std::vector<T>& buf)
: m_mode(mode), m_stride(sizeof(T))
{
	m_bo = glref(glGenBuffers, glDeleteBuffers);

	m_type  = this->element_type<T>();
	m_esize = m_stride / this->component_size<T>();

	this->update(buf);
}

template<typename T>
void buffer::update(const std::vector<T>& buf)
{
	assert(sizeof(T) == m_stride);
	assert(this->element_type<T>() == m_type);

	m_elements = buf.size();

	glBindBuffer(m_mode, m_bo);
	glBufferData(m_mode, m_elements * m_stride, buf.data(), GL_STATIC_DRAW);
	glBindBuffer(m_mode, 0);
}

template<typename T>
GLenum buffer::element_type() const
{
	size_t size = this->component_size<T>();
	if (std::is_integral<T>::value)
	{
		if (std::is_signed<T>::value)
		{
			switch (size)
			{
				case 1: return GL_BYTE;
				case 2: return GL_SHORT;
				case 4: return GL_INT;
			}
		}
		else
		{
			switch (size)
			{
				case 1: return GL_UNSIGNED_BYTE;
				case 2: return GL_UNSIGNED_SHORT;
				case 4: return GL_UNSIGNED_INT;
			}
		}
	}
	else
	{
		switch (size)
		{
			case 4: return GL_FLOAT;
			case 8: return GL_DOUBLE;
		}
	}

	return GL_NONE;
}
