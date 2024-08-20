//
// INCOMPLETE!
//

// needs:
// shared/shared_keymap.cpp
// shared/shared_graphics_2d.cpp

#define IMGUI_MAX_FIELD_SIZE 256 

#define IMGUI_WHITE() 1.0, 1.0, 1.0, 1.0

struct imgui_item
{
    u32 hash = 0;
    imgui_item* parent;
    b32 active;
    
    r32 x;
    r32 y;
    r32 width;
    r32 height;
    
    s32 z;
    
    r32 child_x;
    r32 child_y;
};

struct imgui_theme
{
    r32 bar_width;
    r32 bar_height;

    r32     cursor_width; // some % of glyph width

    r32 hierarchy_padding; // some % of bar width
    r32     label_padding; // some % of bar width
    r32      text_padding;
    r32     value_padding; // some % of bar width
    r32    margin_padding; // some % of bar width (symmetrical)

    v3 colour;           // e.g item colour
    v3 accent_colour_0;  // e.g side bar colour
    v3 accent_colour_1;  // e.g panel item colour
    v3 text_colour;      // 0xFFFFFF
};
struct imgui_state
{
    imgui_theme theme;
    
    action_map* keymap;

    render_information_primitive* primitive;

    void* region_base;
    s32   region_size;
    s32   region_used;
    
    imgui_item* root;
    imgui_item* current_item;
    s32 item_count;

    string text;
    s32 text_cursor;
};

internal inline void* imgui_pushbuffer(imgui_state* imgui, s32 size)
{
    void* data = 0;
    if((imgui->region_used + size) < imgui->region_size)
    {
	data = (u8*)imgui->region_base + imgui->region_used;
	imgui->region_used += size;
    }
    ASSERT(data);
    return(data);
}

internal void
imgui_initialise(imgui_state* imgui, void* region_base, s32 region_size, render_information_primitive* primitive, action_map* keymap)
{
    // theme.

    // theme.

    imgui->keymap = keymap;

    imgui->primitive = primitive;
    
    imgui->region_base = region_base;
    imgui->region_size = region_size;
    imgui->region_used = 0;

    imgui->root = (imgui_item*)imgui->region_base;
}


internal imgui_item*
imgui_finditem(imgui_state* imgui, u32 hash)
{
    imgui_item* item = imgui->root;
    for(u32 i = 0; i < imgui->item_count; i++)
    {
	if(hash == item->hash) return(item);
	item += 1;
    }
    return(0);
}

internal u32
imgui_computehash  (s8* str)
{
    u32 hash = 0;
    s8* p;

    for (p = str; *p != '\0'; p++)
    {
	hash = 37 * hash + *p;
    }
    return (hash); //note: hash % ARRAY_SIZE (hash table)
}



// internal void
// imgui_setstate(imgui_item* item, b32 active)
// {
//     /* active:
//        when setting an item active,
//        1.you want to set that item's siblings deactive (and all their children and their children's children and so on)
//        2.you want to set that item's parent active (and their parent's parent and so on)
//        of course this means setting the siblings of the items's parents deactive
    
//        deactive:
//        when setting an item deactive,
//        1. you want to set that item's children deactive (and their children's children and so on)
//     */
//     if(active)
//     {
// 	item->active = true;

// 	if(item->parent)
// 	{
// 	    imgui_item* parent = item->parent;

// 	    imgui_setstate(parent, true);
		
// 	    u32 child_count = 0;
// 	    while(parent->children[child_count])
// 	    {
// 		imgui_item* child = parent->children[child_count];
// 		child_count += 1;

// 		if(child == item) continue;

// 		imgui_setstate(child, false);
// 	    }
// 	}
//     }
//     else
//     {
// 	item->active = false;

// 	u32 child_count = 0;
// 	while(item->children[child_count])
// 	{
// 	    imgui_item* child = item->children[child_count];

// 	    imgui_setstate(child, false);
	    
// 	    child_count += 1;
// 	}
//     }
// }


