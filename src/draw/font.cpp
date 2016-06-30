#include "font.h"
#include "../resource/tga_image.h"
#include <cassert>

font::text::text(glm::vec2 s, float k, float l)
: m_data(), m_charsize(s), m_kerning(k), m_leading(l), m_color(1.f)
{
	std::vector<glm::vec2> pos;
	std::vector<glm::vec2> tex;

	m_data.attach("position", pos);
	m_data.attach("texcoord", tex);

	m_data.attrib("position", 0);
	m_data.attrib("texcoord", 1);
}

void font::text::update(const std::string& str)
{
	std::vector<glm::vec2> pos;
	std::vector<glm::vec2> tex;

	auto emit = [this, &pos, &tex](float x, float y, char c)
	{
		auto w = m_charsize.x;
		auto h = m_charsize.y;

		const glm::vec2 verts[] =
		{
			{ x,     y     },
			{ x + w, y - h },
			{ x,     y - h },
			{ x,     y     },
			{ x + w, y     },
			{ x + w, y - h },
		};

		auto u  = static_cast<float>((c >> 0) & 0xF) * (1.f / 16.f);
		auto v  = static_cast<float>((c >> 4) & 0x7) * (1.f /  8.f);
		auto su = 1.f / 16.f;
		auto sv = 1.f /  8.f;

		const glm::vec2 coords[] =
		{
			{ u,      v,     },
			{ u + su, v + sv },
			{ u,      v + sv },
			{ u,      v      },
			{ u + su, v      },
			{ u + su, v + sv }
		};

		pos.insert(pos.end(), verts,  verts+6);
		tex.insert(tex.end(), coords, coords+6);
	};

	float x = 0, y = 0;
	for (auto& c : str)
	{
		if (c == '\n')
		{
			x  = 0.f;
			y -= m_charsize.y + m_leading;
			continue;
		}

		emit(x, y, c);
		x += m_charsize.x + m_kerning;
	}

	m_data.update("position", pos);
	m_data.update("texcoord", tex);
}

font::font(const std::string& name, size_t size)
: m_texture(GL_TEXTURE_2D), m_shader()
{
	m_shader.compile("data/shader/font.vs", GL_VERTEX_SHADER);
	m_shader.compile("data/shader/font.fs", GL_FRAGMENT_SHADER);
	m_shader.link();

	m_shader.declare_uniform("MVP");
	m_shader.declare_uniform("Color");
	m_shader.declare_uniform("Texture");

	auto file = "data/font/" + name + std::to_string(size) + ".tga";
	tga_image tex(file);
	if (tex.bpp() != 32)
		throw exception(file, "font does not contain alpha channel");

	texture::source src(tex.pixels(), tex.format());
	texture::target dst(GL_RGBA);

	m_texture.set_data(src, dst, tex.size());
	m_texture.set_filtering(GL_NEAREST, GL_NEAREST);

	auto cw = static_cast<float>(tex.width())  / 16.f;
	auto ch = static_cast<float>(tex.height()) /  8.f;
	m_charsize = glm::vec2(cw, ch);
}

std::unique_ptr<font::text> font::create_text(float kern, float lead)
{
	return std::make_unique<text>(m_charsize, kern, lead);
}

void font::draw(const text& txt, const camera& cam, const glm::mat4& model)
{
	assert(txt.size());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_shader.use();
	txt.bind();
	m_texture.bind(0);

	m_shader.set_uniform("MVP", cam.matrix() * model);
	m_shader.set_uniform("Color", txt.color());
	m_shader.set_uniform("Texture", 0);

	glDrawArrays(GL_TRIANGLES, 0, txt.data_size());

	m_texture.bind(0, false);
	txt.bind(false);
	m_shader.use(false);

	glDisable(GL_BLEND);
}
