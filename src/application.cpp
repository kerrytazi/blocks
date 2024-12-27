#include "world.hpp"

#include "gfxengine/base_application.hpp"
#include "gfxengine/input_controller.hpp"
#include "gfxengine/platform.hpp"
#include "gfxengine/window.hpp"
#include "gfxengine/frame.hpp"
#include "gfxengine/window_event_handler.hpp"

#include <chrono>
#include <thread>
#include <random>

#if GFXENGINE_EDITOR
#include "imgui.h"
#endif // GFXENGINE_EDITOR

class Application : public BaseApplication, public WindowEventHandler
{
private:

	BasePlatform &platform;

	std::unique_ptr<BaseWindow> window;

	bool should_close = false;
	bool mouse_locked = false;
	bool speed_up = false;

	glm::vec3 camera_pos{};
	glm::vec3 camera_front{};
	float camera_fov = 90.0f;

	InputController input_controller;

	int random_seed = 1337;
	std::mt19937 rng{ (uint32_t)random_seed };

	World world;
	TextureManager textures;

#if GFXENGINE_EDITOR
	bool editor_demo_window = false;
	bool editor_settings = true;
	bool editor_fps = true;
	bool editor_stats = true;

	bool editor_gfx_wireframe = false;
	bool editor_gfx_culling = true;
	bool editor_gfx_depth = true;
#endif // GFXENGINE_EDITOR

	virtual void start() override
	{
		window->set_vsync(false);

		world.init_random_chunks(rng, {-2,-2,-2}, {2,2,2});

		textures.load(ItemID::Dirt, "C:\\Users\\rav\\my\\c\\blocks\\dirt.png");
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

			// TODO: Calc time
			//std::this_thread::sleep_for(std::chrono::milliseconds(300));
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

		const glm::mat4 proj = glm::perspective(glm::radians(camera_fov), 1.0f, 0.1f, 100.0f);
		const glm::mat4 view = glm::lookAt(camera_pos, camera_pos + camera_front, glm::vec3(0, 1, 0));
		const glm::mat4 model = glm::identity<glm::mat4>();

		auto _guard1 = frame.set_model_matrix(model);
		auto _guard2 = frame.set_view_matrix(view);
		auto _guard3 = frame.set_projection_matrix(proj);

		frame.clear_background(Color(0.2f*255.0f, 0.3f*255.0f, 0.2f*255.0f, 1.0f*255.0f));

		world.on_render(frame, textures);

#if GFXENGINE_EDITOR
		frame.on_draw_editor([this, &frame]() {
			if (editor_fps)
			{
				ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);

				ImGuiIO &io = ImGui::GetIO();
				ImGui::Text("fps %6.1f   ms %6.3f", io.Framerate, 1000.0f / io.Framerate);

				ImGui::SetWindowPos({ 5, 5 }, ImGuiCond_Once);
				ImGui::End();
			}

			if (editor_stats)
			{
				ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);

				ImGui::Text("vertices:  %7d", (int)frame.data.vertices.size());
				ImGui::Text("indices:   %7d", (int)frame.data.indices.size());
				ImGui::Text("triangles: %7d", (int)frame.data.indices.size() / 3);

				ImGui::SetWindowPos({ 5, 36 }, ImGuiCond_Once);
				ImGui::End();
			}

			if (editor_settings)
			{
				ImGui::Begin("Settings", &editor_settings, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

				ImGui::Checkbox("FPS", &editor_fps);
				ImGui::Checkbox("Stats", &editor_stats);
				ImGui::Checkbox("Speed Up", &speed_up);
				ImGui::Checkbox("ImGui Demo", &editor_demo_window);

				ImGui::PushItemWidth(126);
				ImGui::InputFloat("FOV", &camera_fov, 5.0f);
				ImGui::PopItemWidth();
				ImGui::PushItemWidth(80);
				ImGui::InputInt("Random Seed", &random_seed, 0);
				ImGui::PopItemWidth();

				if (ImGui::Button("Generate Random Chunks"))
				{
					rng.seed(random_seed);
					world.init_random_chunks(rng, {-2,-2,-2}, {2,2,2});
				}

				if (ImGui::CollapsingHeader("Graphics"))
				{
					ImGui::Checkbox("Wireframe", &editor_gfx_wireframe);
					ImGui::Checkbox("Culling", &editor_gfx_culling);
					ImGui::Checkbox("Depth Test", &editor_gfx_depth);
				}

				ImGui::SetWindowPos({ 5, 67+31+3 }, ImGuiCond_Once);
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
			if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::W)
				input_controller.begin_move(time, InputController::Direction::Forward);
			if (event.type == KeyboardEvent::Type::Release && event.key == KeyboardEvent::Key::W)
				input_controller.end_move(time, InputController::Direction::Forward);

			if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::S)
				input_controller.begin_move(time, InputController::Direction::Back);
			if (event.type == KeyboardEvent::Type::Release && event.key == KeyboardEvent::Key::S)
				input_controller.end_move(time, InputController::Direction::Back);

			if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::D)
				input_controller.begin_move(time, InputController::Direction::Right);
			if (event.type == KeyboardEvent::Type::Release && event.key == KeyboardEvent::Key::D)
				input_controller.end_move(time, InputController::Direction::Right);

			if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::A)
				input_controller.begin_move(time, InputController::Direction::Left);
			if (event.type == KeyboardEvent::Type::Release && event.key == KeyboardEvent::Key::A)
				input_controller.end_move(time, InputController::Direction::Left);

			if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::Space)
				input_controller.begin_move(time, InputController::Direction::Up);
			if (event.type == KeyboardEvent::Type::Release && event.key == KeyboardEvent::Key::Space)
				input_controller.end_move(time, InputController::Direction::Up);

			if (event.type == KeyboardEvent::Type::Press && event.key == KeyboardEvent::Key::LeftShift)
				input_controller.begin_move(time, InputController::Direction::Down);
			if (event.type == KeyboardEvent::Type::Release && event.key == KeyboardEvent::Key::LeftShift)
				input_controller.end_move(time, InputController::Direction::Down);
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

	Application(BasePlatform &platform)
		: platform{ platform }
	{
		CreateWindowParams params{ platform };
		params.window_event_handler = this;
		window = platform.create_window(params);

		input_controller.position = { 3.0f, 3.0f, 3.0f };
		input_controller.rotation = { -45.0f, -135.0f };
		input_controller.change_speed(0.0, 4.0f);
		input_controller.change_rotation_sensitivity(0.0, 0.1f);
	}
};

std::unique_ptr<BaseApplication> create_application(BasePlatform &platform)
{
	return std::make_unique<Application>(platform);
}
