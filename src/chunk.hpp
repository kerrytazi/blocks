#pragma once

#include "gfxengine/graphics.hpp"

#include "block.hpp"
#include <vector>

struct Chunk
{
	static const size_t EDGE_SIZE = 16;
	static const size_t EDGE_LAST = EDGE_SIZE - 1;
	Cube blocks[EDGE_SIZE][EDGE_SIZE][EDGE_SIZE]{};

	bool dirty = false;
	FrameCacheVertices render_cache;
	std::shared_ptr<GraphicsCacheVertices> render_cache_gpu;

	void refresh_visible_faces();
	void hide_adjacent_chunk_faces(ivec3 delta, Chunk const &other);
	void on_render(RenderParams const &params);
	void on_render_no_cache(RenderParams const &params);
};
