#include "frame_counter.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

frame_counter::frame_counter()
: m_count(), m_dirty(), m_frames()
{
	m_last = glfwGetTime();
}

void frame_counter::update()
{
	double curr = glfwGetTime();

	m_count += 1;
	if (curr - m_last >= 1.0)
	{
		m_dirty  = true;
		m_frames = static_cast<double>(m_count);

		m_last  = curr;
		m_count = 0;
	}
}
