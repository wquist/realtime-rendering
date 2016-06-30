#pragma once

#include "model.h"
#include <sstream>

class obj_model : public model
{
private:
	struct mesh_data
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> texcoords;
		std::vector<glm::vec3> normals;

		std::vector<glm::uvec3> indices;

		std::string group;
		std::string material;
	};

private:
	const std::string position_header = "v";
	const std::string texcoord_header = "vt";
	const std::string normal_header   = "vn";
	const std::string face_header     = "f";
	const std::string referece_header = "usemtl";

	const std::string material_header = "newmtl";
	const std::string diffuse_header  = "map_Kd";
	const std::string specular_header = "map_Ks";
	const std::string alpha_header    = "map_d";
	const std::string bump_header     = "bump";

public:
	using model::model;
	obj_model(const std::string& file, bool flip_uvs)
	: model() { this->load(file, flip_uvs); }
	~obj_model() = default;

public:
	void load(const std::string& file) { this->load(file, true); }
	void load(const std::string& file, bool flip_uvs);

	void load_materials(const std::string& file);

private:
	glm::vec3 parse_position(std::stringstream& stream);
	glm::vec2 parse_texcoord(std::stringstream& stream, bool flip_uvs);
	glm::vec3 parse_normal(std::stringstream& stream);
	std::vector<glm::uvec3> parse_face(std::stringstream& stream);
	bool parse_index(std::stringstream& stream, glm::uvec3& i);
	std::string parse_reference(std::stringstream& stream);
	model::mesh format_mesh(mesh_data& data);

	std::string parse_name(std::stringstream& stream);
	std::string parse_map(std::stringstream& stream);

	void error(const std::string& file, size_t line, const std::string& msg);
};
