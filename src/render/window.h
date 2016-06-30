#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class window
{
public:
	window(GLFWwindow* wnd);
	~window() = default;

public:
	glm::uvec2 size() const { return m_size; }
	size_t width()    const { return m_size.x; }
	size_t height()   const { return m_size.y; }

	glm::uvec2 render_size() const { return m_rsize; }
	size_t render_width()    const { return m_rsize.x; }
	size_t render_height()   const { return m_rsize.y; }

	glm::uvec2 render_scale() const { return m_rsize / m_size; }

	GLFWwindow* raw() { return m_handle; }

private:
	GLFWwindow* m_handle;
	glm::uvec2 m_size, m_rsize;
};