internal void
imgui_set_deactive(imgui_state* imgui, imgui_item* item)
{
    item->active = false;

    //
    // child(ren) must *also* not be active.
    //

    // find child(ren).
    
    for(s32 i = 0; i < imgui->item_count; i++)
    {
	imgui_item* child = &imgui->root[i];
	
	if(child->parent == item) // is this my child?
	{
	    imgui_set_deactive(imgui, child);
	    continue;
	}
    }
}
internal void
imgui_set_active(imgui_state* imgui, imgui_item* item)
{
    item->active = true;
    
    //
    //  siblings must *also* not be active.
    //

    // find sibling(s).

    for(s32 i = 0; i < imgui->item_count; i++)
    {
	imgui_item* sibling = &imgui->root[i];
	
	if((sibling->parent == item->parent) && (sibling != item)) // is this my sibling? (and not myself!)
 	{
	    imgui_set_deactive(imgui, sibling);
	    continue;
	}
    }
}

internal void
imgui_mousestate(imgui_state* imgui, rect region, b32* pressed, b32* touched, b32* grabbing)
{
    if(touched) *touched = false;
    
    r32 x = imgui->keymap->mouse_position.x;
    r32 y = imgui->keymap->mouse_position.y;
    
    if((x > region.x0 && x < region.x1) &&
       (y > region.y0 && y < region.y1))
    {
	if(touched) *touched = true;
	if(imgui->keymap->actions[ACTION_LMOUSE].pressed) if(pressed)  *pressed  = true;
	if(imgui->keymap->actions[ACTION_RMOUSE].holding) if(grabbing) *grabbing = true;
    }
}

internal v3
imgui_colourcalc(r32 hue, r32 saturation, r32 value, v3* base_colour)
{
    //note: outer circle (hsv -> rgb)
    r32 h = ((hue/TAU) * TAU)/(TAU/6);
    
    r32 c = saturation * (1 - value);
    r32 z = c * (1 - fabs((fmod(h, 2) - 1)));
    r32 m = (1 - value) - c;

    r32 x = 1 - fabs(fmod(h,2) - 1);

    v3 colour = {};
    if(h < 1)
    {
	colour       = { c + m, z + m, m };
	*base_colour = { 1.0, x, 0.0 };
    }
    else if(h < 2)
    {
	colour       = { z + m, c + m, m };
	*base_colour = { x, 1.0, 0.0 };
    }
    else if(h < 3)
    {
	colour       = { m, c + m, z + m };
	*base_colour = { 0.0, 1.0, x };
    }
    else if(h < 4)
    {
	colour       = { m, z + m, c + m };
	*base_colour = { 0.0, x, 1.0 };
    }
    else if(h < 5)
    {
	colour       = { z + m, m, c + m };
	*base_colour = { x, 0.0, 1.0 };
    }
    else
    {
	colour       = { c + m, m, z + m };
	*base_colour = { 1.0, 0.0, x };
    }
    return(colour);
}

imgui_item* imgui_updateitem(imgui_state* imgui, imgui_item* parent, string label, r32 x, r32 y, r32 width, r32 height)
{
    string hash_label;
    if(parent) hash_label.size = string_print(hash_label.s, "%s#%i", label.s, parent->hash);
    else hash_label = label;
    
    imgui_item* item = imgui_finditem(imgui, imgui_computehash(hash_label.s));
    if(!item)
    {
	item         = (imgui_item*)imgui_pushbuffer(imgui, sizeof(imgui_item));
	item->hash   = imgui_computehash(hash_label.s);
	item->parent = parent;
	
	imgui->item_count++;

	if(!item->parent)
	{
	    item->x      = x;
	    item->y      = y;
	    item->width  = width;
	    item->height = height;    
	}
    }
	
    if(item->parent)
    {
	item->x      = item->parent->child_x;
	item->y      = item->parent->child_y;
	item->width  = item->parent->width - (item->parent->width * imgui->theme.hierarchy_padding);
	item->height = item->parent->height;
    }

    item->child_x = item->x + (item->width  * imgui->theme.hierarchy_padding);
    item->child_y = item->y + item->height;
    
    return(item);
}

