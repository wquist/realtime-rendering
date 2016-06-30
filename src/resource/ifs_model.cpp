#include "ifs_model.h"
#include <glm/gtc/type_ptr.hpp>

void ifs_model::load(const std::string& file)
{
	m_meshes.clear();
	model::mesh mesh;

	std::ifstream stream(file.c_str());
	if (!stream.is_open())
		throw model::exception(file, "file not found");

	if (this->read_string(stream) != file_header)
		throw model::exception(file, "bad file header");
	if (!stream.ignore(sizeof(float)))
		throw model::exception(file, "bad file version");
	if (!this->read_string(stream).length())
		throw model::exception(file, "bad model name");

	uint32_t vertex_count;
	auto vertex_count_raw = reinterpret_cast<char*>(&vertex_count);

	if (this->read_string(stream) != vertex_header)
		throw model::exception(file, "bad vertex header");
	if (!stream.read(vertex_count_raw, sizeof(uint32_t)))
		throw model::exception(file, "bad vertex count");

	for (auto i = 0; i != vertex_count; ++i)
	{
		glm::f32vec3 v;
		auto v_raw = reinterpret_cast<char*>(glm::value_ptr(v));

		if (!stream.read(v_raw, sizeof(glm::f32vec3)))
			throw model::exception(file, "bad vertex");

		mesh.positions.push_back(glm::vec3(v));
		mesh.bounds.expand(glm::vec3(v));
	}

	uint32_t face_count;
	auto face_count_raw = reinterpret_cast<char*>(&face_count);

	if (this->read_string(stream) != face_header)
		throw model::exception(file, "bad face header");
	if (!stream.read(face_count_raw, sizeof(uint32_t)))
		throw model::exception(file, "bad face count");

	mesh.normals.resize(mesh.positions.size());
	for (auto i = 0; i != face_count; ++i)
	{
		glm::u32vec3 f;
		auto f_raw = reinterpret_cast<char*>(glm::value_ptr(f));

		if (!stream.read(f_raw, sizeof(glm::u32vec3)))
			throw model::exception(file, "bad face");

		for (auto j = 0; j != 3; ++j)
			mesh.indices.push_back(f[j]);

		auto u = mesh.positions[f[1]] - mesh.positions[f[0]];
		auto v = mesh.positions[f[2]] - mesh.positions[f[0]];
		auto n = glm::cross(u, v);

		for (auto j = 0; j != 3; ++j)
			mesh.normals[f[j]] = n;
	}

	m_meshes.push_back(mesh);
}

std::string ifs_model::read_string(std::ifstream& stream)
{
	uint32_t len;
	char* len_raw = reinterpret_cast<char*>(&len);

	if (!stream.read(len_raw, sizeof(uint32_t)))
		return "";

	auto data = new char[len];
	if (!stream.read(data, len))
		return "";

	std::string res(data);
	delete[] data;

	return res;
}
