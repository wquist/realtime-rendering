#pragma once

#include <glm/glm.hpp>

#include "../render/window.h"
#include "../render/camera.h"
#include "../render/shader.h"
#include "../render/frame_buffer.h"

class shadow_map
{
public:
	static glm::mat4 directional(const camera& c, const glm::vec3& d);

public:
	shadow_map(const window& wnd, const glm::mat4& vp, size_t sz);
	~shadow_map() = default;

public:
	void begin();
	void will_render(const glm::mat4& m) { m_shader.set_uniform("M", m); }
	void end();

	void bind(size_t unit) const { m_buffer.bind_depth(unit); }

	glm::mat4 matrix() const { return m_transform; }
	void update(const glm::mat4& vp) { m_transform = vp; }

private:
	shader m_shader;
	frame_buffer m_buffer;

	float m_size;
	glm::uvec2 m_screen;
	glm::mat4 m_transform;
};