b32 imgui_title(imgui_state* imgui, imgui_item* item, r32 x, r32 y, r32 width, r32 height, string label)
{
    b32 pressed = false;
    b32 touched = false;
    b32 grabbed = false;
	
    rect item_rect = { x, y, x + width, y + height };
    imgui_mousestate(imgui, item_rect, &pressed, &touched, &grabbed); // check interaction.

    v3 background_colour = imgui->theme.colour;
    
    if(touched)
    {
	background_colour = calc_brighten(background_colour, 0.5);
    }
    
    if(pressed)
    {
	if(item->active) imgui_set_deactive(imgui, item);  // de-activate, active, pressed 
	else             imgui_set_active  (imgui, item);   //    activate, not active, pressed 
    }

    if(item->active) background_colour = calc_brighten(background_colour, 0.5);
    
    // draw.

    // background.
    graphics_primitive_set_texture(imgui->primitive, 0);
    graphics_primitive_set_colour(imgui->primitive, background_colour.r, background_colour.g, background_colour.b, 1.0);
    graphics_primitive_render_rect(imgui->primitive, item_rect, 0);
    
    // text.
    r32 text_height = 0;
    r32 text_width  = 0;
    graphics_primitive_metrics_text(imgui->primitive, label.s, label.size, &text_width, &text_height);

    r32 text_x = item->x + (width * imgui->theme.label_padding);
    r32 text_y = item->y + (height - text_height)/2;

    graphics_primitive_set_zindex(imgui->primitive, 1);
    graphics_primitive_set_colour(imgui->primitive, imgui->theme.text_colour.r, imgui->theme.text_colour.g, imgui->theme.text_colour.b, 1.0);
    graphics_primitive_render_text(imgui->primitive, label.s, label.size, vec2(text_x, text_y));
    graphics_primitive_set_zindex(imgui->primitive, 0);
	
    return(pressed);
}
void imgui_advance(imgui_state* imgui, r32 x, r32 y)
{
    imgui->current_item->child_x += x;
    imgui->current_item->child_y += y;
}

void nest(imgui_state* imgui, string label) 
{
    string hash_label;
    if(imgui->current_item)
    {
	hash_label.size = string_print(hash_label.s, "%s#%i", label.s, imgui->current_item->hash);
    }
    else
    {
	hash_label = label;
    }

    imgui_item* item = imgui_finditem(imgui, imgui_computehash(hash_label.s));
    ASSERT(item)
    
    imgui->current_item = item;
}
void unnest(imgui_state* imgui)
{
    v3 colomn_colour = calc_darken(imgui->theme.colour, 0.5);
    graphics_primitive_set_colour(imgui->primitive, colomn_colour.r, colomn_colour.g, colomn_colour.b, 1.0);
    graphics_primitive_set_texture(imgui->primitive, 0);

    rect vertex_rect = {
	imgui->current_item->x,
	imgui->current_item->y + imgui->current_item->height,
	imgui->current_item->child_x,
	imgui->current_item->child_y };

    graphics_primitive_render_rect(imgui->primitive, vertex_rect, 0);

    // the order of events here is important.
    // in the case we update the child_y of the current item, we are updating the parent,
    // when we should be updating the grandparent.
    // it sounds un-intuitive but if you study the heirarchical structure, it makes complete logical sense.

    imgui->current_item = imgui->current_item->parent;
    if(imgui->current_item)
    {
	if(imgui->current_item->parent)
	{
	    imgui->current_item->parent->child_y = vertex_rect.y1;
	}
	imgui->current_item->child_y = vertex_rect.y1;
    }
}

