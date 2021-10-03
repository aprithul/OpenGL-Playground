#include <stdio.h>
#include <iostream>
#include <chrono>
#include <functional>

#include "MathUtil.hpp"
#include "Rendering.hpp"
#include "GUI.hpp"
#include "Particle.hpp"
#include <memory>


//#include "Vertex.hpp"
//#include "Entity.hpp"
//#include "GUI.hpp"

#define S_WIDTH 1280
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





float fov_h = 150;
float fov_v = 150;
float near = 0.1f;
float far = 100.f;




int main(int argc, char *args[])
{
	
	//bool r = DoTheImportThing("Asset/cyborg.obj");

	Float_32 aspect_ratio = 0;
	Mat4x4 projection = Mat4x4::Perspective(60, 0.001f, 10.f, 16.f / 9.f);
	static Vec2f cam_rot = { 0,0 };
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);


	//Mat4x4 projection = Mat4x4::Orthogrpahic(-2, 2, -2, 2, 10, -10);
	bool is_game_running = true;
	if (!InitOpenglWindow(S_WIDTH, S_HEIGHT))
	{
		printf("Window creation with Opengl context failed\n");
		getchar();
		return 0;
	}


	init_gui(window, gl_context);


	// depth buffer to use in shadow mapping
	GLuint depth_texture = 0;
	GLuint silhouette_texture = 0;
	GLuint blur_texture = 0;
	int dim = 2048;
	GLuint depth_buffer = make_depth_framebuffer(dim, dim, &depth_texture);
	GLuint silhouette_buffer = make_color_framebuffer(dim, dim, &silhouette_texture, true);
	GLuint blur_buffer = make_color_framebuffer(dim, dim, &blur_texture, true);

	Shader shader_textured("Shaders/diffuse_shadow.v", "Shaders/diffuse_shadow.f");
	shader_textured.load_uniform_locations();

	Shader shader_shadow("Shaders/shadow.v", "Shaders/shadow.f");
	shader_shadow.load_uniform_locations();

	Shader shader_shape("Shaders/diffuse_line.v", "Shaders/diffuse_line.f");
	shader_shape.load_uniform_locations();

	Shader shader_depth("Shaders/depth.v", "Shaders/depth.f");
	shader_depth.load_uniform_locations();

	Shader shader_single_color("Shaders/single_color.v", "Shaders/single_color.f");
	shader_single_color.load_uniform_locations();

	Shader shader_blur_pass("Shaders/blur_pass.v", "Shaders/blurr_pass.f");
	shader_blur_pass.load_uniform_locations();

	Shader shader_quad_renderer("Shaders/quad_renderer.v", "Shaders/quad_renderer.f");
	shader_quad_renderer.load_uniform_locations();

	Shader shader_particle("Shaders/particle.vs", "Shaders/particle.fs");
	shader_particle.load_uniform_locations();

	// make game world
	Entity& camera = make_camera();
	//Entity sphere = make_gameobject("Asset/sphere.obj", "Asset/world.jpg", nullptr, 0.01f);
	//sphere.position = { -0.05f, 0, 0 };
	//Entity cube = make_gameobject("Asset/plane.obj", "Asset/brickwall.jpg", "Asset/brickwall_normal.jpg", 0.05f);
	//Entity cube = make_gameobject("Asset/cube.obj", "Asset/brickwall.jpg", "Asset/brickwall_normal.jpg", "Asset/brickwall_disp.jpg", 0.05f);
	
	Entity avatar = make_gameobject("Asset/Walking.fbx", "Asset/mini_knight_fem.png", nullptr, nullptr, nullptr, true, 0.005f);
	avatar.transform.position = { 1,0,0 };
	//avatar.transform.rotation = { PI / 2.f,0,PI / 2.f };

	Entity plane = make_gameobject("Asset/plane.obj", "Asset/bricks2.jpg", "Asset/bricks2_normal.jpg", "Asset/bricks2_disp.jpg", nullptr, false, 4.f);
	plane.transform.position = { 0,0,0 };
	plane.transform.rotation = { -PI / 2.f,0,0 };

	Entity cube = make_gameobject("Asset/torch.obj", "Asset/torch.jpg", "torch_norm.jpg", nullptr, nullptr, false, 0.3f);
	cube.transform.position = { -0.6,0,-0.6 };
	cube.transform.rotation = { 0,0,0 };

	Entity cyborg = make_gameobject("Asset/cyborg.obj", "Asset/cyborg_diffuse.png", "Asset/cyborg_normal.png", nullptr, "Asset/cyborg_specular.png", true, 0.4f);
	//Entity cyborg = make_gameobject("Asset/sphere.obj", "Asset/cyborg_diffuse.png", "Asset/cyborg_normal.png", nullptr, "Asset/cyborg_specular.png", true, 0.4f);
	cyborg.transform.position = { 0, 0, 0 };
	cyborg.transform.rotation = { 0,PI / 2.f,0 };

	Entity quad = make_quad();
	quad.transform.position = { 4,0,0 };
	quad.transform.rotation = { 0,0,0 };


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
	world_data.lighting_mode = 0;
	world_data.shadow_mode = 0;

	float parallax_scale = 0.02f;
	bool render_outline = false;
