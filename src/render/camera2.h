#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "camera.h"
#include "window.h"

class camera2 : public camera
{
public:
	camera2(const window& wnd, float scale = 1.f, float far = 10.f);
	~camera2() = default;

public:
	void move_to(const glm::vec2& pos) { camera::move_to(glm::vec3(pos, 0)); }

	void set_scale(float sc) { m_scale = sc; updateProj(); }

private:
	void updateProj() override;
	void updateView() override;

private:
	float m_scale;
};