// internal.
void imgui_value (imgui_state* imgui, imgui_item* item, r32 x, r32 y, r32 width, r32 height, b32 edit, r32* real, s32* integer, u32* hexadecimal, v3* vec3, v4* vec4, s8* text, b32* boolean)
{
    string value_string;

    r32 temp = 0;
    if(text)
    {
	temp = item->width * imgui->theme.text_padding;
    }
    else
    {
	temp = item->width * imgui->theme.value_padding;
    }
	
    if(edit) //note: change value
    {
	if     (imgui->keymap->actions[ACTION_RARROW].pressed) // MOVE CURSOR FORWARD
	{
	    imgui->text_cursor += ((imgui->text_cursor + 1) > IMGUI_MAX_FIELD_SIZE) ? 0 : 1;
	}
	else if(imgui->keymap->actions[ACTION_LARROW].pressed) // MOVE CURSOR BACK
	{
	    imgui->text_cursor -= ((imgui->text_cursor - 1) < 0) ? 0 : 1;
	}
	else if(imgui->keymap->actions[ACTION_BACKSPACE].pressed) // DELETE CHARACTER
	{
	    imgui->text_cursor = ((imgui->text_cursor - 1) > -1) ? imgui->text_cursor - 1 : 0;

	    if(imgui->text.s[imgui->text_cursor + 1] == '\0')
	    {
		// A B \0|
		// 0 1 2
		// A \0|
		imgui->text.s[imgui->text_cursor] = '\0';
	    }
	    else
	    {
		// A B C| D E \0
		// 0 1 2  3 4 5 6 
		// A C| D E 
		mem_copy(&imgui->text.s[imgui->text_cursor + 1],
			 &imgui->text.s[imgui->text_cursor],
		         imgui->text.size-(imgui->text_cursor+1));
		imgui->text.s[imgui->text.size-1] = '\0';
		
	    }
	    imgui->text.size--;
	}
	else // ADD CHARACTER
	{
	    for(s32 k = 0; k < ACTION_COUNT; k++)
	    {
		if(imgui->keymap->actions[k].character && imgui->keymap->actions[k].pressed)
		{
		    if(imgui->text.s[imgui->text_cursor] != '\0')
		    {
			// A B| C D \0
			// 0 1  2 3 4
			// A E B| C D \0|
			mem_copy(&imgui->text.s[imgui->text_cursor],
				 &imgui->text.s[imgui->text_cursor + 1],
			         imgui->text.size - imgui->text_cursor);
			
		    }
		    imgui->text.s[imgui->text_cursor] = imgui->keymap->actions[k].character;
		    imgui->text.size++;
		    imgui->text_cursor += ((imgui->text_cursor + 1) > IMGUI_MAX_FIELD_SIZE) ? 0 : 1;
		    break;
		}
	    }
	    
	}
	
	// // // // // DRAW

	//note: display field
	r32 field_width = 0;
	r32 field_height = 0;
	graphics_primitive_metrics_text(imgui->primitive, imgui->text.s, imgui->text.size, &field_width, &field_height);

	string cursor_text = imgui->text;
	cursor_text.size = imgui->text_cursor;

	graphics_primitive_metrics_text(imgui->primitive, cursor_text.s, cursor_text.size, &field_width, &field_height);
	    
	r32 field_y = y + ((height - field_height)/2);
	graphics_primitive_set_zindex(imgui->primitive, 1);
	graphics_primitive_render_text(imgui->primitive, imgui->text.s, imgui->text.size, vec2(x + temp, field_y));
	graphics_primitive_set_zindex(imgui->primitive, 0);

	//note: field cursor
	
	rect uv_rect = { 0.0, 1.0, 0.0, 1.0 };
	rect cursor_rect =
	{
	    (x + temp) + field_width,
	    y + ((height)-(height * 0.7))/2, 
	    cursor_rect.x0 + imgui->theme.cursor_width,
	    cursor_rect.y0 + (height*0.7)   // BAD
	};
	
	graphics_primitive_set_colour(imgui->primitive, IMGUI_WHITE()); // ??? should this be part of the theme?
	graphics_primitive_set_texture(imgui->primitive, 0);
	graphics_primitive_set_zindex(imgui->primitive, 1);
	graphics_primitive_render_rect(imgui->primitive, cursor_rect, &uv_rect);
	graphics_primitive_set_zindex(imgui->primitive, 0);
    }
    else
    {
	if(real)
	{
	    real_to_string(*real, value_string.s, &value_string.size, 1);
	}
	else if(integer)
	{
	    integer_to_string(*integer, value_string.s, &value_string.size);
	}
	else if(hexadecimal)
	{
	    hex_to_string(*hexadecimal, value_string.s, &value_string.size);
	}
	else if(vec3)
	{
	    value_string.size = string_print(value_string.s, "(%r, %r, %r)", vec3->x, vec3->y, vec3->z);
	}
	else if(vec4)
	{
	    value_string.size = string_print(value_string.s, "(%r, %r, %r, %r)", vec4->x, vec4->y, vec4->z, vec4->w);
	}
	else if(text)
	{
	    value_string.size = STRING_MAX_SIZE;
	    mem_copy(text, value_string.s, STRING_MAX_SIZE);
	}
	else if(boolean)
	{
	    if(*boolean) value_string.size = string_print(value_string.s, "TRUE" );
	    else         value_string.size = string_print(value_string.s, "FALSE");
	}
	    
	// // // // // DRAW
	v3 inactive_colour = calc_darken(imgui->theme.text_colour, 0.3);
	    
	r32 text_height = 0;
	r32 text_width  = 0;
	    
	r32 text_y = 0;
	r32 text_x = x + temp;

	graphics_primitive_metrics_text(imgui->primitive, value_string.s, value_string.size, &text_width, &text_height);
	text_y = y + (height - text_height)/(r32)2;

	// prevent overflow
	// ...

	r32 overflow = (width - ((x+temp) - x)) - text_width;
	if(overflow < 0) text_x += overflow - (width * imgui->theme.label_padding);

	graphics_primitive_set_zindex(imgui->primitive, 1);
	graphics_primitive_set_colour(imgui->primitive, inactive_colour.r, inactive_colour.g, inactive_colour.b, 1.0);
	graphics_primitive_render_text(imgui->primitive, value_string.s, value_string.size, vec2(text_x, text_y));
	graphics_primitive_set_zindex(imgui->primitive, 0);
    }
}

