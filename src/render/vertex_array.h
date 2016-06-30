#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <cassert>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../common/glref.h"
#include "buffer.h"

class vertex_array
{
public:
	vertex_array();
	vertex_array(vertex_array&&) = default;
	~vertex_array() = default;

public:
	template<typename T>
	void attach(const std::string& n, const std::vector<T>& b, int i = -1);
	void attrib(const std::string& n, int i, size_t s = 0, size_t o = 0);
	template<typename T>
	void index(const std::vector<T>& b);

	void bind(bool on = true) const { glBindVertexArray(on?m_vao:0); }

	template<typename T>
	void update(const std::string& n, const std::vector<T>& b);

	size_t size() const;

private:
	glref m_vao;
	std::map<std::string, buffer> m_buffers;
	std::unique_ptr<buffer> m_ibuffer;
};

template<typename T>
void vertex_array::attach(const std::string& n, const std::vector<T>& b, int i)
{
	assert(!m_buffers.count(n));

	auto buf = buffer(GL_ARRAY_BUFFER, b);
	m_buffers.emplace(n, std::move(buf));

	if (i >= 0)
		this->attrib(n, i);
}

template<typename T>
void vertex_array::index(const std::vector<T>& b)
{
	assert(!m_ibuffer);
	m_ibuffer = std::make_unique<buffer>(GL_ELEMENT_ARRAY_BUFFER, b);

	glBindVertexArray(m_vao);
	m_ibuffer->bind();
	glBindVertexArray(0);
	m_ibuffer->bind(false);
}

template<typename T>
void vertex_array::update(const std::string& n, const std::vector<T>& b)
{
	assert(m_buffers.count(n));
	m_buffers.at(n).update(b);
}
