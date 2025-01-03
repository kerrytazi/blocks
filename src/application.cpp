#include "world.hpp"

#include "gfxengine/base_application.hpp"
#include "gfxengine/input_controller.hpp"
#include "gfxengine/platform.hpp"
#include "gfxengine/window.hpp"
#include "gfxengine/graphics.hpp"
#include "gfxengine/frame.hpp"
#include "gfxengine/window_event_handler.hpp"

#include <random>

#if GFXENGINE_EDITOR
#include "imgui.h"
#endif // GFXENGINE_EDITOR

class BlocksApplication : public Application, public WindowEventHandler
{
private:

	Platform &platform;

	std::unique_ptr<Window> window;

	int fps_limit = 120;

	bool should_close = false;
	bool mouse_locked = false;
	bool speed_up = false;

	glm::vec3 camera_pos{};
	glm::vec3 camera_front{};
	float camera_fov = 90.0f;

	InputController input_controller;

	int world_radius = 2;
	int random_seed = 1337;
	int random_count = 400;
	std::mt19937 rng{ (uint32_t)random_seed };

	World world;
	TextureManager textures;

#if GFXENGINE_EDITOR
	bool editor_demo_window = false;
	bool editor_settings = true;
	bool editor_fps = true;
	bool editor_coord = true;
	bool editor_stats = true;

	bool editor_gfx_wireframe = false;
	bool editor_gfx_culling = true;
	bool editor_gfx_depth = true;

	bool editor_window_vsync = false;
	int editor_window_fps_limit = fps_limit;
#endif // GFXENGINE_EDITOR

	virtual void start() override
	{
#if GFXENGINE_EDITOR
		window->set_vsync(editor_window_vsync);
#else
		window->set_vsync(false);
#endif // GFXENGINE_EDITOR

		world.init_random_chunks(rng, {-world_radius,-world_radius,-world_radius}, {world_radius,world_radius,world_radius}, random_count);

		textures.load(ItemID::Dirt, "dirt.png");
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
		frame.setting_wireframe(editor_gfx_wireframe);
		frame.setting_culling(editor_gfx_culling);
		frame.setting_depth(editor_gfx_depth);

		const glm::mat4 proj = glm::perspective(glm::radians(camera_fov), 1.0f, 0.01f, 1000.0f);
		const glm::mat4 view = glm::lookAt(camera_pos, camera_pos + camera_front, glm::vec3(0, 1, 0));
		const glm::mat4 model = glm::identity<glm::mat4>();

		auto _guard1 = frame.set_model_matrix(model);
		auto _guard2 = frame.set_view_matrix(view);
		auto _guard3 = frame.set_projection_matrix(proj);

		frame.clear_background(Color(0.2f*255.0f, 0.3f*255.0f, 0.2f*255.0f, 1.0f*255.0f));

		world.on_render(frame, textures);
		world.cache_graphics(window->get_graphics());

#if GFXENGINE_EDITOR
		frame.on_draw_editor([this, &frame]() {

			ImVec2 _pos{ 5, 5 };

			if (editor_fps)
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

				glm::vec3 pos = input_controller.position;
				glm::ivec3 chunk = glm::floor(pos / 16.0f);
				glm::vec3 lpos = pos - glm::vec3(chunk) * 16.0f;
				glm::vec2 rot = input_controller.rotation;

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

				ImGui::Checkbox("FPS", &editor_fps);
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

					world.init_random_chunks(rng, {-world_radius,-world_radius,-world_radius}, {world_radius,world_radius,world_radius}, random_count);
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
					ImGui::InputInt("FPS Limit", &editor_window_fps_limit, 0);
					if (ImGui::IsItemDeactivatedAfterEdit())
						fps_limit = editor_window_fps_limit;
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
			input_controller.change_speed(time, speed_up ? 16.0f : 4.0f);
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
			glm::vec2 rotation = event.pos;
			std::swap(rotation.x, rotation.y); // y = pitch, x = yaw
			input_controller.rotate_view(time, rotation);
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
		}
	}

	virtual void on_close_event(double time)
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

		input_controller.position = { -35.0f, -35.0f, -35.0f };
		input_controller.rotation = { 45.0f, -315.0f };
		input_controller.change_speed(0.0, 4.0f);
		input_controller.change_rotation_sensitivity(0.0, 0.1f);
	}
};

std::unique_ptr<Application> create_application(Platform &platform)
{
	return std::make_unique<BlocksApplication>(platform);
}
