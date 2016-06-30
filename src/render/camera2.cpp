#include "camera2.h"
#include <glm/gtc/matrix_transform.hpp>

camera2::camera2(const window& wnd, float scale, float far)
: camera(far), m_scale(scale)
{
	m_near = 0.f;
	this->resize(wnd);
}

void camera2::updateProj()
{
	auto x = m_scale * m_aspect;
	auto y = m_scale;

	m_projection = glm::ortho(-x, x, -y, y, m_near, m_far);
}

void camera2::updateView()
{
	m_view = glm::translate(glm::mat4(1.f), m_position * -1.f);
}
