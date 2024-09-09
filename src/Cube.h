#pragma once
#include "Model.h"

class Cube : public Mesh {
public:
	Cube(const std::string& path_to_texture = "../assets/cute_dog.png");
	Cube(std::unique_ptr<Texture> tex);

	virtual ~Cube()
	{}
protected:
};