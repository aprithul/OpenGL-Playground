#include <stdio.h>
#include <iostream>
#include <chrono>
#include <functional>

#include "MathUtil.hpp"
#include "Rendering.hpp"
#include "GUI.hpp"

//#include "Vertex.hpp"
//#include "Entity.hpp"
//#include "GUI.hpp"

#define S_WIDTH 1200
#define S_HEIGHT 720

typedef int Int_32;
typedef unsigned int Uint_32;
typedef float Float_32;

//
//void benchmark(int iterations)
//{
//	Mat4x4 m1 = Mat4x4::Identity();
//	//Mat4x4 m2 = Mat4x4::Orthogrpahic(-1, 1, 5, 2, 6, 7);
//	Float_32 aspect_ratio = 0;
//	Mat4x4 m2 = Mat4x4::Perspective(90.f, 90.f, 0.001, 100.f, aspect_ratio);
//
//	auto start = std::chrono::high_resolution_clock::now();
//
//	while (iterations-- > 0)
//	{
//		Mat4x4 m3 = m1 * m2;
//	}
//
//	auto end = std::chrono::high_resolution_clock::now();
//	std::chrono::duration<double> _duration = (end - start);
//	//std::cout <<_duration.count() << '\n';
//	printf("Benchmark duration: %0.7lf\n", _duration.count());
//
//}




Mat4x4 get_transform(Entity& ent)
{
	Mat3x3 rot_x = { 1, 0, 0,
					0, cosf(ent.rotation.x), -sinf(ent.rotation.x),
					0, sinf(ent.rotation.x), cosf(ent.rotation.x) };

	Mat3x3 rot_y = { cosf(ent.rotation.y), 0, sinf(ent.rotation.y),
				0, 1, 0,
				-sinf(ent.rotation.y), 0,	cosf(ent.rotation.y) };

	Mat3x3 rot_z = { cosf(ent.rotation.z), -sinf(ent.rotation.z), 0,
					sinf(ent.rotation.z), cosf(ent.rotation.z), 0,
					0,	0,	1}; 

	Mat4x4 transform = rot_z * rot_y * rot_x;

	transform(0, 3) = ent.position.x;
	transform(1, 3) = ent.position.y;
	transform(2, 3) = ent.position.z;

	return transform;
}