imgui_item* imgui_x32   (imgui_state* imgui, imgui_item* parent, r32 x, r32 y, r32 width, r32 height, string label, r32* real, s32* integer, u32* hexadecimal, s8* text, b32 enabled)
{
    imgui_item* item = imgui_updateitem(imgui, parent, label, x, y, width, height);

    //title:
    imgui_title(imgui, item, item->x, item->y, item->width, item->height, label);

    //confirm:
    if(item->active && imgui->keymap->actions[ACTION_ENTER].pressed)
    {
	if     (real)        *(r32*)real        = string_to_real       (imgui->text.s);
	else if(integer)     *(s32*)integer     = string_to_integer    (imgui->text.s);
	else if(hexadecimal) *(u32*)hexadecimal = string_to_hexadecimal(imgui->text.s);
	    
	else if(text) mem_copy(imgui->text.s, text, STRING_MAX_SIZE);

	imgui_set_deactive(imgui, item);
	    
	imgui->text_cursor = 0; //note: this is incomplete, we need a better solution
	mem_clear(imgui->text.s, STRING_MAX_SIZE);
    }

    if(enabled)
    {
	if(item->active) imgui_value(imgui, item, item->x, item->y, item->width, item->height, true,  real, integer, hexadecimal, 0, 0, text, 0);    
	else             imgui_value(imgui, item, item->x, item->y, item->width, item->height, false, real, integer, hexadecimal, 0, 0, text, 0);
    }
    else
    {
	imgui_value(imgui, item, item->x, item->y, item->width, item->height, false, real, integer, hexadecimal, 0, 0, text, 0);
    }
	
    return(item);
}

