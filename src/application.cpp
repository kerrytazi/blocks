#include "world.hpp"

#include "gfxengine/material.hpp"
#include "gfxengine/input_controller.hpp"
#include "gfxengine/platform.hpp"
#include "gfxengine/window.hpp"
#include "gfxengine/graphics.hpp"
#include "gfxengine/frame.hpp"
#include "gfxengine/window_event_handler.hpp"
#include "gfxengine/noise_generator.hpp"
#include "gfxengine/logger.hpp"

#include <random>

#if GFXENGINE_EDITOR
#include "imgui.h"
#endif // GFXENGINE_EDITOR

class BlocksApplication : public WindowEventHandler
{
private:

	Platform &platform;
	Logger logger;

	std::unique_ptr<Window> window;

	int fps_limit = 120;

	bool should_close = false;
	bool mouse_locked = false;
	bool fullscreen_enabled = false;
	bool speed_up = false;

	ivec2 render_size{};
	float render_scale = 1.0f;

	float camera_fov = 90.0f;

	InputController input_controller;

	int world_radius = 2;
	int random_seed = 1337;
	int random_count = 400;
	std::mt19937 rng{ (uint32_t)random_seed };
	std::unique_ptr<NoiseGenerator> world_gen = std::make_unique<NoiseGenerator>(random_seed);

	World world;
	MaterialManager materials;

#if GFXENGINE_EDITOR
	bool editor_demo_window = false;
	bool editor_settings = true;
	bool editors = true;
	bool editor_coord = true;
	bool editor_stats = true;

	bool editor_gfx_wireframe = false;
	bool editor_gfx_culling = true;
	bool editor_gfx_depth = true;

	bool editor_window_vsync = false;
	int editor_windows_limit = fps_limit;
#endif // GFXENGINE_EDITOR

	void on_update()
	{
		const double time = platform.get_time();
		input_controller.update_all(time);
	}

