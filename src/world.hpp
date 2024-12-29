#pragma once

#include "chunk.hpp"

#include <random>
#include <unordered_map>
#include <memory>

struct World
{
	std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>> chunks;

	void on_render(Frame &frame, TextureManager const &textures);
	void cache_graphics(Graphics &graphics);
	void init_random_chunks(std::mt19937 &rng, glm::ivec3 from, glm::ivec3 to, int count);
};
