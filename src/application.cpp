#include "world.hpp"

#include "gfxengine/base_application.hpp"
#include "gfxengine/input_controller.hpp"
#include "gfxengine/platform.hpp"
#include "gfxengine/window.hpp"
#include "gfxengine/graphics.hpp"
#include "gfxengine/frame.hpp"
#include "gfxengine/window_event_handler.hpp"
#include "gfxengine/noise_generator.hpp"

#include <random>

#if GFXENGINE_EDITOR
#include "imgui.h"
#endif // GFXENGINE_EDITOR


class BlocksApplication : public Application, public WindowEventHandler
{
private:

	Platform &platform;

	std::unique_ptr<Window> window;

	int fps_limit = 0;//120;

	bool should_close = false;
	bool mouse_locked = false;
	bool speed_up = false;

	vec3 camera_pos{};
	vec3 camera_front{};
	float camera_fov = 90.0f;

	InputController input_controller;

	int world_radius = 2;
	int random_seed = 1337;
	int random_count = 400;
	std::mt19937 rng{ (uint32_t)random_seed };
	std::unique_ptr<NoiseGenerator> world_gen = std::make_unique<NoiseGenerator>(random_seed);

	World world;
	TextureManager textures;

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

	virtual void start() override
	{
#if GFXENGINE_EDITOR
		window->set_vsync(editor_window_vsync);
#else
		window->set_vsync(false);
#endif // GFXENGINE_EDITOR

		//world.init_random_chunks(rng, {-world_radius,-world_radius,-world_radius}, {world_radius,world_radius,world_radius}, random_count);
		world.init(*world_gen, {-world_radius,-world_radius,-world_radius}, {world_radius,world_radius,world_radius});

		textures.load(ItemID::Dirt, "../data/images/dirt.png");
		Frame frame;

		while (true)
		{
			auto t1 = platform.get_time();
			window->poll_events();

			if (should_close)
				return;

			on_update();

			frame.clear();
			on_render(frame);
			window->draw(frame);

			auto t2 = platform.get_time();

			if (fps_limit != 0)
			{
				double max_frame_time = 1.0 / fps_limit;
				double delta = t2 - t1 + 0.0002;

				if (max_frame_time > delta)
					platform.sleep(max_frame_time - delta);
			}
		}
	}

	void on_update()
	{
		const double time = platform.get_time();

		input_controller.update_all(time);
		camera_pos = input_controller.position;
		camera_front = input_controller.calc_front_direction();
	}

