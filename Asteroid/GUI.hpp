#ifndef GUI_HPP
#define GUI_HPP

#include "SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "SDL_opengl.h"


void init_gui(SDL_Window* window, SDL_GLContext gl_context);
void configure_lighting_parameter(int* light_mode, int* shadow_mode, float* bias, float* parallax_scale, bool* render_outline);
void configure_scene(float* fov_h, float* fov_v, float* n, float* f);
void cleanup_gui();

#endif // !GUI_HPP
