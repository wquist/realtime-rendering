#include "camera3.h"
#include <glm/gtc/matrix_transform.hpp>

camera3::camera3(const window& wnd, float fov, float far)
: camera(far), m_fov(fov), m_lookat()
{
	this->resize(wnd);
}

void camera3::updateProj()
{
	auto fov = glm::radians(m_fov);
	m_projection = glm::perspective(fov, m_aspect, m_near, m_far);
}

void camera3::updateView()
{
	glm::vec3 up(0,1,0);
	m_view = glm::lookAt(m_position, m_lookat, up);
}
