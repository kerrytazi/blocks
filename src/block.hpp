#pragma once

#include "gfxengine/frame.hpp"

#include <unordered_map>

class Graphics;

enum class ItemID : uint8_t
{
	Air,
	Dirt,
};

struct BlockVertex
{
	vec3 pos;
	vec3 normal;
	vec2 tex_coord;
	vec2 tex_offset;
};

struct MaterialManager
{
	std::unordered_map<ItemID, std::shared_ptr<Material>> materials;

	std::shared_ptr<Material> find(ItemID id) const
	{
		if (auto it = materials.find(id); it != materials.end())
			return it->second;

		return {};
	}
};

enum class Direction : uint8_t
{
	Left,        // -x (west)
	Right,       // +x (east)
	Down,        // -y
	Up,          // +y
	Back,        // -z (north)
	Front,       // +z (south)
};

static inline const size_t DIRECTION_MAX = 6;

struct RenderParams
{
	Frame &frame;
	MaterialManager const &materials;
	Graphics &graphics;
	ivec3 model_offset;

	RenderParams add_offset(ivec3 offset) const
	{
		RenderParams result(*this);
		result.model_offset += offset;
		return result;
	}
};

struct Cube
{
	ItemID id;
	bool solid;
	bool visible_faces[DIRECTION_MAX];

	void on_render(RenderParams const &params);
};
