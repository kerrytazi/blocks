#pragma once

#include "gfxengine/frame.hpp"

#include <unordered_map>

enum class ItemID : uint8_t
{
	Air,
	Dirt,
};

struct TextureManager
{
	std::unordered_map<ItemID, std::shared_ptr<Image>> textures;

	std::shared_ptr<Image> find(ItemID id) const
	{
		if (auto it = textures.find(id); it != textures.end())
			return it->second;

		return {};
	}

	void load(ItemID id, std::string_view file_name)
	{
		textures.insert({ id, std::make_shared<Image>(Image::load_sync(file_name)) });
	}
};

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
	ItemID id;
	bool solid;
	bool visible_faces[DIRECTION_MAX];

	void on_render(Frame &frame, glm::ivec3 block_coord, TextureManager const &textures)
	{
		if (!solid)
			return;

		glm::ivec3 p = block_coord;

		if (visible_faces[(int)Direction::Left])
			frame.add_quad({ p.x    , p.y    , p.z     }, { p.x    , p.y    , p.z + 1 }, { p.x    , p.y + 1, p.z + 1 }, { p.x    , p.y + 1, p.z     }, Color::RED, textures.find(id), { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f });     // Left
		if (visible_faces[(int)Direction::Right])
			frame.add_quad({ p.x + 1, p.y    , p.z     }, { p.x + 1, p.y + 1, p.z     }, { p.x + 1, p.y + 1, p.z + 1 }, { p.x + 1, p.y    , p.z + 1 }, Color::YELLOW, textures.find(id), { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f });  // Right
		if (visible_faces[(int)Direction::Down])
			frame.add_quad({ p.x    , p.y    , p.z     }, { p.x + 1, p.y    , p.z     }, { p.x + 1, p.y    , p.z + 1 }, { p.x    , p.y    , p.z + 1 }, Color::BLUE, textures.find(id), { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f });    // Down
		if (visible_faces[(int)Direction::Up])
			frame.add_quad({ p.x    , p.y + 1, p.z     }, { p.x    , p.y + 1, p.z + 1 }, { p.x + 1, p.y + 1, p.z + 1 }, { p.x + 1, p.y + 1, p.z     }, Color::MAGENTA, textures.find(id), { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }); // Up
		if (visible_faces[(int)Direction::Forward])
			frame.add_quad({ p.x    , p.y    , p.z     }, { p.x    , p.y + 1, p.z     }, { p.x + 1, p.y + 1, p.z     }, { p.x + 1, p.y    , p.z     }, Color::GREEN, textures.find(id), { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f });   // Forward
		if (visible_faces[(int)Direction::Back])
			frame.add_quad({ p.x    , p.y    , p.z + 1 }, { p.x + 1, p.y    , p.z + 1 }, { p.x + 1, p.y + 1, p.z + 1 }, { p.x    , p.y + 1, p.z + 1 }, Color::CYAN, textures.find(id), { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f });    // Back
	}
};