b32  imgui_label (imgui_state* imgui, imgui_item* parent, string label, r32 x, r32 y, r32 width, r32 height)
{
    imgui_item* item = imgui_updateitem(imgui, parent, label, x, y, width, height);
    
    imgui_title(imgui, item, item->x, item->y, item->width, item->height, label); // title.
    if(item->parent)
    {
	imgui_advance(imgui, 0, item->height);
    }
    return(item->active);
}
b32  imgui_button(imgui_state* imgui, imgui_item* parent, string label, r32 x, r32 y, r32 width, r32 height) 
{
    b32 pressed = false;
    
    imgui_item* item = imgui_updateitem(imgui, parent, label, x, y, width, height);
    pressed = imgui_title(imgui, item, item->x, item->y, item->width, item->height, label); // title.
    item->active = false;
    if(item->parent)
    {
	imgui_advance(imgui, 0, item->height);
    }
    return(pressed);
}
b32  imgui_bool  (imgui_state* imgui, imgui_item* parent, string label, r32 x, r32 y, r32 width, r32 height, b32* boolean)
{
    imgui_item* item = imgui_updateitem(imgui, imgui->current_item, label, x, y, width, height);
    b32 pressed = false;
    pressed = imgui_title(imgui, item, item->x, item->y, item->width, item->height, label); // title.
    if(pressed) *boolean = !(*boolean);
    item->active = false;

    // value:
    imgui_value(imgui, item, item->x, item->y, item->width, item->height, false, 0, 0, 0, 0, 0, 0, boolean);

    if(item->parent)
    {
	imgui_advance(imgui, 0, item->height);
    }
    return(pressed);
}
b32  imgui_colour(imgui_state* imgui, imgui_item* parent, string label, r32 x, r32 y, r32 width, r32 height, v4* vec)
{
    imgui_item* item = imgui_updateitem(imgui, parent, label, x, y, width, height);
    imgui_title(imgui, item, item->x, item->y, item->width, item->height, label); // title.
    if(item->parent)
    {
	imgui_advance(imgui, 0, item->height);
    }

    // preview
    rect preview_colour
    {
	item->x + item->width - item->height, item->y,
	preview_colour.x0 + item->height, preview_colour.y0 + item->height 
    };
    graphics_primitive_set_zindex(imgui->primitive, 1);
    graphics_primitive_set_colour(imgui->primitive, vec->x, vec->y, vec->z, 1.0);
    graphics_primitive_set_texture(imgui->primitive, 0);
    graphics_primitive_render_rect(imgui->primitive, preview_colour, 0);
    graphics_primitive_set_zindex(imgui->primitive, 0);
	
    if(item->active)
    {
	// colour picker.
	r32 popup_width = (item->width * 2) / 3;
	    
	r32 padding = popup_width * imgui->theme.margin_padding;
	    
	r32 column_width  = (popup_width - (3 * padding))/4;
	r32    box_height = column_width * 3; // three times wider than the bar.

	r32 popup_x     = item->x + item->width + padding;
	r32 popup_height = box_height + (2 * padding);

	r32 popup_y = (9.0 < (item->y + popup_height + (item->height * 5))) ? (9.0 - padding - popup_height - (item->height * 5)) : item->y; // is it going off the bottom of the screen?

	if(item->parent)
	{
	    popup_x = item->parent->x - item->parent->width - (item->parent->width * imgui->theme.margin_padding); // place on the left.

	    // is there enough space on the left? 
	    if(popup_x < 0)  {
		popup_x = item->parent->x + item->parent->width + (item->parent->width * imgui->theme.margin_padding); // place on the right.
	    }
	}

	// background (upper section, without the buttons.)
	    
	rect background = {
	    popup_x, popup_y,
	    background.x0 + popup_width, background.y0 + popup_height
	};
	graphics_primitive_set_colour(imgui->primitive, imgui->theme.colour.r, imgui->theme.colour.g, imgui->theme.colour.b, 1.0);
	graphics_primitive_set_texture(imgui->primitive, 0);
	graphics_primitive_render_rect(imgui->primitive, background, 0);

	// box & colomn

	rect box =
	{
	    popup_x + padding, popup_y + padding,
	    box.x0 + box_height, box.y0 + box_height
	};
	rect column =
	{
	    popup_x + (2 * padding) + box_height, popup_y + padding,
	    column.x0 + column_width, column.y0 + box_height
	};
	graphics_primitive_set_zindex(imgui->primitive, 1);
	graphics_primitive_set_colour(imgui->primitive, 0, 0, 0, 1.0);
	graphics_primitive_set_texture(imgui->primitive, 0);
	graphics_primitive_render_rect(imgui->primitive, box, 0);
	graphics_primitive_render_rect(imgui->primitive, column, 0);
	graphics_primitive_set_zindex(imgui->primitive, 0);

	// r, g, b, a
	{
	    v4 colour = { 1.0, 1.0, 1.0, 1.0 };
	    
	    imgui_x32(imgui, 0, popup_x, background.y1,                      popup_width, item->height, "R", &colour.r, 0, 0, 0, true);
	    imgui_x32(imgui, 0, popup_x, background.y1 + item->height,       popup_width, item->height, "G", &colour.g, 0, 0, 0, true);
	    imgui_x32(imgui, 0, popup_x, background.y1 + (item->height * 2), popup_width, item->height, "B", &colour.b, 0, 0, 0, true);
	    imgui_x32(imgui, 0, popup_x, background.y1 + (item->height * 3), popup_width, item->height, "A", &colour.a, 0, 0, 0, true);

	    if(imgui_button(imgui, 0, "confirm", popup_x, background.y1 + (item->height * 4), popup_width, item->height))
	    {
		// save colour.
		item->active = false;
	    }

	    // preview box (comes after 'confirm' because it must be rendered on top.)
	    rect preview
	    {
		column.x0, background.y1 + (item->height * 4),
		preview.x0 + column_width, preview.y0 + item->height 
	    };
	    graphics_primitive_set_zindex(imgui->primitive, 1);
	    graphics_primitive_set_colour(imgui->primitive, colour.r, colour.g, colour.b, 1.0);
	    graphics_primitive_set_texture(imgui->primitive, 0);
	    graphics_primitive_render_rect(imgui->primitive, preview, 0);
	    graphics_primitive_set_zindex(imgui->primitive, 0);
	}
    }
    return(item->active);
}
b32 imgui_image (imgui_state* imgui, string label, GLuint image, r32 x, r32 y, r32 width, r32 height)
{
    imgui_item* item = imgui_updateitem(imgui, imgui->current_item, label, x, y, width, height);

    r32 padding = item->width * imgui->theme.margin_padding;
	
    //background:
    rect item_rect = { item->x, item->y, item->x + item->width, item->y + height + (2*padding) };
    graphics_primitive_set_colour(imgui->primitive, imgui->theme.colour.r, imgui->theme.colour.g, imgui->theme.colour.b, 1.0);
    graphics_primitive_set_texture(imgui->primitive, 0);
    graphics_primitive_render_rect(imgui->primitive, item_rect, 0);

    //image:
    rect uv_rect = { 0.0, 1.0, 1.0, 0.0 };
    rect image_rect = {
	item->x + padding, item->y + padding,
	item->x + width + padding, item->y + height + padding
    };
	
    graphics_primitive_set_colour (imgui->primitive, IMGUI_WHITE());
    graphics_primitive_set_texture(imgui->primitive, image);
    graphics_primitive_set_zindex(imgui->primitive, 1);
    graphics_primitive_render_rect(imgui->primitive, image_rect, &uv_rect);
    graphics_primitive_set_texture(imgui->primitive, 0);
    graphics_primitive_set_zindex(imgui->primitive, 0);

    imgui_advance(imgui, 0, height + (2*padding));

    return(item->active);
}