	void on_render(Frame &frame)
	{
		constexpr auto a = mat4::identity();
		constexpr auto b = a * 2.0f;
		constexpr auto c = b.inverse();
		constexpr auto d = b * c;
		//constexpr auto a = math::vec4(1,1,0,0).normalize();
		//constexpr auto b1 = math::pow(4.0, 2.0);
		//constexpr auto b2 = math::pow(4.0, 2.5);
		//constexpr auto b3 = math::pow(4.0, 3.0);
		//constexpr auto a2 = math::sqrt(4.0f);
		//constexpr auto &ti = rtti::get_type_info<math::mat4>();
		//auto &ti = rtti::get_type_info<math::vec3>();
		//auto &ti = rtti::get_type_info<myvec3<float>>();
		//auto &ti = rtti::get_type_info<Vertex>();
		frame.setting_wireframe(editor_gfx_wireframe);
		frame.setting_culling(editor_gfx_culling);
		frame.setting_depth(editor_gfx_depth);

		{
			const mat4 proj = math::perspective(math::deg_to_rad(camera_fov), 1.0f, 0.01f, 1000.0f);
			const mat4 view = math::look_at(camera_pos, camera_pos + camera_front, vec3::unit_y());
			const mat4 model = mat4::identity();

			auto _guard1 = frame.set_model_matrix(model);
			auto _guard2 = frame.set_view_matrix(view);
			auto _guard3 = frame.set_projection_matrix(proj);

			frame.clear_background(Color(0.2f*255.0f, 0.3f*255.0f, 0.2f*255.0f, 1.0f*255.0f));

			world.on_render(frame, textures);
			world.cache_graphics(window->get_graphics());
		}

#if 0
		{
			static std::shared_ptr<Image> img = std::make_shared<Image>();
			static std::unique_ptr<NoiseGenerator> gen1 = std::make_unique<NoiseGenerator>(0);

			static bool once = true;
			if (once)
			{
				once = false;

				img->width = 2000;
				img->height = 2000;
				img->data.resize(img->width * img->height * 4);

				for (int x = 0; x < img->width; ++x)
				{
					for (int y = 0; y < img->height; ++y)
					{
						int _x = x + input_controller.position.x * 100;
						int _y = y + input_controller.position.y * 100;

						uint8_t val = (
							pow(gen1->noise(_x/400.0, _y/400.0) + 1.0, 2) / 4.0 * 1.00
							) / 1.0 * 256;

						/*
						uint8_t val = (
							((gen1->noise(_x/128.0, _y/128.0) + 1.0) / 2.0) * 1.50 +
							((gen1->noise(_x/64.0, _y/64.0) + 1.0) / 2.0) * 1.00 +
							((gen1->noise(_x/32.0, _y/32.0) + 1.0) / 2.0) * 0.50 +
							((gen1->noise(_x/16.0, _y/16.0) + 1.0) / 2.0) * 0.25
							) / 3.25 * 256;
						*/

						//uint8_t val = gen1->noise_scaled(x, y, 97)* 256;
						//uint8_t val = std::uniform_int_distribution<uint32_t>(0, 255)(rng);
						//uint8_t val = rand() * 255 / RAND_MAX;

						img->data[(y * img->width + x) * 4 + 0] = val;
						img->data[(y * img->width + x) * 4 + 1] = val;
						img->data[(y * img->width + x) * 4 + 2] = val;
						img->data[(y * img->width + x) * 4 + 3] = 255;
					}
				}
			}

			//frame.add_quad({ -0.5f, -0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f }, { 0.5f, 0.5f, 0.0f }, { -0.5f, 0.5f, 0.0f }, Color::WHITE, img, { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 });
			frame.add_quad({ -1, -1, 0 }, { 1, -1, 0 }, { 1, 1, 0 }, { -1, 1, 0 }, Color::WHITE, img, { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 });
		}
#endif // 0

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

				ImGui::Text("pos   %6.2f %6.2f %6.2f", pos.x, pos.y, pos. z);
				ImGui::Text("chunk %3d    %3d    %3d", chunk.x, chunk.y, chunk.z);
				ImGui::Text("lpos  %6.2f %6.2f %6.2f", lpos.x, lpos.y, lpos.z);
				ImGui::Text("rot   %6.2f %6.2f", rot.x, rot.y);

				ImGui::SetWindowPos(_pos, ImGuiCond_Always);
				_pos.y += ImGui::GetWindowSize().y + 5;
				ImGui::End();
			}

			if (editor_stats)
			{
				auto stats = frame.get_stats();

				ImGui::Begin("Frame Stats", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);

				ImGui::Text("draw calls:      %7d", (int)stats.draw_calls);
				ImGui::Text("api calls:       %7d", (int)stats.api_calls);

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
				}

				if (ImGui::CollapsingHeader("Window"))
				{
					if (ImGui::Checkbox("vsync", &editor_window_vsync))
					{
						window->set_vsync(editor_window_vsync);
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
				{ KeyboardEvent::Key::W, InputController::Direction::Forward },
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

	virtual void on_close_event(double time) override
	{
		should_close = true;
	}

public:

	BlocksApplication(Platform &platform)
		: platform{ platform }
	{
		CreateWindowParams params{ platform };
		params.window_event_handler = this;
		window = platform.create_window(params);

		input_controller.position = { 0.0f, 40.0f, 0.0f };
		input_controller.rotation = { -45.0f, 0.0f };
		input_controller.change_speed(0.0, 16.0f);
		input_controller.change_rotation_sensitivity(0.0, 0.1f);
	}
};

std::unique_ptr<Application> create_application(Platform &platform)
{
	return std::make_unique<BlocksApplication>(platform);
}
