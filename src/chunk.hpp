#pragma once

#include "block.hpp"
#include <vector>

struct Chunk
{
	static const size_t EDGE_SIZE = 16;
	static const size_t EDGE_LAST = EDGE_SIZE - 1;
	Cube blocks[EDGE_SIZE][EDGE_SIZE][EDGE_SIZE];

	FrameCache render_cache;

	void refresh_visible_faces()
	{
		for (int x = 0; x < EDGE_SIZE; ++x)
		{
			for (int y = 0; y < EDGE_SIZE; ++y)
			{
				for (int z = 0; z < EDGE_SIZE; ++z)
				{
					if (!blocks[x][y][z].solid)
					{
						for (int f = 0; f < DIRECTION_MAX; ++f)
							blocks[x][y][z].visible_faces[f] = false;

						continue;
					}

					// TODO
					for (int f = 0; f < DIRECTION_MAX; ++f)
						blocks[x][y][z].visible_faces[f] = true;

					if (x > 0           && blocks[x-1][y  ][z  ].solid)
						blocks[x][y][z].visible_faces[(int)Direction::Left] = false;
					if (x < EDGE_SIZE-1 && blocks[x+1][y  ][z  ].solid)
						blocks[x][y][z].visible_faces[(int)Direction::Right] = false;
					if (y > 0           && blocks[x  ][y-1][z  ].solid)
						blocks[x][y][z].visible_faces[(int)Direction::Down] = false;
					if (y < EDGE_SIZE-1 && blocks[x  ][y+1][z  ].solid)
						blocks[x][y][z].visible_faces[(int)Direction::Up] = false;
					if (z > 0           && blocks[x  ][y  ][z-1].solid)
						blocks[x][y][z].visible_faces[(int)Direction::Forward] = false;
					if (z < EDGE_SIZE-1 && blocks[x  ][y  ][z+1].solid)
						blocks[x][y][z].visible_faces[(int)Direction::Back] = false;
				}
			}
		}
	}

	void hide_adjacent_chunk_faces(glm::ivec3 delta, Chunk const &other)
	{
		if (delta.x < 0)
		{
			for (int y = 0; y < EDGE_SIZE; ++y)
				for (int z = 0; z < EDGE_SIZE; ++z)
					if (blocks[0][y][z].solid && other.blocks[EDGE_LAST][y][z].solid)
						blocks[0][y][z].visible_faces[(int)Direction::Left] = false;
		}
		else
		if (delta.x > 0)
		{
			for (int y = 0; y < EDGE_SIZE; ++y)
				for (int z = 0; z < EDGE_SIZE; ++z)
					if (blocks[EDGE_LAST][y][z].solid && other.blocks[0][y][z].solid)
						blocks[EDGE_LAST][y][z].visible_faces[(int)Direction::Right] = false;
		}
		else
		if (delta.y < 0)
		{
			for (int x = 0; x < EDGE_SIZE; ++x)
				for (int z = 0; z < EDGE_SIZE; ++z)
					if (blocks[x][0][z].solid && other.blocks[x][EDGE_LAST][z].solid)
						blocks[x][0][z].visible_faces[(int)Direction::Down] = false;
		}
		else
		if (delta.y > 0)
		{
			for (int x = 0; x < EDGE_SIZE; ++x)
				for (int z = 0; z < EDGE_SIZE; ++z)
					if (blocks[x][EDGE_LAST][z].solid && other.blocks[x][0][z].solid)
						blocks[x][EDGE_LAST][z].visible_faces[(int)Direction::Up] = false;
		}
		else
		if (delta.z < 0)
		{
			for (int x = 0; x < EDGE_SIZE; ++x)
				for (int y = 0; y < EDGE_SIZE; ++y)
					if (blocks[x][y][0].solid && other.blocks[x][y][EDGE_LAST].solid)
						blocks[x][y][0].visible_faces[(int)Direction::Forward] = false;
		}
		else
		if (delta.z > 0)
		{
			for (int x = 0; x < EDGE_SIZE; ++x)
				for (int y = 0; y < EDGE_SIZE; ++y)
					if (blocks[x][y][EDGE_LAST].solid && other.blocks[x][y][0].solid)
						blocks[x][y][EDGE_LAST].visible_faces[(int)Direction::Back] = false;
		}
	}

	void on_render(Frame &frame, glm::ivec3 chunk_coord, TextureManager const &textures)
	{
		if (!render_cache.empty())
		{
			frame.add_cached(render_cache, textures.find(ItemID::Dirt));
		}
		else
		{
			render_cache = frame.cached([&]() {
				for (int x = 0; x < EDGE_SIZE; ++x)
					for (int y = 0; y < EDGE_SIZE; ++y)
						for (int z = 0; z < EDGE_SIZE; ++z)
							blocks[x][y][z].on_render(frame, chunk_coord * (int)Chunk::EDGE_SIZE + glm::ivec3{ x, y, z }, textures);
			});
		}
	}
};
