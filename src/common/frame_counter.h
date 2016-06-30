#pragma once

#include <cstddef>

class frame_counter
{
public:
	frame_counter();
	~frame_counter() = default;

public:
	void update();

	bool dirty() { return m_dirty; }
	double frame_time() { m_dirty = false; return 1000.0 / m_frames; }
	double fps() { m_dirty = false; return m_frames; }

private:
	double m_last;
	size_t m_count;

	bool m_dirty;
	double m_frames;
};
