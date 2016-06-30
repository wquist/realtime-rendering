#pragma once

#include <string>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../common/exception.h"
#include "../common/glref.h"

class shader
{
public:
	class exception : public ::exception
	{
	public:
		using ::exception::exception;
		const char* source() const throw() { return "shader"; }
	};

public:
	shader();
	~shader() = default;

public:
	void compile(const std::string& file, GLenum mode);
	void link();

	void use(bool on = true) const { glUseProgram(on?m_program:0); }

	void declare_uniform(const std::string& name, const std::string& var = "");

	void set_uniform(const std::string& n, float v);
	void set_uniform(const std::string& n, GLint v);
	void set_uniform(const std::string& n, const glm::vec2& v);
	void set_uniform(const std::string& n, const glm::vec3& v);
	void set_uniform(const std::string& n, const glm::mat4& v);

private:
	void error(const std::string& where, GLuint obj, bool prog);

private:
	glref m_program;
	std::vector<GLuint> m_shaders;
	std::map<std::string, GLuint> m_uniforms;
};
