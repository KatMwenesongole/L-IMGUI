#include <windows.h>
#include <shellscalingapi.h>

#include "p:/Handmade Heroine/hh.cpp"
#include "p:/Handmade Heroine/hh_opengl.cpp"
#include "p:/Handmade Heroine/hh_opengl_windows.cpp"
#include "p:/Handmade Heroine/hh_math.cpp"
#include "p:/Handmade Heroine/hh_os.cpp"
#include "p:/Handmade Heroine/hh_windows.cpp"
#include "p:/Handmade Heroine/hh_string.cpp"
#include "p:/Handmade Heroine/hh_keymap.cpp"
#include "p:/Handmade Heroine/hh_keymap_windows.cpp"
#include "p:/Handmade Heroine/hh_graphics_2d.cpp"

#include "l_imgui.cpp"
#include "l_imgui_test.cpp"

global b32 running;

LRESULT WINAPI
windows_messageproc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if(message == WM_CLOSE)
    {
	running = false;
    }
    return(DefWindowProcA(window, message, wparam, lparam));
}

s32 WINAPI
WinMain (HINSTANCE instance,
	 HINSTANCE prev_instance,
	 LPSTR     lpCmdLine,
	 s32       nShowCmd)
{
    WNDCLASSA window_class = {};
    window_class.style 	       = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = windows_messageproc;
    window_class.hInstance     = instance;
    window_class.hCursor       = LoadCursorA(0, IDC_ARROW);
    window_class.lpszClassName = "Immediate Mode Graphical User Interface";

    if(RegisterClassA(&window_class))
    {
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	s32 monitor_width  = GetSystemMetrics(SM_CXSCREEN);
	s32 monitor_height = GetSystemMetrics(SM_CYSCREEN);
	
	// resize the window.
	
	s32 window_width  = 1920;
	s32 window_height = 1080;

	RECT window_rect = {
	    (monitor_width  - window_width )/2,
	    (monitor_height - window_height)/2,
	    window_width  + window_rect.left  ,
	    window_height + window_rect.top   ,
	};
	
	if(!AdjustWindowRect(&window_rect, WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION, false)) {
	    OutputDebugStringA("'AdjustWindowRect' failed!\n");
	}
	
	HWND window = CreateWindowA(window_class.lpszClassName,
				    window_class.lpszClassName,
				    WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION,
				    window_rect.left, window_rect.top,
				    (window_rect.right - window_rect.left),
				    (window_rect.bottom - window_rect.top),
				    0, 0,
				    instance,
				    0);
	if(window)
	{
	    running = true;

	    HDC window_dc = GetDC(window);
	    if(windows_opengl_initialise(window_dc))
	    {
		windows_opengl_updateviewport(window_width, window_height);

		// @ graphics
		render_information_primitive primitive = {};
		graphics_primitives_initialise(&primitive, window_width, window_height);

		// @ font
		io_file file = io_readfile("../data/Fontin_29.font");
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

		// @ input
		action_map  current_map = {};
		action_map previous_map = {};

		// @ imgui

		u32   data_size = megabytes(10);
		void* data_base = VirtualAlloc(0, data_size, MEM_COMMIT, PAGE_READWRITE);
		ASSERT(data_base)

		imgui_state imgui = {};
		imgui_initialise(&imgui, data_base, data_size, &primitive, &current_map);
		
		imgui.theme.bar_width         = 4.0; // default
		imgui.theme.bar_height        = 0.35;
		imgui.theme.cursor_width      = 0.075;
		imgui.theme.hierarchy_padding = 0.05;
		imgui.theme.label_padding     = 0.02;
		imgui.theme.text_padding      = 0.7;
		imgui.theme.value_padding     = 0.7;
		imgui.theme.margin_padding    = 0.05;
		imgui.theme.colour            = { 0.27, 0.17, 0.18 };
		imgui.theme.accent_colour_0   = { 0.44, 0.23, 0.27 };
		imgui.theme.accent_colour_1   = { 0.06, 0.2 , 0.21 };
		imgui.theme.text_colour       = { 1.0 , 1.0 , 1.0  };
		
		// @ time
		LARGE_INTEGER counter_frequency;
		QueryPerformanceFrequency(&counter_frequency);
    
		LARGE_INTEGER counter_begin = {};
		LARGE_INTEGER counter_mid   = {};
		LARGE_INTEGER counter_end   = {};

		r32 actual_ms = 0.0;
		r32 target_ms = ((r32)1000) / 60;
		r32  frame_ms = 0;

		while(running)
		{
		    QueryPerformanceCounter(&counter_begin);
		    
		    MSG message = {};
		    while(PeekMessageA(&message, window, 0, 0, PM_REMOVE))
		    {
			TranslateMessage(&message);
			DispatchMessageA(&message);
		    }
		    windows_actions_update(window, window_width, window_height, &current_map, &previous_map);

		    if(current_map.actions[ACTION_ESC].pressed) // should this be here?
		    {
			running = false;
		    }

		    // //

		    imgui_update(&primitive, &imgui);

		    // //
		    
		    QueryPerformanceCounter(&counter_mid);
		    actual_ms = ((r32)(counter_mid.QuadPart - counter_begin.QuadPart)/counter_frequency.QuadPart) * 1000;

		    if(actual_ms < target_ms)
		    {
			Sleep(target_ms - actual_ms);
		    }
		    SwapBuffers(window_dc); // @ show prepared frame

		    previous_map = current_map;

		    QueryPerformanceCounter(&counter_end);
		    frame_ms = ((r32)(counter_end.QuadPart - counter_begin.QuadPart)/counter_frequency.QuadPart) * 1000;
		}
		
	    }

	    DestroyWindow(window);
	}
	else
	{
	    OutputDebugStringA("'CreateWindowA' failed!\n");
	}

	UnregisterClassA(window_class.lpszClassName, instance);
    }
    else
    {
	OutputDebugStringA("'RegisterClassA' failed!\n");
	
    }

    return(0);
}