	void on_render(Frame &frame)
	{
		frame.setting_wireframe(editor_gfx_wireframe);
		frame.setting_culling(editor_gfx_culling);
		frame.setting_depth(editor_gfx_depth);

		{
			vec3 camera_pos = input_controller.position;
			vec3 camera_front = input_controller.calc_front_direction();

			const mat4 proj = math::perspective(math::deg_to_rad(camera_fov), float(render_size.x) / float(render_size.y), 0.01f, 1000.0f);
			const mat4 view = math::look_at(camera_pos, camera_pos + camera_front, vec3::unit_y());
			const mat4 model = mat4::identity();
			const mat4 mvp = proj * view * model;
			materials.find(ItemID::Dirt)->uniforms[0] = mvp;
			materials.find(ItemID::Dirt)->uniforms[3] = camera_pos;

			frame.clear_background(ColorF(0.2f, 0.3f, 0.2f, 1.0f));

			RenderParams params{
				.frame = frame,
				.materials = materials,
				.graphics = window->get_graphics(),
				.model_offset = ivec3{},
			};
			world.on_render(params);
		}

#if GFXENGINE_EDITOR
		frame.on_draw_editor([this, &frame]() {

			ImVec2 _pos{ 5, 5 };

			if (editors)
			{
				ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);

				ImGuiIO &io = ImGui::GetIO();
				ImGui::Text("fps %6.1f   ms %6.3f", io.Framerate, 1000.0f / io.Framerate);

				ImGui::SetWindowPos(_pos, ImGuiCond_Always);
				_pos.y += ImGui::GetWindowSize().y + 5;
				ImGui::End();
			}

			if (editor_coord)
			{
				ImGui::Begin("Coords", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);

				vec3 pos = input_controller.position;
				ivec3 chunk = math::floor(pos / 16.0f);
				vec3 lpos = pos - vec3(chunk) * 16.0f;
				vec2 rot = input_controller.rotation;

				float angle = math::fmod(-rot.y + 22.5f, 360.0f);
				if (angle < 0.0f)
					angle += 360.0f;

				char const *dir = nullptr;

				if      (angle < 45.0f * 1) dir = " E +x  ";
				else if (angle < 45.0f * 2) dir = "NE +x-z";
				else if (angle < 45.0f * 3) dir = "N    -z";
				else if (angle < 45.0f * 4) dir = "NW -x-z";
				else if (angle < 45.0f * 5) dir = " W -x  ";
				else if (angle < 45.0f * 6) dir = "SW -x+z";
				else if (angle < 45.0f * 7) dir = "S    +z";
				else if (angle < 45.0f * 8) dir = "SE +x+z";

				ImGui::Text("pos   %6.2f %6.2f %6.2f", pos.x, pos.y, pos. z);
				ImGui::Text("chunk %3d    %3d    %3d", chunk.x, chunk.y, chunk.z);
				ImGui::Text("lpos  %6.2f %6.2f %6.2f", lpos.x, lpos.y, lpos.z);
				ImGui::Text("rot   %6.2f %6.2f", rot.x, rot.y);
				ImGui::Text("dir   %s", dir);

				ImGui::SetWindowPos(_pos, ImGuiCond_Always);
				_pos.y += ImGui::GetWindowSize().y + 5;
				ImGui::End();
			}

			if (editor_stats)
			{
				auto stats = frame.get_stats();

				ImGui::Begin("Frame Stats", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);

				ImGui::Text("draw calls:      %7d", (int)stats.draw_calls);

				ImGui::Text("vertices:        %7d", (int)stats.vertices);
				ImGui::Text("indices:         %7d", (int)stats.indices);
				ImGui::Text("triangles:       %7d", (int)stats.indices / 3);

				ImGui::Text("cache vertices:  %7d", (int)stats.cache_vertices);
				ImGui::Text("cache indices:   %7d", (int)stats.cache_indices);
				ImGui::Text("cache triangles: %7d", (int)stats.cache_indices / 3);

				ImGui::SetWindowPos(_pos, ImGuiCond_Always);
				_pos.y += ImGui::GetWindowSize().y + 5;
				ImGui::End();
			}

			if (editor_settings)
			{
				ImGui::Begin("Settings", &editor_settings, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

				ImGui::Checkbox("FPS", &editors);
				ImGui::Checkbox("Coords", &editor_coord);
				ImGui::Checkbox("Stats", &editor_stats);
				ImGui::Checkbox("Speed Up", &speed_up);
				ImGui::Checkbox("ImGui Demo", &editor_demo_window);

				ImGui::PushItemWidth(126);
				ImGui::InputFloat("FOV", &camera_fov, 5.0f);
				ImGui::PopItemWidth();
				ImGui::PushItemWidth(80);
				ImGui::InputInt("Random Seed", &random_seed, 0);
				ImGui::InputInt("World Radius", &world_radius, 0);
				ImGui::InputInt("Blocks per Chunk", &random_count, 0);
				ImGui::PopItemWidth();

				if (ImGui::Button("Generate Random Chunks"))
				{
					if (random_seed == -1)
						rng.seed(rng());
					else
						rng.seed(random_seed);

					//world.init_random_chunks(rng, {-world_radius,-world_radius,-world_radius}, {world_radius,world_radius,world_radius}, random_count);
					world.init(*world_gen, {-world_radius,-world_radius,-world_radius}, {world_radius,world_radius,world_radius});
				}

				if (ImGui::CollapsingHeader("Graphics"))
				{
					ImGui::Checkbox("Wireframe", &editor_gfx_wireframe);
					ImGui::Checkbox("Culling", &editor_gfx_culling);
					ImGui::Checkbox("Depth Test", &editor_gfx_depth);

					ImGui::PushItemWidth(80);
					ImGui::InputFloat("Render Scale", &render_scale);
					if (ImGui::IsItemDeactivatedAfterEdit())
						window->get_graphics().resize(render_size, render_scale);
					ImGui::PopItemWidth();
				}

				if (ImGui::CollapsingHeader("Window"))
				{
					if (ImGui::Checkbox("vsync", &editor_window_vsync))
					{
						window->set_vsync(editor_window_vsync);
					}

					if (ImGui::Checkbox("fullscreen", &fullscreen_enabled))
					{
						window->fullscreen(fullscreen_enabled);
					}

					ImGui::PushItemWidth(80);
					ImGui::InputInt("FPS Limit", &editor_windows_limit, 0);
					if (ImGui::IsItemDeactivatedAfterEdit())
						fps_limit = editor_windows_limit;
					ImGui::PopItemWidth();
				}

				ImGui::SetWindowPos(_pos, ImGuiCond_Always);
				_pos.y += ImGui::GetWindowSize().y + 5;
				ImGui::End();
			}

			if (editor_demo_window)
				ImGui::ShowDemoWindow(&editor_demo_window);
		});
#endif // GFXENGINE_EDITOR
	}

