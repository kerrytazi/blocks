#pragma once

#include "gfxengine/graphics.hpp"

#include "block.hpp"
#include <vector>

struct Chunk
{
	static const size_t EDGE_SIZE = 16;
	static const size_t EDGE_LAST = EDGE_SIZE - 1;
	Cube blocks[EDGE_SIZE][EDGE_SIZE][EDGE_SIZE];

	bool greedy_meshing = true;

	bool graphics_cache_dirty = false;
	std::shared_ptr<FrameCacheGraphicsCache> graphics_cache;
	FrameCache render_cache = FrameCache::with_reserve(32 * 1024);

	void refresh_visible_faces();
	void hide_adjacent_chunk_faces(glm::ivec3 delta, Chunk const &other);
	void on_render(Frame &frame, glm::ivec3 chunk_coord, TextureManager const &textures);
	void cache_graphics(Graphics &graphics);
};
