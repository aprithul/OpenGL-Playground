#ifndef GUI_HPP
#define GUI_HPP

#include "SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "SDL_opengl.h"


void init_gui(SDL_Window* window, SDL_GLContext gl_context);
void draw_gui(int* light_mode, int* shadow_mode, float* bias, float* parallax_scale);
void cleanup_gui();

#endif // !GUI_HPP
