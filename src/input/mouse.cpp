#include "mouse.h"
#include <cstddef>
#include <cassert>

std::map<mouse::event, mouse::callback> mouse::s_events;

glm::uvec2 mouse::get(window& wnd, bool glspace)
{
	double x,y;
	glfwGetCursorPos(wnd.raw(), &x, &y);

	auto mx = static_cast<size_t>(x);
	auto my = static_cast<size_t>(y);

	if (glspace)
		my = wnd.height() - my;

	return glm::uvec2(mx, my);
}

glm::vec2 mouse::get_normalized(window& wnd)
{
	auto pos = glm::vec2(mouse::get(wnd, true));
	return pos / glm::vec2(wnd.size()) * 2.f - 1.f;

	return pos;
}

void mouse::listen(window& w, event ev, const callback& cb)
{
	assert(!s_events.count(ev));
	s_events.emplace(ev, cb);

	glfwSetMouseButtonCallback(w.raw(), on_input);
}

void mouse::on_input(GLFWwindow*, int btn, int act, int mod)
{
	event ev;
	if (act == GLFW_PRESS)
		ev = event::down;
	else if (act == GLFW_RELEASE)
		ev = event::up;
	else
		return;

	if (s_events.count(ev))
		s_events.at(ev)(btn);
}
