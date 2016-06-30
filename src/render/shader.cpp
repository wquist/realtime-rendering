#include "shader.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <glm/gtc/type_ptr.hpp>

shader::shader()
{
	m_program = glref(glCreateProgram, glDeleteProgram);
}

void shader::compile(const std::string& file, GLenum mode)
{
	GLuint shader = glCreateShader(mode);

	std::ifstream stream(file.c_str());
	if (!stream.is_open())
		throw shader::exception(file, "file not found");

	auto start = std::istreambuf_iterator<char>(stream);
	auto end   = std::istreambuf_iterator<char>();
	std::string contents(start, end);

	stream.close();

	const char* raw = contents.c_str();
	glShaderSource(shader, 1, &raw, nullptr);
	glCompileShader(shader);

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE)
		this->error(file, shader, false);

	m_shaders.push_back(shader);
}

void shader::link()
{
	for (GLuint& shader : m_shaders)
		glAttachShader(m_program, shader);

	glLinkProgram(m_program);

	GLint result;
	glGetProgramiv(m_program, GL_LINK_STATUS, &result);
	if (result != GL_TRUE)
		this->error("program linkage", m_program, true);

	for (GLuint& shader : m_shaders)
	{
		glDetachShader(m_program, shader);
		glDeleteShader(shader);
	}

	m_shaders.clear();
}

void shader::declare_uniform(const std::string& name, const std::string& var)
{
	std::string glname = (var.length()) ? var : name;

	GLint id = glGetUniformLocation(m_program, glname.c_str());
	if (id == -1)
	{
		const std::string where = "uniform binding";
		const std::string what  = "'" + glname + "' undeclared";

		throw exception(where, what);
	}

	m_uniforms.emplace(name, static_cast<GLuint>(id));
}

void shader::set_uniform(const std::string& n, float v)
{
	assert(m_uniforms.count(n));
	GLint id = m_uniforms.at(n);

	glUseProgram(m_program);
	glUniform1f(id, v);
}

void shader::set_uniform(const std::string& n, GLint v)
{
	assert(m_uniforms.count(n));
	GLint id = m_uniforms.at(n);

	glUseProgram(m_program);
	glUniform1i(id, v);
}

void shader::set_uniform(const std::string& n, const glm::vec2& v)
{
	assert(m_uniforms.count(n));
	GLint id = m_uniforms.at(n);

	glUseProgram(m_program);
	glUniform2fv(id, 1, glm::value_ptr(v));
}

void shader::set_uniform(const std::string& n, const glm::vec3& v)
{
	assert(m_uniforms.count(n));
	GLint id = m_uniforms.at(n);

	glUseProgram(m_program);
	glUniform3fv(id, 1, glm::value_ptr(v));
}

void shader::set_uniform(const std::string& n, const glm::mat4& v)
{
	assert(m_uniforms.count(n));
	GLint id = m_uniforms.at(n);

	glUseProgram(m_program);
	glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(v));
}

void shader::error(const std::string& where, GLuint obj, bool prog)
{
	auto getiv  = (prog) ? glGetProgramiv      : glGetShaderiv;
	auto getlog = (prog) ? glGetProgramInfoLog : glGetShaderInfoLog;

	int log_length;
	getiv(obj, GL_INFO_LOG_LENGTH, &log_length);
	assert(log_length);

	std::vector<char> message(log_length + 1);
	getlog(obj, log_length, nullptr, message.data());

	auto what = std::string(message.begin(), message.end());
	throw exception(where, what);
}

