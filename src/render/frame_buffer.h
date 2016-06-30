#pragma once

#include <memory>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../common/exception.h"
#include "../common/glref.h"
#include "window.h"
#include "texture.h"

class frame_buffer
{
public:
	class exception : public ::exception
	{
	public:
		using ::exception::exception;
		const char* source() const throw() { return "frame buffer"; }
	};

	enum operation { read, write };

public:
	frame_buffer(size_t w, size_t h);
	frame_buffer(const window& wnd)
	: frame_buffer(wnd.render_size().x, wnd.render_size().y) {}
	frame_buffer(const glm::uvec2& s)
	: frame_buffer(s.x, s.y) {}
	~frame_buffer() = default;

public:
	void attach_color(size_t index, GLenum fmt, GLenum inf = GL_NONE);
	void attach_color(size_t index, texture&& tex);
	void attach_depth(GLenum inf = GL_NONE);
	void finish();

	void write_to(std::vector<size_t> indices);
	void write_to(size_t index) { write_to({ index }); }
	void write_to_all();
	void read_from(size_t index);

	void bind(bool on = true) const;
	void bind_for(enum operation op, bool on = true) const;

	void bind_color(size_t index, size_t unit, bool on = true) const;
	void bind_depth(size_t unit, bool on = true) const;

	glm::uvec2 size() const { return m_size; }
	size_t width()    const { return m_size.x; }
	size_t height()   const { return m_size.y; }

private:
	glref m_fbo;

	glm::uvec2 m_size;
	std::map<GLenum, texture> m_textures;
	glref m_rbo;
};
