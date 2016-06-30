#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "camera.h"
#include "window.h"

class camera3 : public camera
{
public:
	camera3(const window& wnd, float fov = 60.f, float far = 10.f);
	~camera3() = default;

public:
	void look_at(const glm::vec3& pt) { m_lookat = pt; updateView(); }

	float fov() const { return m_fov; }
	void set_fov(float f) { m_fov = f; updateProj(); }

	glm::vec3 looking() const { return m_lookat; }

private:
	void updateProj() override;
	void updateView() override;

private:
	float m_fov;
	glm::vec3 m_lookat;
};
