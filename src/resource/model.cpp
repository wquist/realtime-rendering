#include "model.h"

void model::load(const std::string& file)
{
	throw model::exception("model", "loading not supported");
}

void model::calculate_tangents()
{
	for (auto& m : m_meshes)
	{
		assert(m.has_positions() && m.has_texcoords() && m.has_normals());

		m.tangents.clear();
		m.bitangents.clear();

		for (auto i = 0; i != m.positions.size(); i += 3)
		{
			auto& p0 = m.positions[i];
			auto& p1 = m.positions[i+1];
			auto& p2 = m.positions[i+2];

			auto& u0 = m.texcoords[i];
			auto& u1 = m.texcoords[i+1];
			auto& u2 = m.texcoords[i+2];

			auto dp1 = p1 - p0;
			auto dp2 = p2 - p0;
			auto du1 = u1 - u0;
			auto du2 = u2 - u0;

			auto r = 1.f / (du1.x * du2.y - du1.y * du2.x);
			auto t = (dp1 * du2.y - dp2 * du1.y) * r;
			auto b = (dp2 * du1.x - dp1 * du2.x) * r;

			for (auto j = 0; j != 3; ++j)
			{
				auto n  = m.normals[i+j];
				auto to = glm::normalize(t - n * glm::dot(n, t));

				m.tangents.push_back(to);
				m.bitangents.push_back(b);
			}
		}
	}
}

const aabb<glm::vec3>& model::calculate_bounds()
{
	m_bounds.clear();
	for (auto& m : m_meshes)
		m_bounds.expand(m.bounds);

	return m_bounds;
}

const std::map<std::string, model::material>& model::materials() const
{
	return m_materials;
}
