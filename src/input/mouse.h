#pragma once

#include <map>
#include <functional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../render/window.h"

class mouse
{
public:
	enum class event { down, up };
	using callback = std::function<void(int)>;

public:
	static glm::uvec2 get(window& wnd, bool glspace = true);
	static glm::vec2  get_normalized(window& wnd);

	static void listen(window& w, event ev, const callback& cb);

private:
	static void on_input(GLFWwindow*, int btn, int act, int mod);

private:
	static std::map<event, callback> s_events;
};