	virtual void on_keyboard_event(double time, KeyboardEvent event) override
	{
		if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::Q)
			window->close();

		if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::Z)
		{
			speed_up = !speed_up;
			input_controller.change_speed(time, speed_up ? 64.0f : 16.0f);
		}

#if GFXENGINE_EDITOR
		if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::F11)
			editor_demo_window = !editor_demo_window;

		if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::F2)
			editor_settings = !editor_settings;
#endif // GFXENGINE_EDITOR

		if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::F)
		{
			mouse_locked = !mouse_locked;
			window->lock_mouse(mouse_locked);

			if (!mouse_locked)
				input_controller.stop_all(time);
		}

		if (mouse_locked && event.locked)
		{
			struct KeyDir
			{
				KeyboardEvent::Key key;
				InputController::Direction direction;
			};

			KeyDir key_dirs[] {
				{ KeyboardEvent::Key::W, InputController::Direction::Front },
				{ KeyboardEvent::Key::S, InputController::Direction::Back },
				{ KeyboardEvent::Key::D, InputController::Direction::Right },
				{ KeyboardEvent::Key::A, InputController::Direction::Left },
				{ KeyboardEvent::Key::Space, InputController::Direction::Up },
				{ KeyboardEvent::Key::LeftShift, InputController::Direction::Down },
			};

			for (auto key_dir : key_dirs)
			{
				if (event.type == KeyboardEvent::Type::Press && event.key == key_dir.key)
					input_controller.begin_move(time, key_dir.direction);
				if (event.type == KeyboardEvent::Type::Release && event.key == key_dir.key)
					input_controller.end_move(time, key_dir.direction);
			}
		}
	}

	virtual void on_mouse_event(double time, MouseEvent event) override
	{
		if (mouse_locked && event.locked && event.type == MouseEvent::Type::Move)
		{
			vec2 rotation = event.pos;
			input_controller.rotate_view(time, rotation._yx());
		}

		if (mouse_locked && event.locked && event.type == MouseEvent::Type::Scroll)
		{
			camera_fov -= event.scroll.y * 4.0f;

			if (camera_fov < 0.125f)
				camera_fov = 0.125f;
			if (camera_fov > 120.0f)
				camera_fov = 120.0f;

			input_controller.change_rotation_sensitivity(time, 0.1f * camera_fov / 90.0f);
		}

		if (mouse_locked && event.locked && event.type == MouseEvent::Type::Press && event.key == MouseEvent::Key::MMB)
		{
			camera_fov = 90.0f;
			input_controller.change_rotation_sensitivity(time, 0.1f * camera_fov / 90.0f);
		}
	}

	virtual void on_mouse_external_unlock(double time, MouseExternalUnlockEvent event) override
	{
		mouse_locked = false;
		input_controller.stop_all(time);
	}

	virtual void on_resize(double time, ResizeEvent event) override
	{
		render_size = event.new_size;
		window->get_graphics().resize(render_size, render_scale);
	}

	virtual void on_close_event(double time, CloseEvent event) override
	{
		should_close = true;
	}

