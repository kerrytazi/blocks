#include "block.hpp"

void Cube::on_render(RenderParams const &params)
{
	if (!solid)
		return;

	auto material = params.materials.find(id);
	vec3 p = params.model_offset;
	vec3 e = p + 1;

	if (visible_faces[(int)Direction::Left])
	{
		params.frame.add_quad(material,
			BlockVertex{ { p.x, p.y, p.z }, -vec3::unit_x(), { 0.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, p.y, e.z }, -vec3::unit_x(), { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, e.y, e.z }, -vec3::unit_x(), { 1.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, e.y, p.z }, -vec3::unit_x(), { 0.0f, 0.0f }, { 0.0f, 0.0f } }
		);
	}

	if (visible_faces[(int)Direction::Right])
	{
		params.frame.add_quad(material,
			BlockVertex{ { e.x, p.y, p.z },  vec3::unit_x(), { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, p.z },  vec3::unit_x(), { 1.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, e.z },  vec3::unit_x(), { 0.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, p.y, e.z },  vec3::unit_x(), { 0.0f, 1.0f }, { 0.0f, 0.0f } }
		);
	}

	if (visible_faces[(int)Direction::Down])
	{
		params.frame.add_quad(material,
			BlockVertex{ { p.x, p.y, p.z }, -vec3::unit_y(), { 0.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, p.y, p.z }, -vec3::unit_y(), { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, p.y, e.z }, -vec3::unit_y(), { 1.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, p.y, e.z }, -vec3::unit_y(), { 0.0f, 0.0f }, { 0.0f, 0.0f } }
		);
	}

	if (visible_faces[(int)Direction::Up])
	{
		params.frame.add_quad(material,
			BlockVertex{ { p.x, e.y, p.z },  vec3::unit_y(), { 0.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, e.y, e.z },  vec3::unit_y(), { 0.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, e.z },  vec3::unit_y(), { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, p.z },  vec3::unit_y(), { 1.0f, 0.0f }, { 0.0f, 0.0f } }
		);
	}

	if (visible_faces[(int)Direction::Back])
	{
		params.frame.add_quad(material,
			BlockVertex{ { p.x, p.y, p.z }, -vec3::unit_z(), { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, e.y, p.z }, -vec3::unit_z(), { 1.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, p.z }, -vec3::unit_z(), { 0.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, p.y, p.z }, -vec3::unit_z(), { 0.0f, 1.0f }, { 0.0f, 0.0f } }
		);
	}

	if (visible_faces[(int)Direction::Front])
	{
		params.frame.add_quad(material,
			BlockVertex{ { p.x, p.y, e.z },  vec3::unit_z(), { 0.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, p.y, e.z },  vec3::unit_z(), { 1.0f, 1.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { e.x, e.y, e.z },  vec3::unit_z(), { 1.0f, 0.0f }, { 0.0f, 0.0f } },
			BlockVertex{ { p.x, e.y, e.z },  vec3::unit_z(), { 0.0f, 0.0f }, { 0.0f, 0.0f } }
		);
	}
}