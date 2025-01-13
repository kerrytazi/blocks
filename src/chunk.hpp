#pragma once

#include "gfxengine/graphics.hpp"

#include "block.hpp"
#include <vector>

struct Chunk
{
	static const size_t EDGE_SIZE = 16;
	static const size_t EDGE_LAST = EDGE_SIZE - 1;
	Cube blocks[EDGE_SIZE][EDGE_SIZE][EDGE_SIZE]{};

	void refresh_visible_faces();
	void hide_adjacent_chunk_faces(ivec3 delta, Chunk const &other);
	void on_render(Frame &frame, ivec3 chunk_coord, MaterialManager const &textures);
};
