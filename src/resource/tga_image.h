#pragma once

#include "image.h"
#include <fstream>

class tga_image : public image
{
private:
	enum class header
	{
		id_length       = 0x00,
		colormap_type   = 0x01,
		datatype_code   = 0x02,
		colormap_origin = 0x03,
		colormap_length = 0x05,
		colormap_depth  = 0x07,
		origin_x        = 0x08,
		origin_y        = 0x0A,
		width           = 0x0C,
		height          = 0x0E,
		bpp             = 0x10,
		descriptor      = 0x11
	};

	enum class datatype
	{
		none         = 0x0,
		colormap_raw = 0x1,
		rgb_raw      = 0x2,
		bw_raw       = 0x3,
		colormap_rle = 0x9,
		rgb_rle      = 0xA,
	};

private:
	static constexpr const size_t header_length = 18;

public:
	using image::image;
	tga_image(const std::string& file) : image() { this->load(file); }
	~tga_image() = default;

public:
	void load(const std::string& file) override;

private:
	template<typename T>
	T get_field(GLubyte* h, header f);
};
