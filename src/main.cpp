#include <cassert>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "scene/scene.h"

int main()
{
	auto glfw_result = glfwInit();
	assert(glfw_result);

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	auto size = scene::window_size;
	auto name = scene::window_name;

	GLFWwindow* wnd = glfwCreateWindow(size.x, size.y, name, nullptr, nullptr);
	assert(wnd);

	glfwMakeContextCurrent(wnd);
	glfwSetInputMode(wnd, GLFW_STICKY_KEYS, GL_TRUE);

	glewExperimental = true;

	auto glew_result = glewInit();
	assert(glew_result == GLEW_OK);

	scene::create(wnd);
	while (true)
	{
		scene::update();
		scene::render();

		glfwSwapBuffers(wnd);
		glfwPollEvents();

		if (glfwGetKey(wnd, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			break;
		if (glfwWindowShouldClose(wnd))
			break;
	}

	scene::destroy();
	return 0;
}
