internal void imgui_update(render_information_primitive* primitive, imgui_state* imgui)
{
    graphics_set_backgroundcolour(112/(r32)255, 169/(r32)255, 161/(r32)255);
    
    local b32 boolean   = false;
    local v4  colour    = { 1.0, 1.0, 1.0, 1.0 };
    local r32 real      = 3.14159;
    local s32 integer   = 12;
    local u32 hex       = 3735928559;
    local v3  vector3   = { 0.2, 0.3, 0.4 };
    local s8  text[256] = {};

    imgui_beginframe(imgui);

    if(imgui_label(imgui, "panel one", 0.3, 0.3, 4.0, 0.35))
    {
	nest(imgui, "panel one");
	
	if(imgui_label(imgui, "item label (with nesting)"))
	{
	    nest(imgui, "item label (with nesting)");
	    imgui_label(imgui, "child item 0");
	    if(imgui_label(imgui, "child item 1"))
	    {
		nest(imgui, "child item 1");
		imgui_label(imgui, "child 0 of child item 1");
		imgui_label(imgui, "child 1 of child item 1");
		unnest(imgui);
	    }
	    imgui_label(imgui, "child item 2");
	    imgui_label(imgui, "child item 3");
	    imgui_label(imgui, "child item 4");
	    unnest(imgui);
	}
	imgui_button(imgui, "item button");
	imgui_bool  (imgui, "item bool",           &boolean);
	imgui_colour(imgui, "item colour",         &colour);
	//imgui_image (imgui, "item image", 0);
	imgui_r32   (imgui, "item real",           &real);
	imgui_s32   (imgui, "item signed integer", &integer);
	imgui_hex   (imgui, "item hexadecimal",    &hex);
	imgui_v3    (imgui, "item vector 3",       &vector3);
	imgui_text  (imgui, "item text",           text);
	
	unnest(imgui);
    }

    imgui_label (imgui, "a label",                    8.0, 0.25);
    imgui_button(imgui, "a button",                   8.5, 0.75);
    imgui_bool  (imgui, "a bool",           &boolean, 8.0, 1.25);
    imgui_colour(imgui, "a colour",         &colour,  8.5, 1.75);
    imgui_r32   (imgui, "a real",           &real,    true,8.0, 2.25);
    imgui_s32   (imgui, "a signed integer", &integer, true,8.5, 2.75);
    imgui_hex   (imgui, "a hexadecimal",    &hex,     true,8.0, 3.25);
    imgui_v3    (imgui, "a vector 3",       &vector3, true,8.5, 3.75); 
     
}
