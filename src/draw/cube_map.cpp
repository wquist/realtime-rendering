#include "cube_map.h"
#include "../resource/cube_model.h"
#include <cassert>

cube_map::cube_map()
: m_texture(GL_TEXTURE_CUBE_MAP), m_cdata(), m_shader()
{
	auto cmodel = cube_model();
	auto cmesh  = cmodel.get_mesh();

	m_cdata.attach("position", cmesh.positions);
	m_cdata.attrib("position", 0);
	m_cdata.index(cmesh.indices);

	m_shader.compile("data/shader/cubemap.vs", GL_VERTEX_SHADER);
	m_shader.compile("data/shader/cubemap.fs", GL_FRAGMENT_SHADER);
	m_shader.link();

	m_shader.declare_uniform("VP");
	m_shader.declare_uniform("CubeMap");

	m_texture.set_wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

void cube_map::attach(const image& tex, GLenum face)
{
	texture::source src(tex.pixels(), tex.format());
	texture::target dst((tex.bpp() == 24) ? GL_RGB : GL_RGBA);

	dst.mode = face;
	m_texture.set_data(src, dst, tex.size());
}

void cube_map::attach(const std::vector<image>& texes)
{
	assert(texes.size() == 6);

	for (auto i = 0; i != 6; ++i)
		this->attach(texes[i], GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
}

void cube_map::draw(const camera& cam)
{
	auto view = glm::mat4(glm::mat3(cam.view()));
	auto vp   = cam.projection() * view;

	glDepthMask(GL_FALSE);

	m_shader.use();
	m_cdata.bind();
	m_texture.bind(0);

	m_shader.set_uniform("VP", vp);
	m_shader.set_uniform("CubeMap", 0);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

	m_texture.bind(0, false);
	m_cdata.bind(false);
	m_shader.use(false);

	glDepthMask(GL_TRUE);
}