public:

	BlocksApplication(Platform &platform)
		: platform{ platform }
		, logger{ platform }
	{
		logger.add_handler([this](char const *c_str, size_t len) {
			this->platform.debug_log(c_str, len);
		});

		CreateWindowParams params{ platform };
		params.window_event_handler = this;
		window = platform.create_window(params);

		input_controller.position = { 0.0f, 40.0f, 0.0f };
		input_controller.rotation = { -45.0f, 0.0f };
		input_controller.change_speed(0.0, 16.0f);
		input_controller.change_rotation_sensitivity(0.0, 0.1f);

		{
			auto params = CreateMaterialParams{};

			params.attributes.add(ShaderFieldInfo{ "pos", ShaderFieldType::F32, false, 3 });
			params.attributes.add(ShaderFieldInfo{ "normal", ShaderFieldType::F32, false, 3 });
			params.attributes.add(ShaderFieldInfo{ "tex_coord", ShaderFieldType::F32, false, 2 });
			params.attributes.add(ShaderFieldInfo{ "tex_offset", ShaderFieldType::F32, false, 2 });

			params.uniforms.add(ShaderFieldInfo{ "mvp", ShaderFieldType::Matrix4, false, 1 });
			params.uniforms.add(ShaderFieldInfo{ "tex", ShaderFieldType::Texture, false, 1 });
			params.uniforms.add(ShaderFieldInfo{ "light_pos", ShaderFieldType::Vec3, false, 1 });
			params.uniforms.add(ShaderFieldInfo{ "camera_pos", ShaderFieldType::Vec3, false, 1 });

			params.vertex_shader = R"tag(
#version 460 core

in vec3 pos;
in vec3 normal;
in vec2 tex_coord;
in vec2 tex_offset;

uniform mat4 mvp;

out vec3 v_pos;
out vec3 v_normal;
out vec2 v_tex_coord;
out vec2 v_tex_offset;

void main()
{
	gl_Position = mvp * vec4(pos, 1.0);
	v_pos = pos;
	v_normal = normal;
	v_tex_coord = tex_coord;
	v_tex_offset = tex_offset;
}
)tag";

			params.fragment_shader = R"tag(
#version 460 core

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_tex_coord;
in vec2 v_tex_offset;

out vec4 o_frag_color;

uniform sampler2D tex;
uniform vec3 light_pos;
uniform vec3 camera_pos;

void main()
{
	vec3 light_dir = normalize(light_pos - v_pos);
	// vec3 view_dir = normalize(camera_pos - v_pos);
	// vec3 reflect_dir = reflect(-light_dir, v_normal);
	// float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 1) * 0.5;
	// vec3 specular = vec3(spec, spec, spec);
	float diff = min(max(dot(v_normal, light_dir), 0.0) + 0.4, 1.0);
	vec4 obj_color = textureGrad(tex, v_tex_offset + fract(v_tex_coord), dFdx(v_tex_coord), dFdy(v_tex_coord));
	// o_frag_color = vec4((diff + specular) * obj_color.xyz, obj_color.w);
	// o_frag_color = vec4(pow(diff * obj_color.xyz, vec3(1.0/2.2)), obj_color.w);
	o_frag_color = vec4(diff * obj_color.xyz, obj_color.w);
}
)tag";
			{
				auto material = window->get_graphics().create_material(params);
				auto img = std::make_shared<Image>(Image::load_sync("../data/images/dirt.png"));
				material->uniforms[0] = mat4::identity();
				material->uniforms[1] = ShaderFieldTexture_t{ std::move(img) };
				material->uniforms[2] = vec3{ 100.0f, 300.0f, 100.0f };
				material->uniforms[3] = input_controller.position;
				materials.materials.insert({ ItemID::Dirt, std::move(material) });
			}
		}

#if GFXENGINE_EDITOR
		window->set_vsync(editor_window_vsync);
#else
		window->set_vsync(false);
#endif // GFXENGINE_EDITOR

		//world.init_random_chunks(rng, {-world_radius,-world_radius,-world_radius}, {world_radius,world_radius,world_radius}, random_count);
		world.init(*world_gen, {-world_radius,-world_radius,-world_radius}, {world_radius,world_radius,world_radius});
	}

	void run_app()
	{
		Frame frame;

		while (true)
		{
			auto t1 = platform.get_time();
			window->poll_events();

			if (should_close)
				return;

			on_update();

			frame.reset();
			on_render(frame);
			window->draw(frame);

			auto t2 = platform.get_time();

			if (fps_limit != 0)
			{
				double max_frame_time = 1.0 / fps_limit;
				double delta = t2 - t1 + 0.0003;

				if (max_frame_time > delta)
					platform.sleep(max_frame_time - delta);
			}
		}
	}
};

void run_app(Platform &platform)
{
	auto app = std::make_unique<BlocksApplication>(platform);
	app->run_app();
}
