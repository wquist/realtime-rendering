#include "frame_buffer.h"
#include <string>
#include <vector>
#include <cassert>

frame_buffer::frame_buffer(size_t w, size_t h)
: m_textures(), m_rbo(), m_size(w, h)
{
	m_fbo = glref(glGenFramebuffers, glDeleteFramebuffers);
}

void frame_buffer::attach_color(size_t index, GLenum fmt, GLenum inf)
{
	auto pos = GL_COLOR_ATTACHMENT0 + index;
	assert(!m_textures.count(pos));

	texture::source src(fmt);
	texture::target dst((inf != GL_NONE) ? inf : fmt);

	auto tex = texture(GL_TEXTURE_2D);
	tex.set_data(src, dst, m_size);
	tex.set_wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	m_textures.emplace(pos, std::move(tex));
}

void frame_buffer::attach_color(size_t index, texture&& tex)
{
	auto pos = GL_COLOR_ATTACHMENT0 + index;
	assert(!m_textures.count(pos));

	m_textures.emplace(pos, tex);
}

void frame_buffer::attach_depth(GLenum inf)
{
	if (inf != GL_NONE)
	{
		assert(!m_textures.count(GL_DEPTH_ATTACHMENT));

		texture::source src(GL_DEPTH_COMPONENT);
		texture::target dst(inf);

		auto tex = texture(GL_TEXTURE_2D);
		tex.set_data(src, dst, m_size);
		tex.set_wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		m_textures.emplace(GL_DEPTH_ATTACHMENT, std::move(tex));
	}
	else
	{
		inf   = GL_DEPTH_COMPONENT;
		m_rbo = glref(glGenRenderbuffers, glDeleteRenderbuffers);

		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, inf, m_size.x, m_size.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
}

void frame_buffer::finish()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	for (auto& tex : m_textures)
		glFramebufferTexture(GL_FRAMEBUFFER, tex.first, tex.second.raw(), 0);

	if (m_rbo.valid())
	{
		GLenum fmt = GL_DEPTH_ATTACHMENT;
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, fmt, GL_RENDERBUFFER, m_rbo);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		const std::string where = "set draw buffers";
		const std::string what  = "framebuffer not complete";

		throw exception(where, what);
	}

	this->write_to_all();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void frame_buffer::write_to(std::vector<size_t> indices)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	std::vector<GLenum> attachments;
	for (auto& i : indices)
	{
		auto pos = GL_COLOR_ATTACHMENT0 + i;
		assert(m_textures.count(pos));

		attachments.push_back(pos);
	}

	glDrawBuffers(attachments.size(), attachments.data());
}

void frame_buffer::write_to_all()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	std::vector<GLenum> attachments;
	for (auto& tex : m_textures)
		if (tex.first != GL_DEPTH_ATTACHMENT)
			attachments.push_back(tex.first);

	glDrawBuffers(attachments.size(), attachments.data());
}

void frame_buffer::read_from(size_t index)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	
	auto pos = GL_COLOR_ATTACHMENT0 + index;
	assert(m_textures.count(pos));

	glReadBuffer(pos);
}

void frame_buffer::bind(bool on) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, on?m_fbo:0);
}

void frame_buffer::bind_for(enum operation op, bool on) const
{
	GLenum mode;
	switch (op)
	{
		case read:
			mode = GL_READ_FRAMEBUFFER;
			break;
		case write:
			mode = GL_DRAW_FRAMEBUFFER;
			break;
	}

	glBindFramebuffer(mode, on?m_fbo:0);
}

void frame_buffer::bind_color(size_t index, size_t unit, bool on) const
{
	GLenum pos = GL_COLOR_ATTACHMENT0 + index;
	assert(m_textures.count(pos));

	glActiveTexture(on ? GL_TEXTURE0 + unit : 0);
	m_textures.at(pos).bind(on);
}

void frame_buffer::bind_depth(size_t unit, bool on) const
{
	assert(m_textures.count(GL_DEPTH_ATTACHMENT));

	glActiveTexture(on ? GL_TEXTURE0 + unit : 0);
	m_textures.at(GL_DEPTH_ATTACHMENT).bind(on);
}
