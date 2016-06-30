#pragma once

#include <string>
#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "../common/exception.h"
#include "../common/aabb.h"

class model
{
public:
	class exception : public ::exception
	{
	public:
		using ::exception::exception;
		const char* source() const throw() { return "model"; }
	};

	struct mesh
	{
		mesh()  = default;
		~mesh() = default;

		bool indexed()       const { return indices.size(); }
		bool has_positions() const { return positions.size(); }
		bool has_texcoords() const { return texcoords.size(); }
		bool has_normals()   const { return normals.size(); }
		bool has_tangents()  const { return tangents.size(); }

		std::vector<unsigned int> indices;

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> texcoords;
		std::vector<glm::vec3> normals;

		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;

		std::string material;

		aabb<glm::vec3> bounds;
	};

	struct material
	{
		material()  = default;
		~material() = default;

		bool has_specular() const { return (specular != ""); }
		bool has_normal()   const { return (normal != ""); }
		bool has_alpha()    const { return (alpha != ""); }

		std::string diffuse;
		std::string specular;
		std::string normal;
		std::string alpha;
	};

public:
	model() = default;
	virtual ~model() = default;

public:
	void load(const std::string& file);
	void calculate_tangents();
	const aabb<glm::vec3>& calculate_bounds();

	const std::vector<mesh>& meshes() const { return m_meshes; }
	size_t mesh_count() const { return m_meshes.size(); }
	const mesh& get_mesh(size_t i = 0) const { return m_meshes[i]; }

	const std::map<std::string, material>& materials() const;
	size_t material_count() const { return m_materials.size(); }

	const aabb<glm::vec3>& bounds() const { return m_bounds; }

protected:
	std::vector<mesh> m_meshes;
	std::map<std::string, material> m_materials;
	aabb<glm::vec3> m_bounds;
};
