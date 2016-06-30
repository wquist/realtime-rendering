#include "vertex_array.h"

vertex_array::vertex_array()
: m_buffers(), m_ibuffer()
{
	m_vao = glref(glGenVertexArrays, glDeleteVertexArrays);
}

void vertex_array::attrib(const std::string& n, int i, size_t s, size_t o)
{
	assert(m_buffers.count(n));
	auto& buf = m_buffers.at(n);

	glBindVertexArray(m_vao);
	glEnableVertexAttribArray(i);

	auto stride = buf.element_stride();
	auto elems  = s ? s : buf.element_size();
	auto offset = reinterpret_cast<void*>(o);

	buf.bind();
	glVertexAttribPointer(i, elems, buf.type(), GL_FALSE, stride, offset);
	buf.bind(false);

	glBindVertexArray(0);
}

size_t vertex_array::size() const
{
	if (m_ibuffer.get())
		return m_ibuffer->size();
	
	if (m_buffers.empty())
		return 0;
	
	return m_buffers.begin()->second.size();
}
