/*
 * Portions of this file are copyright Rebirth contributors and licensed as
 * described in COPYING.txt.
 * Portions of this file are copyright Parallax Software and licensed
 * according to the Parallax license below.
 * See COPYING.txt for license details.

THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

/*
 *
 * Routines for manipulating the button gadgets.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "strutil.h"
#include "u_mem.h"
#include "maths.h"
#include "pstypes.h"
#include "event.h"
#include "gr.h"
#include "ui.h"
#include "mouse.h"
#include "key.h"

namespace dcx {

#define Middle(x) ((2*(x)+1)/4)

#define BUTTON_EXTRA_WIDTH  15
#define BUTTON_EXTRA_HEIGHT 2

int ui_button_any_drawn = 0;

gr_string_size ui_get_button_size(const grs_font &cv_font, const char *text)
{
	const auto r = gr_get_string_size(cv_font, text);
	return {r.width + BUTTON_EXTRA_WIDTH * 2 + 6, r.height + BUTTON_EXTRA_HEIGHT * 2 + 6};
}

namespace {

void ui_draw_button(UI_DIALOG &dlg, UI_GADGET_BUTTON &button)
{
#if 0  //ndef OGL
	if ((button.status==1) || (button.position != button.oldposition))
#endif
	{
		ui_button_any_drawn = 1;
		gr_set_current_canvas(button.canvas);
		auto &canvas = *grd_curcanv;
		color_palette_index color{0};

		gr_set_fontcolor(canvas, dlg.keyboard_focus_gadget == &button
			? CRED
			: (!button.user_function && button.dim_if_no_function
				? CGREY
				: CBLACK
			), -1);

		if (!button.text.empty())
		{
			unsigned offset;
			if (button.position == 0)
			{
				ui_draw_box_out(canvas, 0, 0, button.width-1, button.height-1);
				offset = 0;
			}
			else
			{
				ui_draw_box_in(canvas, 0, 0, button.width-1, button.height-1);
				offset = 1;
			}
			ui_string_centered(canvas, Middle(button.width) + offset, Middle(button.height) + offset, button.text.c_str());
		} else {
			unsigned left, top, right, bottom;
			if (button.position == 0)
			{
				left = top = 1;
				right = button.width - 1;
				bottom = button.height - 1;
			}
			else
			{
				left = top = 2;
				right = button.width;
				bottom = button.height;
			}
			gr_rect(canvas, 0, 0, button.width, button.height, CBLACK);
			gr_rect(canvas, left, top, right, bottom, color);
		}
	}
}

}

std::unique_ptr<UI_GADGET_BUTTON> ui_add_gadget_button(UI_DIALOG &dlg, short x, short y, short w, short h, const char *const text, int (*const function_to_call)())
{
	auto button = ui_gadget_add<UI_GADGET_BUTTON>(dlg, x, y, x + w - 1, y + h - 1);

	if ( text )
	{
		button->text = text;
	} else {
		button->text.clear();
	}
	button->width = w;
	button->height = h;
	button->position = 0;
	button->oldposition = 0;
	button->pressed = 0;
	button->user_function = function_to_call;
	button->user_function1 = NULL;
	button->hotkey1= -1;
	button->dim_if_no_function = 0;
	return button;
}

window_event_result UI_GADGET_BUTTON::event_handler(UI_DIALOG &dlg, const d_event &event)
{
	window_event_result rval = window_event_result::ignored;
	
	oldposition = position;
	pressed = 0;

	if (event.type == event_type::mouse_button_down || event.type == event_type::mouse_button_up)
	{
		const auto OnMe = ui_mouse_on_gadget(*this);

		if (B1_JUST_PRESSED && OnMe)
		{
			position = 1;
			rval = window_event_result::handled;
		}
		else if (B1_JUST_RELEASED)
		{
			if ((position == 1) && OnMe)
				pressed = 1;

			position = 0;
		}
	}

	
	if (event.type == event_type::key_command)
	{
		const auto keypress = event_key_get(event);
		if (keypress == hotkey ||
			(keypress == hotkey1 && user_function1) || 
			(dlg.keyboard_focus_gadget == this && (keypress == KEY_SPACEBAR || keypress == KEY_ENTER)))
		{
			position = 2;
			rval = window_event_result::handled;
		}
	}
	else if (event.type == event_type::key_release)
	{
		const auto keypress = event_key_get(event);
		position = 0;

		if (keypress == hotkey ||
			(dlg.keyboard_focus_gadget == this && (keypress == KEY_SPACEBAR || keypress == KEY_ENTER)))
			pressed = 1;

		if (keypress == hotkey1 && user_function1)
		{
			user_function1();
			rval = window_event_result::handled;
		}
	}

	if (event.type == event_type::window_draw)
		ui_draw_button(dlg, *this);

	if (pressed && user_function )
	{
		user_function();
		return window_event_result::handled;
	}
	else if (pressed)
	{
		rval = ui_gadget_send_event(dlg, event_type::ui_gadget_pressed, *this);
		if (rval == window_event_result::ignored)
			rval = window_event_result::handled;
	}
	
	return rval;
}

}
