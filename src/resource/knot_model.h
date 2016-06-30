#pragma once

#include "model.h"
#include <fstream>

class knot_model : public model
{
public:
	knot_model(const glm::ivec2& pq, float r, size_t sg, size_t sl);
	~knot_model() = default;

public:
	void generate(const glm::ivec2& pq, float r, size_t sg, size_t sl);
};
