#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "window.h"

class camera
{
public:
	camera(float far = 10.f)
	: m_near(0.1f), m_far(far), m_position() {}
	camera(const window& wnd, float far = 10.f);
	virtual ~camera() = default;

public:
	void resize(const window& wnd);
	void move_to(const glm::vec3& pt) { m_position = pt; updateView(); }

	glm::mat4 projection() const { return m_projection; }
	glm::mat4 view() const { return m_view; }
	glm::mat4 matrix() const { return m_projection * m_view; }

	glm::vec3 position() const { return m_position; }

	float aspect() const { return m_aspect; }
	void set_aspect(float a) { m_aspect = a; updateProj(); }

	float near() const { return m_near; }
	float far()  const { return m_far; }
	void set_range(float n, float f) { m_near = n; m_far = f; updateProj(); }

protected:
	virtual void updateProj() { m_projection = glm::mat4(1.f); }
	virtual void updateView() { m_view = glm::mat4(1.f); }

protected:
	float m_aspect, m_near, m_far;
	glm::vec3 m_position;
	glm::mat4 m_projection, m_view;
};
