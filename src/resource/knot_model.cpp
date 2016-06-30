#include "knot_model.h"
#include <glm/gtc/constants.hpp>

knot_model::knot_model(const glm::ivec2& pq, float r, size_t sg, size_t sl)
: model()
{
	this->generate(pq, r, sg, sl);
}

void knot_model::generate(const glm::ivec2& pq, float r, size_t sg, size_t sl)
{
	model::mesh mesh;

	float dt = (2.f * glm::pi<float>()) / sg;
	float du = (2.f * glm::pi<float>()) / sl;

	for (auto i = 0; i <= sg; ++i)
	{
		float t0 = i * dt;
		float r0 = (2.f + glm::cos(pq.y * t0)) * 0.5f;

		auto p0h = glm::vec2(glm::cos(pq.x * t0), glm::sin(pq.x * t0));
		auto p0  = glm::vec3(r0 * p0h, -glm::sin(pq.y * t0));

		float t1 = (i + 1) * dt;
		float r1 = (2.f + glm::cos(pq.y * t1)) * 0.5f;

		auto p1h = glm::vec2(glm::cos(pq.x * t1), glm::sin(pq.x * t1));
		auto p1  = glm::vec3(r1 * p1h, -glm::sin(pq.y * t1));

		auto t = p1 - p0;
		auto n = p1 + p0;
		auto b = glm::cross(t, n);

		n = glm::normalize(glm::cross(b, t));
		b = glm::normalize(b);

		for (auto j = 0; j <= sl; ++j)
		{
			float u = j * du;

			float x = r * glm::cos(u);
			float y = r * glm::sin(u);

			auto p2 = x * n + y * b;

			mesh.positions.push_back(p0 + p2);
			mesh.normals.push_back(glm::normalize(p2));

			mesh.bounds.expand(p0 + p2);
		}
	}

	for (auto i = 0; i != sg * (sl + 1); ++i)
	{
		mesh.indices.push_back(i);
		mesh.indices.push_back(i + sl + 1);
		mesh.indices.push_back(i + sl);

		mesh.indices.push_back(i);
		mesh.indices.push_back(i + 1);
		mesh.indices.push_back(i + 1 + sl);
	}

	m_meshes.push_back(mesh);
}
