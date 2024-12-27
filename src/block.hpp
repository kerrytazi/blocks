#pragma once

#include "gfxengine/frame.hpp"

enum class Direction : uint8_t
{
	Left,        // -x (west)
	Right,       // +x (east)
	Down,        // -y
	Up,          // +y
	Forward,     // -z (north)
	Back,        // +z (south)
};

static inline const size_t DIRECTION_MAX = 6;

struct Cube
{
	enum class ID : uint8_t
	{
		Air,
	};

	ID id;
	bool solid;
	bool visible_faces[DIRECTION_MAX];

	void on_render(Frame &frame, glm::ivec3 block_coord)
	{
		if (!solid)
			return;

		glm::ivec3 p = block_coord;

		if (visible_faces[(int)Direction::Left])
			frame.add_quad({ p.x    , p.y    , p.z     }, { p.x    , p.y    , p.z + 1 }, { p.x    , p.y + 1, p.z + 1 }, { p.x    , p.y + 1, p.z     }, Color::RED);     // Left
		if (visible_faces[(int)Direction::Right])
			frame.add_quad({ p.x + 1, p.y    , p.z     }, { p.x + 1, p.y + 1, p.z     }, { p.x + 1, p.y + 1, p.z + 1 }, { p.x + 1, p.y    , p.z + 1 }, Color::YELLOW);  // Right
		if (visible_faces[(int)Direction::Down])
			frame.add_quad({ p.x    , p.y    , p.z     }, { p.x + 1, p.y    , p.z     }, { p.x + 1, p.y    , p.z + 1 }, { p.x    , p.y    , p.z + 1 }, Color::BLUE);    // Down
		if (visible_faces[(int)Direction::Up])
			frame.add_quad({ p.x    , p.y + 1, p.z     }, { p.x    , p.y + 1, p.z + 1 }, { p.x + 1, p.y + 1, p.z + 1 }, { p.x + 1, p.y + 1, p.z     }, Color::MAGENTA); // Up
		if (visible_faces[(int)Direction::Forward])
			frame.add_quad({ p.x    , p.y    , p.z     }, { p.x    , p.y + 1, p.z     }, { p.x + 1, p.y + 1, p.z     }, { p.x + 1, p.y    , p.z     }, Color::GREEN);   // Forward
		if (visible_faces[(int)Direction::Back])
			frame.add_quad({ p.x    , p.y    , p.z + 1 }, { p.x + 1, p.y    , p.z + 1 }, { p.x + 1, p.y + 1, p.z + 1 }, { p.x    , p.y + 1, p.z + 1 }, Color::CYAN);    // Back
	}
};
