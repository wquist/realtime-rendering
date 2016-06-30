#include "obj_model.h"
#include <fstream>

void obj_model::load(const std::string& file, bool flip_uvs)
{
	m_meshes.clear();

	std::ifstream file_stream(file.c_str());
	if (!file_stream.is_open())
		throw model::exception(file, "file not found");

	std::string line;
	size_t line_num = 1;
	
	mesh_data d;
	while (std::getline(file_stream, line))
	{
		std::stringstream stream(line);

		std::string header;
		if (!(stream >> header))
			continue;

		try
		{
			if (header == position_header)
				d.positions.push_back(this->parse_position(stream));
			else if (header == texcoord_header)
				d.texcoords.push_back(this->parse_texcoord(stream, flip_uvs));
			else if (header == normal_header)
				d.normals.push_back(this->parse_normal(stream));
			else if (header == face_header)
				for (auto& i : this->parse_face(stream))
					d.indices.push_back(i);
			else if (header == referece_header)
			{
				if (d.indices.size())
					m_meshes.emplace_back(std::move(this->format_mesh(d)));

				d.material = this->parse_reference(stream);
			}
		}
		catch (model::exception& e)
		{
			auto what = e.what();
			this->error(file, line_num, what);
		}

		line_num += 1;
	}

	m_meshes.emplace_back(std::move(this->format_mesh(d)));
}

void obj_model::load_materials(const std::string& file)
{
	m_materials.clear();

	std::ifstream file_stream(file.c_str());
	if (!file_stream.is_open())
		throw model::exception(file, "file not found");
	
	std::string line;
	size_t line_num = 1;

	std::string material = "";
	while (std::getline(file_stream, line))
	{
		std::stringstream stream(line);

		std::string header;
		if (!(stream >> header))
			continue;

		try
		{
			if (header == material_header)
				material = this->parse_name(stream);
			else if (header == diffuse_header)
				m_materials[material].diffuse = this->parse_map(stream);
			else if (header == specular_header)
				m_materials[material].specular = this->parse_map(stream);
			else if (header == alpha_header)
				m_materials[material].alpha = this->parse_map(stream);
			else if (header == bump_header)
				m_materials[material].normal = this->parse_map(stream);
		}
		catch (model::exception& e)
		{
			auto what = e.what();
			this->error(file, line_num, what);
		}

		line_num += 1;
	}
}

glm::vec3 obj_model::parse_position(std::stringstream& stream)
{
	glm::vec3 v;
	if (!(stream >> v.x >> v.y >> v.z))
		throw model::exception("", "bad vertex");

	return v;
}

glm::vec2 obj_model::parse_texcoord(std::stringstream& stream, bool flip_uvs)
{
	glm::vec2 t;
	if (!(stream >> t.x >> t.y))
		throw model::exception ("", "bad texcoord");

	if (flip_uvs)
		t.y = -t.y;

	return t;
}

glm::vec3 obj_model::parse_normal(std::stringstream& stream)
{
	glm::vec3 n;
	if (!(stream >> n.x >> n.y >> n.z))
		throw model::exception("", "bad normal");

	return n;
}

bool obj_model::parse_index(std::stringstream& stream, glm::uvec3& i)
{
	char slash;
	i = glm::uvec3();

	if (!(stream >> i[0]))
		return false;
	if (stream.peek() != '/')
		return true;

	if (!(stream >> slash))
		return false;

	if (stream.peek() == '/')
		return (bool)(stream >> slash >> i[2]);
	
	if (!(stream >> i[1]))
		return false;
	if (stream.peek() != '/')
		return true;

	return (bool)(stream >> slash >> i[2]);
}

std::vector<glm::uvec3> obj_model::parse_face(std::stringstream& stream)
{
	glm::uvec3 i;
	std::vector<glm::uvec3> is;

	while (this->parse_index(stream, i))
		is.push_back(i);

	if (is.size() < 3)
		throw model::exception("", "bad face");

	std::vector<glm::uvec3> indices;
	for (auto i = 1; i != is.size() - 1; ++i)
	{
		indices.push_back(is[0  ]);
		indices.push_back(is[i  ]);
		indices.push_back(is[i+1]);
	}

	return indices;
}

std::string obj_model::parse_reference(std::stringstream& stream)
{
	std::string r;
	if (!(stream >> r))
		throw model::exception("", "bad material reference");

	return r;
}

model::mesh obj_model::format_mesh(mesh_data& data)
{
	model::mesh mesh;
	for (auto i = 0; i != data.indices.size(); ++i)
	{
		auto v = data.indices[i][0];
		auto t = data.indices[i][1];
		auto n = data.indices[i][2];

		mesh.positions.push_back(data.positions[v-1]);
		if (t) mesh.texcoords.push_back(data.texcoords[t-1]);
		if (n) mesh.normals.push_back(data.normals[n-1]);

		mesh.bounds.expand(data.positions[v-1]);
	}

	mesh.material = data.material;

	data.indices.clear();
	return mesh;
}

std::string obj_model::parse_name(std::stringstream& stream)
{
	std::string n;
	if (!(stream >> n))
		throw model::exception("", "bad material name");

	return n;
}

std::string obj_model::parse_map(std::stringstream& stream)
{
	std::string m;
	if (!(stream >> m))
		throw model::exception("", "bad map file");

	return m;
}

void obj_model::error(const std::string& f, size_t line, const std::string& m)
{
	std::string where = f + ":" + std::to_string(line);
	throw model::exception(where, m);
}
