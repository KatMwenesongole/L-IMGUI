#include <windows.h>
#include <shellscalingapi.h>

#include "p:/Handmade/handmade.cpp"
#include "p:/Handmade/handmade_opengl.cpp"
#include "p:/Handmade/handmade_opengl_windows.cpp"
#include "p:/Handmade/handmade_math.cpp"
#include "p:/Handmade/handmade_os.cpp"
#include "p:/Handmade/handmade_windows.cpp"
#include "p:/Handmade/handmade_string.cpp"
#include "p:/Handmade/handmade_keymap.cpp"
#include "p:/Handmade/handmade_keymap_windows.cpp"
#include "p:/Handmade/handmade_graphics_2d.cpp"

#include "l_imgui.cpp"
#include "l_imgui_test.cpp"

global b32 running = false;

LRESULT WINAPI
windows_procedure_messages(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if(message == WM_CLOSE)
    {
	running = false;
    }
    return(DefWindowProcA(window, message, wparam, lparam));
}

s32 WINAPI
WinMain (HINSTANCE      instance,
	 HINSTANCE prev_instance,
	 LPSTR           cmdline,
	 s32        show_cmdline)
{
    
    b32 fullscreen = false;

    windows_information info = {};
    
    if(windows_initialise_window(&info, instance, windows_procedure_messages, 1920, 1080,
	                         "Lightweight Immediate Mode Graphical User Interface (L-IMGUI)"))
    {
	// input.
	action_map current_map  = {};
	action_map previous_map = {};
	
	// @ graphics
	render_information_primitive primitive = {};
	graphics_primitives_initialise(&primitive, info.window_width, info.window_height);

	// @ font
	io_file file = io_readfile("../data/Ubuntu_36.font");
	if(file.source)
	{
	    font_header* header = (font_header*)file.source;

	    s8* source = (s8*)header + header->byte_offset;
	    s8* glyphs = (s8*)header + header->glyph_offset;

	    graphics_primitive_set_font_colour(&primitive.font, 1.0, 1.0, 1.0, 1.0);
	    graphics_primitive_set_font_texture(&primitive.font, opengl_texture_compile(source, header->width, header->height));
	    graphics_primitive_set_font_linespacing(&primitive.font, header->line_spacing);

	    for(s32 g = 0; g < header->glyph_count; g++)
	    {
		graphics_primitive_set_font_glyph(&primitive.font,
						  ((glyph_header*)glyphs)[g].character,
						  ((glyph_header*)glyphs)[g].width, ((glyph_header*)glyphs)[g].height,
						  ((glyph_header*)glyphs)[g].offset,
						  ((glyph_header*)glyphs)[g].spacing,((glyph_header*)glyphs)[g].pre_spacing,
						  ((glyph_header*)glyphs)[g].u0, ((glyph_header*)glyphs)[g].v0, ((glyph_header*)glyphs)[g].u1, ((glyph_header*)glyphs)[g].v1);
	    }
	    io_freefile(file);
	}
	
	// @ imgui

	u32   data_size = megabytes(10);
	void* data_base = VirtualAlloc(0, data_size, MEM_COMMIT, PAGE_READWRITE);
	ASSERT(data_base)

	imgui_state imgui = {};
	imgui_initialise(&imgui, data_base, data_size, &primitive, &current_map);

	// timing.
	LARGE_INTEGER counter_frequency;
	QueryPerformanceFrequency(&counter_frequency);
    
	LARGE_INTEGER counter_begin = {};
	LARGE_INTEGER counter_mid   = {};
	LARGE_INTEGER counter_end   = {};

	r32 actual_ms = 0.0;
	r32 target_ms = ((r32)1000) / 60;
	r32  frame_ms = 0;

	running = true;

	while(running)
	{
	    QueryPerformanceCounter(&counter_begin);
	    
	    windows_process_messages(&info);
	    windows_actions_update(info.window, info.window_width, info.window_height, &current_map, &previous_map);

	    if(current_map.actions[ACTION_CTRL].down && current_map.actions[ACTION_F].pressed)
	    {
		windows_fullscreen(&info, &fullscreen);
		graphics_primitive_set_window_dimensions(&primitive, info.window_width, info.window_height);
	    }
	    else if(current_map.actions[ACTION_ESC].pressed)
	    {
		running = false;
	    }

	    // begin program.

	    imgui_update(&primitive, &imgui);

	    // end program.
	    
	    QueryPerformanceCounter(&counter_mid);
	    actual_ms = ((r32)(counter_mid.QuadPart - counter_begin.QuadPart)/counter_frequency.QuadPart) * 1000;

	    if(actual_ms < target_ms)
	    {
		Sleep(target_ms - actual_ms);
	    }

	    SwapBuffers(info.window_dc); // @ show prepared frame

	    previous_map = current_map;

	    QueryPerformanceCounter(&counter_end);
	    frame_ms = ((r32)(counter_end.QuadPart - counter_begin.QuadPart)/counter_frequency.QuadPart) * 1000;
	}
    }
    
    return(0);
}
