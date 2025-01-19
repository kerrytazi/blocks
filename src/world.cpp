#include "world.hpp"

#include "gfxengine/noise_generator.hpp"

void World::on_render(RenderParams const &params)
{
	for (auto &chunk : chunks)
		chunk.second->on_render(params.add_offset(chunk.first * Chunk::EDGE_SIZE));
}

void World::init_random_chunks(std::mt19937 &rng, ivec3 from, ivec3 to, int count)
{
	chunks.clear();
	std::uniform_int_distribution<std::mt19937::result_type> dist(0, Chunk::EDGE_SIZE - 1);

	for (int x = from.x; x < to.x; ++x)
	{
		for (int y = from.y; y < to.y; ++y)
		{
			for (int z = from.z; z < to.z; ++z)
			{
				auto &chunk = chunks.emplace(ivec3{ x, y, z }, std::make_unique<Chunk>()).first->second;

				if (0)
				{
					for (int _x = 0; _x < Chunk::EDGE_SIZE; ++_x)
					{
						for (int _y = 0; _y < Chunk::EDGE_SIZE; ++_y)
						{
							for (int _z = 0; _z < Chunk::EDGE_SIZE; ++_z)
							{
								chunk->blocks[_x][_y][_z].solid = true;
								chunk->blocks[_x][_y][_z].id = ItemID::Dirt;
							}
						}
					}
				}
				else
				{
					for (int i = 0; i < count; ++i)
					{
						int _x = dist(rng);
						int _y = dist(rng);
						int _z = dist(rng);

						chunk->blocks[_x][_y][_z].solid = true;
						chunk->blocks[_x][_y][_z].id = ItemID::Dirt;
					}
				}

				chunk->refresh_visible_faces();
			}
		}
	}

	for (auto &chunk : chunks)
	{
		static const ivec3 directions[DIRECTION_MAX]{ {-1,0,0}, {1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}, {0,0,1} };

		for (auto direction : directions)
			if (auto it = chunks.find(chunk.first + direction); it != chunks.end())
				chunk.second->hide_adjacent_chunk_faces(direction, *it->second);
	}
}

void World::init(NoiseGenerator const &gen, ivec3 from, ivec3 to)
{
	chunks.clear();

	for (int x = from.x; x < to.x; ++x)
	{
		for (int z = from.x; z < to.z; ++z)
		{
			int y = 0;
			auto &chunk = chunks.emplace(ivec3{ x, y, z }, std::make_unique<Chunk>()).first->second;

			for (int _x = 0; _x < Chunk::EDGE_SIZE; ++_x)
			{
				for (int _z = 0; _z < Chunk::EDGE_SIZE; ++_z)
				{
					double val = (
						(gen.noise((x * (int)Chunk::EDGE_SIZE + _x) / 64.0, (z * (int)Chunk::EDGE_SIZE + _z) / 64.0) + 1) / 2 * 1.00 +
						(gen.noise((x * (int)Chunk::EDGE_SIZE + _x) / 32.0, (z * (int)Chunk::EDGE_SIZE + _z) / 32.0) + 1) / 2 * 0.50 +
						(gen.noise((x * (int)Chunk::EDGE_SIZE + _x) / 16.0, (z * (int)Chunk::EDGE_SIZE + _z) / 16.0) + 1) / 2 * 0.25
						) / 1.75;
					int height = val * 15 + 1;

					for (int _y = math::max(0, height - 5); _y < height; ++_y)
					{
						chunk->blocks[_x][_y][_z].solid = true;
						chunk->blocks[_x][_y][_z].id = ItemID::Dirt;
					}
				}
			}

			chunk->refresh_visible_faces();
		}
	}

	for (auto &chunk : chunks)
	{
		static const ivec3 directions[DIRECTION_MAX]{ {-1,0,0}, {1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}, {0,0,1} };

		for (auto direction : directions)
			if (auto it = chunks.find(chunk.first + direction); it != chunks.end())
				chunk.second->hide_adjacent_chunk_faces(direction, *it->second);
	}
}
