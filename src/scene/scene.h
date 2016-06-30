#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class scene
{
public:
	static glm::uvec2 window_size;
	static const char window_name[];

public:
	static void create(GLFWwindow* wnd);
	static void destroy();

	static void update();
	static void render();
};
