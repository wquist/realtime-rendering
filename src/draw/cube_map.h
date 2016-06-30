#pragma once

#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../common/glref.h"
#include "../render/camera.h"
#include "../render/shader.h"
#include "../render/vertex_array.h"
#include "../render/texture.h"
#include "../resource/image.h"

class cube_map
{
public:
	cube_map();
	~cube_map() = default;

public:
	void attach(const image& tex, GLenum face);
	void attach(const std::vector<image>& texes);

	void bind(size_t index, bool on = true) const { m_texture.bind(index, on); }
	void draw(const camera& cam);

private:
	texture m_texture;
	vertex_array m_cdata;
	shader m_shader;
};
