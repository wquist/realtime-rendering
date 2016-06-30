#include "keyboard.h"

std::map<keyboard::event, keyboard::callback> keyboard::s_events;

bool keyboard::get(window& w, int key)
{
	auto state = glfwGetKey(w.raw(), key);
	return (state == GLFW_PRESS);
}

void keyboard::listen(window& w, event ev, const callback& cb)
{
	assert(!s_events.count(ev));
	s_events.emplace(ev, cb);

	glfwSetKeyCallback(w.raw(), on_input);
}

void keyboard::on_input(GLFWwindow*, int key, int, int act, int)
{
	event ev;
	if (act == GLFW_PRESS)
		ev = event::down;
	else if (act == GLFW_RELEASE)
		ev = event::up;
	else
		return;

	if (s_events.count(ev))
		s_events.at(ev)(key);
}
