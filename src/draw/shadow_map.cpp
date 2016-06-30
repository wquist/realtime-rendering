#include "shadow_map.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 shadow_map::directional(const camera& c, const glm::vec3& d)
{
	auto extent = c.far() / 2.f;
	glm::mat4 proj = glm::ortho(-extent, extent, -extent, extent, 0.f, c.far());
	
	auto pos = glm::vec3(extent) * -d;
	glm::mat4 view = glm::lookAt(pos, glm::vec3(0.f), glm::vec3(1.f));

	return proj * view;
}

shadow_map::shadow_map(const window& wnd, const glm::mat4& vp, size_t sz)
: m_shader(), m_buffer(sz, sz), m_size(sz), m_transform(vp)
{
	m_screen = wnd.render_size();

	m_shader.compile("data/shader/shadow.vs", GL_VERTEX_SHADER);
	m_shader.compile("data/shader/shadow.fs", GL_FRAGMENT_SHADER);
	m_shader.link();

	m_shader.declare_uniform("M");
	m_shader.declare_uniform("VP");

	m_buffer.attach_depth(GL_DEPTH_COMPONENT24);
	m_buffer.finish();
}

void shadow_map::begin()
{
	m_buffer.bind();
	glViewport(0.f, 0.f, m_size, m_size);
	glClear(GL_DEPTH_BUFFER_BIT);

	m_shader.use();
	m_shader.set_uniform("VP", m_transform);
}

void shadow_map::end()
{
	m_shader.use(false);
	m_buffer.bind(false);

	glViewport(0.f, 0.f, m_screen.x, m_screen.y);
}
