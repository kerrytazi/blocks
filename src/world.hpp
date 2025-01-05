#pragma once

#include "chunk.hpp"

#include <random>
#include <unordered_map>
#include <memory>

class NoiseGenerator;

struct World
{
	std::unordered_map<ivec3, std::unique_ptr<Chunk>> chunks;

	void on_render(Frame &frame, TextureManager const &textures);
	void cache_graphics(Graphics &graphics);
	void init_random_chunks(std::mt19937 &rng, ivec3 from, ivec3 to, int count);
	void init(NoiseGenerator const &gen, ivec3 from, ivec3 to);
};
