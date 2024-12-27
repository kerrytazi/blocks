#pragma once

#include "chunk.hpp"

#include <random>
#include <unordered_map>
#include <memory>

struct World
{
	std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>> chunks;

	void on_render(Frame &frame)
	{
		for (auto &chunk : chunks)
			chunk.second->on_render(frame, chunk.first);
	}

	void init_random_chunks(std::mt19937 &rng, glm::ivec3 from, glm::ivec3 to)
	{
		std::uniform_int_distribution<std::mt19937::result_type> dist(0, Chunk::EDGE_SIZE - 1);

		for (int x = from.x; x < to.x; ++x)
		{
			for (int y = from.y; y < to.y; ++y)
			{
				for (int z = from.z; z < to.z; ++z)
				{
					//chunks.insert(glm::ivec3{ x, y, z }, std::make_unique<Chunk>());
					auto &chunk = chunks.emplace(glm::ivec3{ x, y, z }, std::make_unique<Chunk>()).first->second;

					if (0)
					{
						for (int _x = 0; _x < Chunk::EDGE_SIZE; ++_x)
						{
							for (int _y = 0; _y < Chunk::EDGE_SIZE; ++_y)
							{
								for (int _z = 0; _z < Chunk::EDGE_SIZE; ++_z)
								{
									chunk->blocks[_x][_y][_z].solid = true;
								}
							}
						}
					}
					else
					{
						for (int i = 0; i < 400; ++i)
						{
							int _x = dist(rng);
							int _y = dist(rng);
							int _z = dist(rng);

							chunk->blocks[_x][_y][_z].solid = true;
						}
					}

					chunk->refresh_visible_faces();
				}
			}
		}

		for (auto &chunk : chunks)
		{
			static const glm::ivec3 directions[DIRECTION_MAX]{ {-1,0,0}, {1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}, {0,0,1} };

			for (auto direction : directions)
				if (auto it = chunks.find(chunk.first + direction); it != chunks.end())
					chunk.second->hide_adjacent_chunk_faces(direction, *it->second);
		}
	}
};