//	ParticleEmitter particle_emitter({ 1,1.f,0 }, { 0,0,0 }, { 0,0.2f,0 }, { 1, 1,0.f }, 1.5f, 512, "Asset/particle.png");
	ParticleEmitter particle_emitter(
		{ -0.05,1.27,0 }, { 0,0,0 }, // pos - rot
		0.2f, PI/8.f, // velocity
		{ 1,0,0,1 }, { 1,1,0,0 }, 0.01f, // color
		1.5f, 0.2f,	//life time
		512, "Asset/particle.png"); //rate ,texture

	particle_emitter.start();

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
						particle_emitter.pause();
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

		Float_32 light_rot_speed = 1.f;
		if (keystate[SDL_SCANCODE_LEFT])
			light.transform.rotation.y -= light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_RIGHT])
			light.transform.rotation.y += light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_UP])
			light.transform.rotation.x -= light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_DOWN])
			light.transform.rotation.x += light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_PAGEUP])
			light.transform.rotation.z -= light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_PAGEDOWN])
			light.transform.rotation.z += light_rot_speed * delta_time;
		if (keystate[SDL_SCANCODE_I])
			light.transform.position.z += 4 * delta_time;
		if (keystate[SDL_SCANCODE_K])
			light.transform.position.z -= 4 * delta_time;
		if (keystate[SDL_SCANCODE_L])
			light.transform.position.x += 4 * delta_time;
		if (keystate[SDL_SCANCODE_J])
			light.transform.position.x -= 4 * delta_time;
		if (keystate[SDL_SCANCODE_U])
			light.transform.position.y += 4 * delta_time;
		if (keystate[SDL_SCANCODE_O])
			light.transform.position.y -= 4 * delta_time;
		if (keystate[SDL_SCANCODE_HOME])
			particle_emitter.transform.rotation.z += delta_time * 2 * PI;
		if (keystate[SDL_SCANCODE_END])
			particle_emitter.transform.rotation.z -= delta_time * 2 * PI;



		// end input

		//light.rotation.y -= light_rot_speed * delta_time;

		Mat4x4 light_transform = light.transform.get_transform();
		float ortho_dim = 10;
		Float_32 light_aspect_ratio = 0;

		//Mat4x4 light_proj = Mat4x4::Perspective(fov_v, near, far, 1.f);
		Mat4x4 light_proj = Mat4x4::Orthogrpahic(-ortho_dim, ortho_dim, -ortho_dim, ortho_dim, 0, 5);
		world_data.light_dir = light_transform[2]; // forward vector == 3rd column
		world_data.light_dir.Normalize();
		world_data.light_view_proj = light_proj * light_transform.GetInverse();
		// calculate entity transforms
		//Mat4x4 sphere_transform = get_transform(sphere);
		Mat4x4 cube_transform = cube.transform.get_transform();
		Mat4x4 cyborg_transform = cyborg.transform.get_transform();
		Mat4x4 plane_transform = plane.transform.get_transform();
		Mat4x4 quad_transform = quad.transform.get_transform();
		Mat4x4 avatar_transform = avatar.transform.get_transform();

		// Set camera position and orientation
		Mat4x4 cam_transform = camera.transform.get_transform();
		Vec3f cam_right = cam_transform[0];
		Vec3f cam_up = cam_transform[2];
		camera.transform.position += (cam_right * right_move_factor + cam_up * up_move_factor)*delta_time*0.5f;
		Vec3f mouse_dir = { mouse_motion_y / (float)S_HEIGHT, mouse_motion_x / (float)S_WIDTH, 0 };
		camera.transform.rotation += mouse_dir * delta_time*100.f;
		//cam_transform = camera.get_transform();
		world_data.camera_pos = camera.transform.position;


		// render
		

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		
		//ImGui::ShowDemoWindow();
		configure_lighting_parameter(&world_data.lighting_mode, &world_data.shadow_mode, &world_data.bias, &parallax_scale, &render_outline);
		configure_scene(&fov_h, &fov_v, &near, &far);


		particle_emitter.update(delta_time);
		//std::cout << "Particle count: "<< particle_emitter.get_count() << std::endl;

		// shadow depth pass
		// render shadow depth from light's pov to depth_buffer
		glViewport(0, 0, dim, dim);
		glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer);
		glClear(GL_DEPTH_BUFFER_BIT);
		draw_entity(plane, world_data, light_proj, light_transform.GetInverse(), plane_transform, shader_shadow, parallax_scale, GL_TRIANGLES);
		draw_entity(cube, world_data, light_proj, light_transform.GetInverse(), cube_transform, shader_shadow, parallax_scale, GL_TRIANGLES);
		draw_entity(avatar, world_data, light_proj, light_transform.GetInverse(), avatar_transform, shader_shadow, parallax_scale, GL_TRIANGLES);

		//draw_entity(cyborg, world_data, light_proj, light_transform.GetInverse(), cyborg_transform, shader_shadow, parallax_scale, GL_TRIANGLES);



		if (render_outline)
		{
			// VFX
			// blur buffer pass
			// render single color (white) to silhouette buffer
			glDisable(GL_DEPTH_TEST);
			glViewport(0, 0, dim, dim);
			glBindFramebuffer(GL_FRAMEBUFFER, silhouette_buffer);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//draw_entity(plane, world_data, projection, cam_transform.GetInverse(), plane_transform, shader_single_color, parallax_scale, GL_TRIANGLES);
			draw_entity(cyborg, world_data, projection, cam_transform.GetInverse(), cyborg_transform, shader_single_color, 0, GL_TRIANGLES);

			// render blurred silhoutte to blur_buffer
			glBindFramebuffer(GL_FRAMEBUFFER, blur_buffer);
			glViewport(0, 0, dim, dim);
			quad.texture_diffuse = silhouette_texture;
			glClearColor(0.4, 0.4, 0.4, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			draw_entity(quad, world_data, projection, cam_transform.GetInverse(), quad_transform, shader_blur_pass, 0, GL_TRIANGLES);
		}

		// render opeque geometry
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, S_WIDTH, S_HEIGHT);
		glClearColor(0.05, 0.05, 0.05,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_entity(plane, world_data, projection, cam_transform.GetInverse(), plane_transform, shader_textured, parallax_scale, GL_TRIANGLES);
		draw_entity(cube, world_data, projection, cam_transform.GetInverse(), cube_transform, shader_textured, parallax_scale, GL_TRIANGLES);
		draw_entity(avatar, world_data, projection, cam_transform.GetInverse(), avatar_transform, shader_textured, parallax_scale, GL_TRIANGLES);
		//draw_entity(light, world_data, projection, cam_transform.GetInverse(), light_transform, shader_shape, 0, GL_TRIANGLES);

		if (render_outline)
		{	// render outline
			glDisable(GL_DEPTH_TEST);
			quad.texture_diffuse = blur_texture;
			//glClearColor(0.3f, 0.3f, 0.3f, 1);
			//glClear(GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT);
			draw_entity(quad, world_data, projection, cam_transform.GetInverse(), quad_transform, shader_quad_renderer, 0, GL_TRIANGLES);
		}

		// render entities on top of the outline
		glEnable(GL_DEPTH_TEST);
		//draw_entity(cyborg, world_data, projection, cam_transform.GetInverse(), cyborg_transform, shader_textured, 0, GL_TRIANGLES);

		//glDisable(GL_DEPTH_TEST);
		draw_particle_emitter(particle_emitter, shader_particle, cam_transform.GetInverse(), projection,  GL_TRIANGLES);



		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glViewport(0, 0, S_WIDTH, S_HEIGHT);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//
//		// and then blur it
//		Mat4x4 quad_transform = get_transform(quad);
//		quad.texture_diffuse = blur_texture;
//		draw_entity(quad, world_data, projection, cam_transform.GetInverse(), quad_transform, shader_blur_pass, 0, GL_TRIANGLES);

		// color pass
		// do a full render pass on top of the blur
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//quad.texture_diffuse = depth_texture;
		//quad.texture_diffuse = blur_texture;
		//glViewport(0, 0, S_WIDTH, S_HEIGHT);
		//glClearColor(0.3f, 0.3f, 0.3f, 1);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//draw_entity(sphere, world_data, projection, cam_transform.GetInverse(), sphere_transform, shader, GL_TRIANGLES);
			//draw_entity(quad, world_data, projection, cam_transform.GetInverse(), quad_transform, shader_depth, 0, GL_TRIANGLES);
		

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//draw_entity(quad, world_data, projection, cam_transform.GetInverse(), quad_transform, shader_quad_renderer, 0, GL_TRIANGLES);



		ImGui::Render();


		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		

		SDL_GL_SwapWindow(window);

		delta_time = (SDL_GetTicks() - last_time) / 1000.f;
		last_time = SDL_GetTicks();

	}
	
	cleanup_gui();
	SDL_Quit();
	return 0;
}