#include "camera.h"

camera::camera(const window& wnd, float far)
: camera(far)
{
	this->resize(wnd);
	this->updateView();
}

void camera::resize(const window& wnd)
{
	auto sz = glm::vec2(wnd.size());

	m_aspect = sz.x / sz.y;
	this->updateProj();
}