#include "block.hpp"

void Cube::on_render(Frame &frame, ivec3 block_coord, MaterialManager const &materials)
{
	if (!solid)
		return;

	auto material = materials.find(id);
	vec3 p = block_coord;
	vec3 e = p + 1;

	if (visible_faces[(int)Direction::Left])
	{
		frame.add_quad(material,
			BlockVertex{ { p.x, p.y, p.z }, { 0.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, p.y, e.z }, { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, e.y, e.z }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, e.y, p.z }, { 0.0f, 0.0f }, { 0.0f, 0.0f } }
		);
	}

	if (visible_faces[(int)Direction::Right])
	{
		frame.add_quad(material,
			BlockVertex{ { e.x, p.y, p.z }, { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, p.z }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, e.z }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, p.y, e.z }, { 0.0f, 1.0f }, { 0.0f, 0.0f } }
		);
	}

	if (visible_faces[(int)Direction::Down])
	{
		frame.add_quad(material,
			BlockVertex{ { p.x, p.y, p.z }, { 0.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, p.y, p.z }, { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, p.y, e.z }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, p.y, e.z }, { 0.0f, 0.0f }, { 0.0f, 0.0f } }
		);
	}

	if (visible_faces[(int)Direction::Up])
	{
		frame.add_quad(material,
			BlockVertex{ { p.x, e.y, p.z }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, e.y, e.z }, { 0.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, e.z }, { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, p.z }, { 1.0f, 0.0f }, { 0.0f, 0.0f } }
		);
	}

	if (visible_faces[(int)Direction::Back])
	{
		frame.add_quad(material,
			BlockVertex{ { p.x, p.y, p.z }, { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, e.y, p.z }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, p.z }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, p.y, p.z }, { 0.0f, 1.0f }, { 0.0f, 0.0f } }
		);
	}

	if (visible_faces[(int)Direction::Front])
	{
		frame.add_quad(material,
			BlockVertex{ { p.x, p.y, e.z }, { 0.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, p.y, e.z }, { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, e.z }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, e.y, e.z }, { 0.0f, 0.0f }, { 0.0f, 0.0f } }
		);
	}
}