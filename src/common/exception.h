#pragma once

#include <string>
#include <stdexcept>

class exception : public std::exception
{
public:
	exception(const std::string& f, const std::string& m)
	: m_where(f), m_what(m) {}

	virtual const char* source() const throw() { return "application"; }
	virtual const char* where()  const throw() { return m_where.c_str(); }
	virtual const char* what()   const throw() { return m_what.c_str(); }

private:
	const std::string m_where;
	const std::string m_what;
};
