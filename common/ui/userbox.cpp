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


#include <stdlib.h>
#include <string.h>

#include "maths.h"
#include "pstypes.h"
#include "event.h"
#include "gr.h"
#include "ui.h"
#include "mouse.h"
#include "key.h"

namespace dcx {

namespace {

void ui_draw_userbox(UI_DIALOG &dlg, UI_GADGET_USERBOX &userbox)
{
#if 0  //ndef OGL
	if ( userbox->status==1 )
#endif
	{
		gr_set_current_canvas(userbox.canvas);

		const uint8_t color = (dlg.keyboard_focus_gadget == &userbox)
			? CRED
			: CBRIGHT;
		gr_ubox(*grd_curcanv, -1, -1, userbox.width, userbox.height, color);
	}
}

}

std::unique_ptr<UI_GADGET_USERBOX> ui_add_gadget_userbox(UI_DIALOG &dlg, short x, short y, short w, short h)
{
	auto userbox = ui_gadget_add<UI_GADGET_USERBOX>(dlg, x, y, x + w - 1, y + h - 1);
	userbox->width = w;
	userbox->height = h;
	userbox->b1_held_down=0;
	userbox->b1_clicked=0;
	userbox->b1_double_clicked=0;
	userbox->b1_dragging=0;
	userbox->b1_drag_x1=0;
	userbox->b1_drag_y1=0;
	userbox->b1_drag_x2=0;
	userbox->b1_drag_y2=0;
	userbox->b1_done_dragging = 0;
	userbox->keypress = 0;
	userbox->mouse_onme = 0;
	userbox->mouse_x = 0;
	userbox->mouse_y = 0;
	userbox->bitmap = &(userbox->canvas->cv_bitmap);
	return userbox;
}

window_event_result UI_GADGET_USERBOX::event_handler(UI_DIALOG &dlg, const d_event &event)
{
	window_event_result rval = window_event_result::ignored;
	if (event.type == event_type::window_draw)
		ui_draw_userbox(dlg, *this);

	const auto keypress = (event.type == event_type::key_command)
		? event_key_get(event)
		: 0u;
	const auto [x, y, z] = mouse_get_pos();
	const auto OnMe = ui_mouse_on_gadget(*this);

	const auto olddrag  = b1_held_down;

	mouse_onme = OnMe;
	mouse_x = x - x1;
	mouse_y = y - y1;

	b1_dragging = 0;
	b1_clicked = 0;

	if (OnMe)
	{
		if ( B1_JUST_PRESSED )
		{
			b1_held_down = 1;
			b1_drag_x1 = x - x1;
			b1_drag_y1 = y - y1;
			rval = window_event_result::handled;
		}
		else if (B1_JUST_RELEASED)
		{
			if (b1_held_down)
				b1_clicked = 1;
			b1_held_down = 0;
			rval = window_event_result::handled;
		}

		if (event.type == event_type::mouse_moved && b1_held_down)
		{
			b1_dragging = 1;
			b1_drag_x2 = x - x1;
			b1_drag_y2 = y - y1;
		}

		if ( B1_DOUBLE_CLICKED )
		{
			b1_double_clicked = 1;
			rval = window_event_result::handled;
		}
		else
			b1_double_clicked = 0;

	}

	if (B1_JUST_RELEASED)
		b1_held_down = 0;

	b1_done_dragging = 0;

	if (olddrag==1 && b1_held_down==0 )
	{
		if ((b1_drag_x1 !=  b1_drag_x2) || (b1_drag_y1 !=  b1_drag_y2) )
			b1_done_dragging = 1;
	}

	if (dlg.keyboard_focus_gadget == this)
	{
		this->keypress = keypress;
		rval = window_event_result::handled;
	}
	if (b1_clicked || b1_dragging)
	{
		rval = ui_gadget_send_event(dlg, b1_clicked ? event_type::ui_gadget_pressed : event_type::ui_userbox_dragged, *this);
		if (rval == window_event_result::ignored)
			rval = window_event_result::handled;
	}

	return rval;
}





}
