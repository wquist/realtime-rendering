#include "window.h"

window::window(GLFWwindow* wnd)
: m_handle(wnd)
{
	int w, h;

	glfwGetWindowSize(wnd, &w, &h);
	m_size.x = static_cast<size_t>(w);
	m_size.y = static_cast<size_t>(h);

	glfwGetFramebufferSize(wnd, &w, &h);
	m_rsize.x = static_cast<size_t>(w);
	m_rsize.y = static_cast<size_t>(h);
}
