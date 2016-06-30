#pragma once

#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../common/exception.h"

class image
{
public:
	class exception : public ::exception
	{
	public:
		using ::exception::exception;
		const char* source() const throw() { return "image"; }
	};

public:
	image()  = default;
	virtual ~image() = default;

public:
	virtual void load(const std::string& file);

	const std::vector<GLubyte>& pixels() const { return m_pixels; }
	GLenum format() const { return m_format; }
	size_t bpp()    const { return m_bpp; }

	size_t width()    const { return m_width; }
	size_t height()   const { return m_height; }
	glm::uvec2 size() const { return glm::uvec2(m_width, m_height); }

protected:
	std::vector<GLubyte> m_pixels;
	GLenum m_format;
	size_t m_bpp, m_width, m_height;
};
