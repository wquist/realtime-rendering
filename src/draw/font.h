#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../common/exception.h"
#include "../common/glref.h"
#include "../render/shader.h"
#include "../render/camera.h"
#include "../render/vertex_array.h"
#include "../render/texture.h"

class font
{
public:
	class exception : public ::exception
	{
	public:
		using ::exception::exception;
		const char* source() const throw() { return "font"; }
	};

	class text
	{
	public:
		text(glm::vec2 s, float k, float l);
		~text() = default;

	public:
		void update(const std::string& str);

		void bind(bool on = true) const { m_data.bind(on); }

		size_t size() const { return m_data.size() / 6; }
		size_t data_size() const { return m_data.size(); }

		glm::vec3 color() const { return m_color; }

	private:
		vertex_array m_data;

		glm::vec2 m_charsize;
		float m_kerning, m_leading;
		glm::vec3 m_color;
	};

public:
	font(const std::string& name, size_t size);
	~font() = default;

public:
	std::unique_ptr<text> create_text(float kern, float lead);

	void draw(const text& txt, const camera& cam, const glm::mat4& model);

private:
	glm::vec2 m_charsize;

	texture m_texture;
	shader  m_shader;
};