int main(int argc, char *args[])
{

	Float_32 aspect_ratio = 0;
	Mat4x4 projection = Mat4x4::Perspective(90, 60, 0.001f, 10.f, aspect_ratio);
	static Vec2f cam_rot = {0,0};
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);


	

	//Mat4x4 projection = Mat4x4::Orthogrpahic(-2, 2, -2, 2, 10, -10);
	bool is_game_running = true;
	if (!InitOpenglWindow(1280, 720))
	{
		printf("Window creation with Opengl context failed\n");
		getchar();
		return 0;
	}

	
	init_gui(window, gl_context);
	

	// depth buffer to use in shadow mapping
	GLuint depth_texture = 0;
	int dim = 4096;
	GLuint depth_buffer = make_depth_framebuffer(dim, dim, &depth_texture);

	Shader shader_textured("Shaders/diffuse_shadow.v", "Shaders/diffuse_shadow.f");
	shader_textured.load_uniform_locations();

	Shader shader_shadow("Shaders/shadow.v", "Shaders/shadow.f");
	shader_shadow.load_uniform_locations();

	Shader shader_shape("Shaders/diffuse_line.v", "Shaders/diffuse_line.f");
	shader_shape.load_uniform_locations();


	// make game world
	Entity camera = make_camera();
	//Entity sphere = make_gameobject("Asset/sphere.obj", "Asset/world.jpg", nullptr, 0.01f);
	//sphere.position = { -0.05f, 0, 0 };
	//Entity cube = make_gameobject("Asset/plane.obj", "Asset/brickwall.jpg", "Asset/brickwall_normal.jpg", 0.05f);
	//Entity cube = make_gameobject("Asset/cube.obj", "Asset/brickwall.jpg", "Asset/brickwall_normal.jpg", "Asset/brickwall_disp.jpg", 0.05f);
	Entity plane = make_gameobject("Asset/plane.obj", "Asset/bricks2.jpg", "Asset/bricks2_normal.jpg", "Asset/bricks2_disp.jpg", nullptr, false, 4.f);
	plane.position = { 0,0,0 };
	plane.rotation = { -PI/2.f,0,0 };
	//Entity cube = make_gameobject("Asset/cube_2.obj", "Asset/bricks2.jpg", "Asset/bricks2_normal.jpg", "Asset/bricks2_disp.jpg", 1.f);
	//cube.position = { 0,-0.01,0 };
	//cube.rotation = {PI,0,0 };

	Entity cyborg = make_gameobject("Asset/cyborg.obj", "Asset/cyborg_diffuse.png", "Asset/cyborg_normal.png", nullptr, "Asset/cyborg_specular.png", true, 0.4f);
	cyborg.position = { 0,0.f,0 };
	cyborg.rotation = { 0,PI,0 };


	Entity light = make_light_entity();

	SDL_Event game_event;
	// input variables
	int rotation_direction = 0;
	int forward_move_factor = 0;
	int right_move_factor = 0;
	int up_move_factor = 0;
	bool if_mouse_right_button_held = 0;
	double last_time = SDL_GetTicks();
	float delta_time = 0;
	WorldData world_data;
	world_data.light_depth_map = depth_texture;
	world_data.bias = 0.0005f;
	world_data.lighting_mode = 4;
	float parallax_scale = 0.02f;

	while (is_game_running)
	{
		Int_32 mouse_motion_x = 0;
		Int_32 mouse_motion_y = 0;
		
		// handle inputs
		while (SDL_PollEvent(&game_event))
		{
			switch (game_event.type)
			{
			
			case SDL_KEYDOWN:
			{
				switch (game_event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
					{
						is_game_running = false;
					}
					break;
					case SDLK_w:
					{
						up_move_factor = 1;
					}
					break;
					case SDLK_a:
					{
						right_move_factor = -1;
					}
					break;
					case SDLK_s:
					{
						up_move_factor = -1;
					}
					break;
					case SDLK_d:
					{
						right_move_factor = 1;
					}
					break;
					case SDLK_LCTRL:
					{
					}
					break;
				}

			}
				break;
			case SDL_KEYUP:
			{
				switch (game_event.key.keysym.sym)
				{
					case SDLK_w:
					case SDLK_s:
					{
						up_move_factor = 0;
					}
					break;
					case SDLK_a:
					case SDLK_d:
					{
						right_move_factor = 0;
					}
					break;
					case SDLK_LCTRL:
						if_mouse_right_button_held = false;
					break;
				}
			}
			break;
			case SDL_MOUSEMOTION:
			{
				if (if_mouse_right_button_held)
				{
					mouse_motion_x = game_event.motion.xrel;
					mouse_motion_y = game_event.motion.yrel;
				}
			}
			break;
			case SDL_MOUSEBUTTONDOWN:
			{
				if(game_event.button.button == 3)
					if_mouse_right_button_held = true;

			}
			break;
			case SDL_MOUSEBUTTONUP:
			{
				if (game_event.button.button == 3)
					if_mouse_right_button_held = false;

			}
			break;

			default:
				break;
			}
		}

		Float_32 light_rot_speed = 2.f;
		if (keystate[SDL_SCANCODE_LEFT])
			light.rotation.y -= light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_RIGHT])
			light.rotation.y += light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_UP])
			light.rotation.x -= light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_DOWN])
			light.rotation.x += light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_I])
			light.position.y += delta_time;
		if (keystate[SDL_SCANCODE_K])
			light.position.y -= delta_time;

		// end input


		Mat4x4 light_transform = get_transform(light);
		float ortho_dim = 10;
		Float_32 light_aspect_ratio = 0;
		//Mat4x4 light_proj = Mat4x4::Perspective(90,90, 0.1f, 1000.f, light_aspect_ratio);
		Mat4x4 light_proj = Mat4x4::Orthogrpahic(-ortho_dim, ortho_dim, -ortho_dim, ortho_dim, 0, 10);
		world_data.light_dir = light_transform[2]; // forward vector == 3rd column
		world_data.light_dir.Normalize();
		world_data.light_view = light_proj * light_transform.GetInverse();
		// calculate entity transforms
		//Mat4x4 sphere_transform = get_transform(sphere);
		//Mat4x4 cube_transform = get_transform(cube);
		Mat4x4 cyborg_transform = get_transform(cyborg);
		Mat4x4 plane_transform = get_transform(plane);

		// Set camera position and orientation
		Mat4x4 cam_transform = get_transform(camera);
		Vec3f cam_right = cam_transform[0];
		Vec3f cam_up = cam_transform[2];
		camera.position += (cam_right * right_move_factor + cam_up*up_move_factor)*delta_time;
		Vec3f mouse_dir = { mouse_motion_y / (float)S_HEIGHT, mouse_motion_x / (float)S_WIDTH, 0 };
		camera.rotation += mouse_dir*delta_time*100.f;
		cam_transform = get_transform(camera);
		world_data.camera_pos = camera.position;


		// render
		// shadow depth pass

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		
		//ImGui::ShowDemoWindow();
		draw_gui(&world_data.lighting_mode, &world_data.bias, &parallax_scale);
		


		glViewport(0, 0, dim, dim);
		glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer);
		glClear(GL_DEPTH_BUFFER_BIT);
		draw_entity(plane, world_data, light_proj, light_transform.GetInverse(), plane_transform, shader_shadow, parallax_scale, GL_TRIANGLES);
		draw_entity(cyborg, world_data, light_proj, light_transform.GetInverse(), cyborg_transform, shader_shadow, parallax_scale, GL_TRIANGLES);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// color pass
		glViewport(0, 0, S_WIDTH, S_HEIGHT);
		glClearColor(0.3f, 0.3f, 0.3f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//draw_entity(sphere, world_data, projection, cam_transform.GetInverse(), sphere_transform, shader, GL_TRIANGLES);
		draw_entity(plane, world_data, projection, cam_transform.GetInverse(), plane_transform, shader_textured, parallax_scale, GL_TRIANGLES);
		draw_entity(cyborg, world_data, projection, cam_transform.GetInverse(), cyborg_transform, shader_textured, 0, GL_TRIANGLES);
		draw_entity(light, world_data, projection, cam_transform.GetInverse(), light_transform, shader_shape, 0, GL_TRIANGLES);




		ImGui::Render();


		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		

		SDL_GL_SwapWindow(window);

		delta_time = (SDL_GetTicks() - last_time) / 1000.f;
		last_time = SDL_GetTicks();

	}

	cleanup_gui();
	 
	return 0;
}