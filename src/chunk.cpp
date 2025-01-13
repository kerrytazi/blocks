#include "chunk.hpp"

void Chunk::refresh_visible_faces()
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
					blocks[x][y][z].visible_faces[(int)Direction::Back] = false;
				if (z < EDGE_SIZE-1 && blocks[x  ][y  ][z+1].solid)
					blocks[x][y][z].visible_faces[(int)Direction::Front] = false;
			}
		}
	}
}

void Chunk::hide_adjacent_chunk_faces(ivec3 delta, Chunk const &other)
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
					blocks[x][y][0].visible_faces[(int)Direction::Back] = false;
	}
	else
	if (delta.z > 0)
	{
		for (int x = 0; x < EDGE_SIZE; ++x)
			for (int y = 0; y < EDGE_SIZE; ++y)
				if (blocks[x][y][EDGE_LAST].solid && other.blocks[x][y][0].solid)
					blocks[x][y][EDGE_LAST].visible_faces[(int)Direction::Front] = false;
	}
}

void Chunk::on_render(Frame &frame, ivec3 chunk_coord, MaterialManager const &materials)
{
	bool greedy_meshing = true;
	if (greedy_meshing)
	{
		// Left
		for (int x = 0; x < EDGE_SIZE; ++x)
		{
			bool meshed[EDGE_SIZE][EDGE_SIZE]{};

			for (int z = 0; z < EDGE_SIZE; ++z)
			{
				for (int y = 0; y < EDGE_SIZE; ++y)
				{
					if (!blocks[x][y][z].visible_faces[(int)Direction::Left] || meshed[z][y])
						continue;

					ItemID id = blocks[x][y][z].id;

					int ez = z + 1;

					for (int z2 = z + 1; z2 < EDGE_SIZE; ++z2)
					{
						if (!blocks[x][y][z2].visible_faces[(int)Direction::Left] || meshed[z2][y])
							break;

						ez++;
					}

					int ey = y + 1;

					for (int y2 = y + 1; y2 < EDGE_SIZE; ++y2)
					{
						for (int z2 = z; z2 < ez; ++z2)
							if (!blocks[x][y2][z2].visible_faces[(int)Direction::Left] || meshed[z2][y2])
								goto _outer_left;

						ey++;
					}
					_outer_left:{}

					for (int z2 = z; z2 < ez; ++z2)
						for (int y2 = y; y2 < ey; ++y2)
							meshed[z2][y2] = true;

					{
						ivec3 p = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ x, y, z };
						ivec3 e = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ x, ey, ez };
						vec2 s = { ez - z, ey - y };
						vec2 c = { 0.0f, 0.0f };

						frame.add_quad(materials.find(id),
							BlockVertex{ { p.x, p.y, p.z }, { c.x, s.y }, { 0.0f, 0.0f } },
							BlockVertex{ { p.x, p.y, e.z }, { s.x, s.y }, { 0.0f, 0.0f } },
							BlockVertex{ { p.x, e.y, e.z }, { s.x, c.y }, { 0.0f, 0.0f } },
							BlockVertex{ { p.x, e.y, p.z }, { c.x, c.y }, { 0.0f, 0.0f } }
						);
					}
				}
			}
		}

		// Right
		for (int x = 0; x < EDGE_SIZE; ++x)
		{
			bool meshed[EDGE_SIZE][EDGE_SIZE]{};

			for (int z = 0; z < EDGE_SIZE; ++z)
			{
				for (int y = 0; y < EDGE_SIZE; ++y)
				{
					if (!blocks[x][y][z].visible_faces[(int)Direction::Right] || meshed[z][y])
						continue;

					ItemID id = blocks[x][y][z].id;

					int ez = z + 1;

					for (int z2 = z + 1; z2 < EDGE_SIZE; ++z2)
					{
						if (!blocks[x][y][z2].visible_faces[(int)Direction::Right] || meshed[z2][y])
							break;

						ez++;
					}

					int ey = y + 1;

					for (int y2 = y + 1; y2 < EDGE_SIZE; ++y2)
					{
						for (int z2 = z; z2 < ez; ++z2)
							if (!blocks[x][y2][z2].visible_faces[(int)Direction::Right] || meshed[z2][y2])
								goto _outer_right;

						ey++;
					}
					_outer_right:{}

					for (int z2 = z; z2 < ez; ++z2)
						for (int y2 = y; y2 < ey; ++y2)
							meshed[z2][y2] = true;

					{
						ivec3 p = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ x+1, y, z };
						ivec3 e = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ x+1, ey, ez };
						vec2 s = { ez - z, ey - y };
						vec2 c = { 0.0f, 0.0f };

						frame.add_quad(materials.find(id),
							BlockVertex{ { e.x, p.y, p.z }, { s.x, s.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, e.y, p.z }, { s.x, c.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, e.y, e.z }, { c.x, c.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, p.y, e.z }, { c.x, s.y }, { 0.0f, 0.0f } }
						);
					}
				}
			}
		}

		// Down
		for (int y = 0; y < EDGE_SIZE; ++y)
		{
			bool meshed[EDGE_SIZE][EDGE_SIZE]{};

			for (int x = 0; x < EDGE_SIZE; ++x)
			{
				for (int z = 0; z < EDGE_SIZE; ++z)
				{
					if (!blocks[x][y][z].visible_faces[(int)Direction::Down] || meshed[x][z])
						continue;

					ItemID id = blocks[x][y][z].id;

					int ex = x + 1;

					for (int x2 = x + 1; x2 < EDGE_SIZE; ++x2)
					{
						if (!blocks[x2][y][z].visible_faces[(int)Direction::Down] || meshed[x2][z])
							break;

						ex++;
					}

					int ez = z + 1;

					for (int z2 = z + 1; z2 < EDGE_SIZE; ++z2)
					{
						for (int x2 = x; x2 < ex; ++x2)
							if (!blocks[x2][y][z2].visible_faces[(int)Direction::Down] || meshed[x2][z2])
								goto _outer_down;

						ez++;
					}
					_outer_down:{}

					for (int x2 = x; x2 < ex; ++x2)
						for (int z2 = z; z2 < ez; ++z2)
							meshed[x2][z2] = true;

					{
						ivec3 p = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ x, y, z };
						ivec3 e = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ ex, y, ez };
						vec2 s = { ex - x, ez - z };
						vec2 c = { 0.0f, 0.0f };

						frame.add_quad(materials.find(id),
							BlockVertex{ { p.x, p.y, p.z }, { c.x, s.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, p.y, p.z }, { s.x, s.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, p.y, e.z }, { s.x, c.y }, { 0.0f, 0.0f } },
							BlockVertex{ { p.x, p.y, e.z }, { c.x, c.y }, { 0.0f, 0.0f } }
						);
					}
				}
			}
		}

		// Up
		for (int y = 0; y < EDGE_SIZE; ++y)
		{
			bool meshed[EDGE_SIZE][EDGE_SIZE]{};

			for (int x = 0; x < EDGE_SIZE; ++x)
			{
				for (int z = 0; z < EDGE_SIZE; ++z)
				{
					if (!blocks[x][y][z].visible_faces[(int)Direction::Up] || meshed[x][z])
						continue;

					ItemID id = blocks[x][y][z].id;

					int ex = x + 1;

					for (int x2 = x + 1; x2 < EDGE_SIZE; ++x2)
					{
						if (!blocks[x2][y][z].visible_faces[(int)Direction::Up] || meshed[x2][z])
							break;

						ex++;
					}

					int ez = z + 1;

					for (int z2 = z + 1; z2 < EDGE_SIZE; ++z2)
					{
						for (int x2 = x; x2 < ex; ++x2)
							if (!blocks[x2][y][z2].visible_faces[(int)Direction::Up] || meshed[x2][z2])
								goto _outer_up;

						ez++;
					}
					_outer_up:{}

					for (int x2 = x; x2 < ex; ++x2)
						for (int z2 = z; z2 < ez; ++z2)
							meshed[x2][z2] = true;

					{
						ivec3 p = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ x, y+1, z };
						ivec3 e = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ ex, y+1, ez };
						vec2 s = { ex - x, ez - z };
						vec2 c = { 0.0f, 0.0f };

						frame.add_quad(materials.find(id),
							BlockVertex{ { p.x, e.y, p.z }, { c.x, c.y }, { 0.0f, 0.0f } },
							BlockVertex{ { p.x, e.y, e.z }, { c.x, s.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, e.y, e.z }, { s.x, s.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, e.y, p.z }, { s.x, c.y }, { 0.0f, 0.0f } }
						);
					}
				}
			}
		}

		// Back
		for (int z = 0; z < EDGE_SIZE; ++z)
		{
			bool meshed[EDGE_SIZE][EDGE_SIZE]{};

			for (int x = 0; x < EDGE_SIZE; ++x)
			{
				for (int y = 0; y < EDGE_SIZE; ++y)
				{
					if (!blocks[x][y][z].visible_faces[(int)Direction::Back] || meshed[x][y])
						continue;

					ItemID id = blocks[x][y][z].id;

					int ex = x + 1;

					for (int x2 = x + 1; x2 < EDGE_SIZE; ++x2)
					{
						if (!blocks[x2][y][z].visible_faces[(int)Direction::Back] || meshed[x2][y])
							break;

						ex++;
					}

					int ey = y + 1;

					for (int y2 = y + 1; y2 < EDGE_SIZE; ++y2)
					{
						for (int x2 = x; x2 < ex; ++x2)
							if (!blocks[x2][y2][z].visible_faces[(int)Direction::Back] || meshed[x2][y2])
								goto _outer_back;

						ey++;
					}
					_outer_back:{}

					for (int x2 = x; x2 < ex; ++x2)
						for (int y2 = y; y2 < ey; ++y2)
							meshed[x2][y2] = true;

					{
						ivec3 p = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ x, y, z };
						ivec3 e = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ ex, ey, z };
						vec2 s = { ex - x, ey - y };
						vec2 c = { 0.0f, 0.0f };

						frame.add_quad(materials.find(id),
							BlockVertex{ { p.x, p.y, p.z }, { s.x, s.y }, { 0.0f, 0.0f } },
							BlockVertex{ { p.x, e.y, p.z }, { s.x, c.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, e.y, p.z }, { c.x, c.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, p.y, p.z }, { c.x, s.y }, { 0.0f, 0.0f } }
						);
					}
				}
			}
		}

		// Front
		for (int z = 0; z < EDGE_SIZE; ++z)
		{
			bool meshed[EDGE_SIZE][EDGE_SIZE]{};

			for (int x = 0; x < EDGE_SIZE; ++x)
			{
				for (int y = 0; y < EDGE_SIZE; ++y)
				{
					if (!blocks[x][y][z].visible_faces[(int)Direction::Front] || meshed[x][y])
						continue;

					ItemID id = blocks[x][y][z].id;

					int ex = x + 1;

					for (int x2 = x + 1; x2 < EDGE_SIZE; ++x2)
					{
						if (!blocks[x2][y][z].visible_faces[(int)Direction::Front] || meshed[x2][y])
							break;

						ex++;
					}

					int ey = y + 1;

					for (int y2 = y + 1; y2 < EDGE_SIZE; ++y2)
					{
						for (int x2 = x; x2 < ex; ++x2)
							if (!blocks[x2][y2][z].visible_faces[(int)Direction::Front] || meshed[x2][y2])
								goto _outer_forward;

						ey++;
					}
					_outer_forward:{}

					for (int x2 = x; x2 < ex; ++x2)
						for (int y2 = y; y2 < ey; ++y2)
							meshed[x2][y2] = true;

					{
						ivec3 p = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ x, y, z+1 };
						ivec3 e = chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ ex, ey, z+1 };
						vec2 s = { ex - x, ey - y };
						vec2 c = { 0.0f, 0.0f };

						frame.add_quad(materials.find(id),
							BlockVertex{ { p.x, p.y, e.z }, { c.x, s.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, p.y, e.z }, { s.x, s.y }, { 0.0f, 0.0f } },
							BlockVertex{ { e.x, e.y, e.z }, { s.x, c.y }, { 0.0f, 0.0f } },
							BlockVertex{ { p.x, e.y, e.z }, { c.x, c.y }, { 0.0f, 0.0f } }
						);
					}
				}
			}
		}
	}
	else
	{
		for (int x = 0; x < EDGE_SIZE; ++x)
			for (int y = 0; y < EDGE_SIZE; ++y)
				for (int z = 0; z < EDGE_SIZE; ++z)
					blocks[x][y][z].on_render(frame, chunk_coord * (int)Chunk::EDGE_SIZE + ivec3{ x, y, z }, materials);
	}
}
