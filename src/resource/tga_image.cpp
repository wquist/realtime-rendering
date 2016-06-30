#include "tga_image.h"

template<> uint8_t  tga_image::get_field<uint8_t>(GLubyte* h, header f);
template<> uint16_t tga_image::get_field<uint16_t>(GLubyte* h, header f);

void tga_image::load(const std::string& file)
{
	std::ifstream stream(file.c_str());
	if (!stream.is_open())
		throw image::exception(file, "file not found");

	GLubyte h[header_length];
	auto h_raw = reinterpret_cast<char*>(h);

	if (!stream.read(h_raw, header_length))
		throw image::exception(file, "invalid header");

	auto dt_raw = this->get_field<uint8_t>(h, header::datatype_code);

	auto dt = static_cast<datatype>(dt_raw);
	if (!(dt == datatype::rgb_raw || dt == datatype::bw_raw))
		throw image::exception(file, "unsupported data type");

	m_width  = this->get_field<uint16_t>(h, header::width);
	m_height = this->get_field<uint16_t>(h, header::height);

	m_bpp = this->get_field<uint8_t>(h, header::bpp);
	switch (m_bpp)
	{
		case 8:
			m_format = GL_RED;
			break;
		case 24:
			m_format = GL_BGR;
			break;
		case 32:
			m_format = GL_BGRA;
			break;
		default:
			throw image::exception(file, "unsupported pixel format");
	}

	auto idlen = this->get_field<uint8_t>(h, header::id_length);
	if (idlen && !stream.ignore(idlen))
		throw image::exception(file, "invalid identification field");

	auto cm = this->get_field<uint8_t>(h, header::colormap_type);
	if (cm)
		throw image::exception(file, "extraneous color map data");

	size_t pixw = m_bpp / 8;
	size_t size = pixw * m_width * m_height;
	auto data = new GLubyte[size];

	auto data_raw = reinterpret_cast<char*>(data);
	if (!stream.read(data_raw, size))
		throw image::exception(file, "invalid pixel data");

	size_t step = pixw * m_width;
	for (auto i = 0; i != size; i += step)
	{
		auto start = &data_raw[size - (i + step)];
		m_pixels.insert(m_pixels.end(), start, start + step);
	}

	delete[] data;
}

template<>
uint8_t tga_image::get_field<uint8_t>(GLubyte* h, header f)
{
	auto idx = static_cast<size_t>(f);
	return static_cast<uint8_t>(h[idx]);
}

template<>
uint16_t tga_image::get_field<uint16_t>(GLubyte* h, header f)
{
	auto idx = static_cast<size_t>(f);
	return *reinterpret_cast<uint16_t*>(&h[idx]);
}
