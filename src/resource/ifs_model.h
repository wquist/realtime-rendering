#pragma once

#include "model.h"
#include <fstream>

class ifs_model : public model
{
private:
	const std::string file_header   = "IFS";
	const std::string vertex_header = "VERTICES";
	const std::string face_header   = "TRIANGLES";

public:
	using model::model;
	ifs_model(const std::string& file) : model() { this->load(file); }
	~ifs_model() = default;

public:
	void load(const std::string& file);

private:
	std::string read_string(std::ifstream& stream);
};