// standard.
b32 imgui_r32   (imgui_state* imgui, string label, r32* real,    r32 x = 0, r32 y = 0, r32 width = 0, r32 height = 0, b32 enabled = true)
{
    imgui_item* item = imgui_x32(imgui, imgui->current_item, x, y, (width < 1E-15) ? imgui->theme.bar_width : width, (height < 1E-15) ? imgui->theme.bar_height : height, label, real, 0, 0, 0, enabled);
    if(item->parent) imgui_advance(imgui, 0, item->height);
    return(item->active);
}
b32 imgui_s32   (imgui_state* imgui, string label, s32* integer, r32 x = 0, r32 y = 0, r32 width = 0, r32 height = 0, b32 enabled = true)
{
    imgui_item* item = imgui_x32(imgui, imgui->current_item, x, y, (width < 1E-15) ? imgui->theme.bar_width : width, (height < 1E-15) ? imgui->theme.bar_height : height, label, 0, integer, 0, 0, enabled);
    if(item->parent) imgui_advance(imgui, 0, item->height);
    return(item->active);
}
b32 imgui_hex   (imgui_state* imgui, string label, u32* hex,     r32 x = 0, r32 y = 0, r32 width = 0, r32 height = 0, b32 enabled = true)
{
    imgui_item* item = imgui_x32(imgui, imgui->current_item, x, y, (width < 1E-15) ? imgui->theme.bar_width : width, (height < 1E-15) ? imgui->theme.bar_height : height, label, 0, 0, hex, 0, enabled);
    if(item->parent) imgui_advance(imgui, 0, item->height);
    return(item->active);
}
b32 imgui_text  (imgui_state* imgui, string label, s8* text,     r32 x = 0, r32 y = 0, r32 width = 0, r32 height = 0, b32 enabled = true)
{
    imgui_item* item  = imgui_x32(imgui, imgui->current_item, x, y, (width < 1E-15) ? imgui->theme.bar_width : width, (height < 1E-15) ? imgui->theme.bar_height : height, label, 0, 0, 0, text, enabled);
    if(item->parent) imgui_advance(imgui, 0, item->height);
    return(item->active);
}
b32 imgui_v3    (imgui_state* imgui, string label, v3* vector3,  r32 x = 0, r32 y = 0, r32 width = 0, r32 height = 0, b32 enabled = true)
{
    imgui_item* item = imgui_updateitem(imgui, imgui->current_item, label, x, y, (width < 1E-15) ? imgui->theme.bar_width : width, (height < 1E-15) ? imgui->theme.bar_height : height);

    imgui_title(imgui, item, item->x, item->y, item->width, item->height, label); // title.
    imgui_value(imgui, item, item->x, item->y, item->width, item->height, false, 0, 0, 0, vector3, 0, 0, 0); // value.

    if(item->parent) imgui_advance(imgui, 0, item->height);
    
    if(item->active)
    {
	nest(imgui, label);
	imgui_r32(imgui, "x", &vector3->x, true);
	imgui_r32(imgui, "y", &vector3->y, true);
	imgui_r32(imgui, "z", &vector3->z, true);
	unnest(imgui);
    }
    
    return(item->active);
}

