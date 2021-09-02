#include "GUI.hpp"

void init_gui(SDL_Window* window, SDL_GLContext gl_context )
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
	const char* glsl_version = "#version 330";

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);
}
void draw_gui(int* light_mode, int* shadow_mode, float* bias, float* parallax_scale)
{
	if (!ImGui::Begin("Control panel"))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
	}

	ImGui::PushItemWidth(-1);
	ImGui::Text("Light Mode");
	ImGui::SameLine();
	const char* light_modes[] = { "Unlit", "Lit", "Normal", "Parallax", "PCF shadow" };
	ImGui::PushID(light_mode);
	ImGui::Combo("", light_mode, light_modes, IM_ARRAYSIZE(light_modes));
	ImGui::PopID();
	
	ImGui::NewLine();
	ImGui::Text("Shadow");
	ImGui::SameLine();
	const char* shadow_modes[] = { "Off", "Hard Shadow", "Soft Shadow", "PCSS"};
	ImGui::PushID(shadow_mode);
	ImGui::Combo("", shadow_mode, shadow_modes, IM_ARRAYSIZE(shadow_modes));
	ImGui::PopID();

	ImGui::NewLine();
	ImGui::Text("Shadow Bias");
	ImGui::SameLine();
	ImGui::PushID(bias);
	ImGui::InputFloat("", bias, 0.0001f, 0.01f, "%0.5f");
	ImGui::PopID();

	ImGui::NewLine();
	ImGui::Text("Parallax Scale");
	ImGui::SameLine();
	ImGui::PushID(parallax_scale);
	ImGui::InputFloat("", parallax_scale, 0.01f, 0.01f, "%0.5f");
	ImGui::PopID();
	ImGui::PopItemWidth();
	//if (ImGui::BeginCombo("light_mode_combo", "0"))
	//{
	//	ImGui::EndCombo();
	//}
	//ImGui::EndCombo();


	ImGui::End();
}
void cleanup_gui()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}
