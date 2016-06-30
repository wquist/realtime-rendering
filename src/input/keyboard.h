#pragma once

#include <map>
#include <functional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../render/window.h"

class keyboard
{
public:
	enum class event { down, up };
	using callback = std::function<void(int)>;

public:
	static bool get(window& w, int key);
	static void listen(window& w, event ev, const callback& cb);

private:
	static void on_input(GLFWwindow*, int key, int, int act, int);

private:
	static std::map<event, callback> s_events;
};
