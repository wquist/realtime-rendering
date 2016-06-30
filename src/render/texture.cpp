#include "texture.h"

texture::texture(GLenum mode)
: m_mode(mode)
{
	m_to = glref(glGenTextures, glDeleteTextures);
	this->set_filtering(GL_LINEAR, GL_LINEAR);
}

void texture::bind(int unit, bool on) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(m_mode, on?m_to:0);
}

void texture::set_data(const source& s, const target& d, size_t b)
{
	auto mode = (d.mode == GL_NONE) ? m_mode : d.mode;
	auto ifmt = d.storage;

	glBindTexture(m_mode, m_to);
	glTexImage1D(mode, d.level, ifmt, b, 0, s.format, s.type, s.raw);
	glBindTexture(m_mode, 0);
}

void texture::set_data(const source& s, const target& d, const glm::uvec2& b)
{
	auto mode = (d.mode == GL_NONE) ? m_mode : d.mode;
	auto ifmt = d.storage;

	glBindTexture(m_mode, m_to);
	glTexImage2D(mode, d.level, ifmt, b.x,b.y, 0, s.format, s.type, s.raw);
	glBindTexture(m_mode, 0);
}

void texture::set_data(const source& s, const target& d, const glm::uvec3& b)
{
	auto mode = (d.mode == GL_NONE) ? m_mode : d.mode;
	auto ifmt = d.storage;

	glBindTexture(m_mode, m_to);
	glTexImage3D(mode, d.level, ifmt, b.x,b.y,b.z, 0, s.format, s.type, s.raw);
	glBindTexture(m_mode, 0);
}

void texture::set_filtering(GLenum min, GLenum mag, GLenum mip)
{
	glBindTexture(m_mode, m_to);

	if (mip != GL_NONE)
	{
		glGenerateMipmap(m_mode);

		switch (min)
		{
			case GL_NEAREST:
				if (mip == GL_NEAREST)
					min = GL_NEAREST_MIPMAP_NEAREST;
				else
					min = GL_NEAREST_MIPMAP_LINEAR;
				break;
			case GL_LINEAR:
				if (mip == GL_NEAREST)
					min = GL_LINEAR_MIPMAP_NEAREST;
				else
					min = GL_LINEAR_MIPMAP_LINEAR;
				break;
		}
	}

	glTexParameteri(m_mode, GL_TEXTURE_MIN_FILTER, min);
	glTexParameteri(m_mode, GL_TEXTURE_MAG_FILTER, mag);

	glBindTexture(m_mode, 0);
}

void texture::set_wrap(GLenum s, GLenum t, GLenum r)
{
	glBindTexture(m_mode, m_to);

	glTexParameteri(m_mode, GL_TEXTURE_WRAP_S, s);
	if (t != GL_NONE)
		glTexParameteri(m_mode, GL_TEXTURE_WRAP_T, t);
	if (r != GL_NONE)
		glTexParameteri(m_mode, GL_TEXTURE_WRAP_R, r);

	glBindTexture(m_mode, 0);
}

