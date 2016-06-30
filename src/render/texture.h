#pragma once

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../common/glref.h"

class texture
{
public:
	struct source
	{
		GLenum format, type;
		const void* const raw;

		template<typename T>
		source(const std::vector<T>& ps, GLenum f, GLenum t = GL_UNSIGNED_BYTE)
		: format(f), type(t), raw(ps.data()) {}
		source(GLenum f, GLenum t = GL_UNSIGNED_BYTE)
		: format(f), type(t), raw(nullptr) {}
		~source() = default;
	};

	struct target
	{
		GLenum mode;
		GLint  level;
		GLenum storage;

		target(GLenum s, GLint l = 0)
		: mode(GL_NONE), level(l), storage(s) {}
		~target() = default;
	};

public:
	texture(GLenum mode);
	~texture() = default;

public:
	void bind(bool on = true) const { glBindTexture(m_mode, on?m_to:0); }
	void bind(int unit, bool on = true) const;

	void set_data(const source& s, const target& d, size_t b);
	void set_data(const source& s, const target& d, const glm::uvec2& b);
	void set_data(const source& s, const target& d, const glm::uvec3& b);

	void set_filtering(GLenum min, GLenum mag, GLenum mip = GL_NONE);
	void set_wrap(GLenum s, GLenum t = GL_NONE, GLenum r = GL_NONE);

	GLuint raw() const { return m_to; }

private:
	glref  m_to;
	GLenum m_mode;
};