b32 imgui_label  (imgui_state* imgui, string label, r32 x = 0, r32 y = 0, r32 width = 0, r32 height = 0) {
    return(imgui_label(imgui, imgui->current_item, label, x, y, (width < 1E-15) ? imgui->theme.bar_width : width, (height < 1E-15) ? imgui->theme.bar_height : height));
}
b32 imgui_button (imgui_state* imgui, string label, r32 x = 0, r32 y = 0, r32 width = 0, r32 height = 0) {
    return(imgui_button(imgui, imgui->current_item, label, x, y, (width < 1E-15) ? imgui->theme.bar_width : width, (height < 1E-15) ? imgui->theme.bar_height : height));
}
b32 imgui_bool (imgui_state* imgui, string label, b32* boolean, r32 x = 0, r32 y = 0, r32 width = 0, r32 height = 0) {
    return(imgui_bool(imgui, imgui->current_item, label, x, y, (width < 1E-15) ? imgui->theme.bar_width : width, (height < 1E-15) ? imgui->theme.bar_height : height, boolean));
}
b32 imgui_colour (imgui_state* imgui, string label, v4* vec, r32 x = 0, r32 y = 0, r32 width = 0, r32 height = 0) {
    return(imgui_colour(imgui, imgui->current_item, label, x, y, (width < 1E-15) ? imgui->theme.bar_width : width, (height < 1E-15) ? imgui->theme.bar_height : height, vec));
}


void imgui_beginframe(imgui_state* imgui)
{
    graphics_primitive_set_colour(imgui->primitive, IMGUI_WHITE());
    graphics_primitive_set_texture(imgui->primitive, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
