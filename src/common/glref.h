#pragma once

#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class glref : private std::shared_ptr<GLuint>
{
private:
	using reference_ctor = void (*)(GLsizei, GLuint*);
	using reference_dtor = void (*)(GLsizei, const GLuint*);
	using direct_ctor = GLuint (*)();
	using direct_dtor = void (*)(GLuint);

public:
	explicit glref() : std::shared_ptr<GLuint>() {}
	glref(const reference_ctor& ctor, const reference_dtor& dtor)
	: std::shared_ptr<GLuint>(new GLuint, [=](GLuint* o) {dtor(1,o);delete o;})
	{
		ctor(1, this->get());
	}

	glref(const direct_ctor& ctor, const direct_dtor& dtor)
	: std::shared_ptr<GLuint>(new GLuint, [=](GLuint* o) {dtor(*o);delete o;})
	{
		*(this->get()) = ctor();
	}

public:
	bool valid() const { return this->get(); }
	operator GLuint() const { return *(this->get()); }
};
