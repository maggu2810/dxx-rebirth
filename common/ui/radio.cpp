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
 * Radio box gadget stuff.
 *
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
#include "u_mem.h"
#include "strutil.h"

namespace dcx {

#define Middle(x) ((2*(x)+1)/4)

namespace {

void ui_draw_radio(UI_DIALOG &dlg, UI_GADGET_RADIO &radio)
{
#if 0  //ndef OGL
	if ((radio->status==1) || (radio->position != radio->oldposition))
#endif
	{
		gr_set_current_canvas(radio.canvas);
		auto &canvas = *grd_curcanv;
		gr_set_fontcolor(canvas, dlg.keyboard_focus_gadget == &radio ? CRED : CBLACK, -1);

		unsigned bias;
		if (radio.position == 0)
		{
			ui_draw_box_out(canvas, 0, 0, radio.width - 1, radio.height - 1);
			bias = 0;
		} else {
			ui_draw_box_in(canvas, 0, 0, radio.width - 1, radio.height - 1);
			bias = 1;
		}
		ui_string_centered(canvas, Middle(radio.width) + bias, Middle(radio.height) + bias, radio.flag ? "O" : " ");
		gr_ustring(canvas, *canvas.cv_font, radio.width + 4, 2, radio.text.get());
	}
}

}


std::unique_ptr<UI_GADGET_RADIO> ui_add_gadget_radio(UI_DIALOG &dlg, short x, short y, short w, short h, short group, const char *const text)
{
	auto radio = ui_gadget_add<UI_GADGET_RADIO>(dlg, x, y, x + w - 1, y + h - 1);
	radio->text = d_strdup(text);
	radio->width = w;
	radio->height = h;
	radio->position = 0;
	radio->oldposition = 0;
	radio->pressed = 0;
	radio->flag = 0;
	radio->group = group;
	return radio;
}

window_event_result UI_GADGET_RADIO::event_handler(UI_DIALOG &dlg, const d_event &event)
{
	oldposition = position;
	pressed = 0;

	window_event_result rval = window_event_result::ignored;
	if (event.type == event_type::mouse_button_down || event.type == event_type::mouse_button_up)
	{
		const auto OnMe = ui_mouse_on_gadget(*this);

		if ( B1_JUST_PRESSED && OnMe)
		{
			position = 1;
			rval = window_event_result::handled;
		} 
		else if (B1_JUST_RELEASED)
		{
			if ((position==1) && OnMe)
				pressed = 1;
			
			position = 0;
		}
	}

	if (event.type == event_type::key_command)
	{
		const auto key = event_key_get(event);
		if (dlg.keyboard_focus_gadget == this && (key == KEY_SPACEBAR || key==KEY_ENTER))
		{
			position = 2;
			rval = window_event_result::handled;
		}
	}
	else if (event.type == event_type::key_release)
	{
		const auto key = event_key_get(event);
		position = 0;
		if (dlg.keyboard_focus_gadget == this && (key == KEY_SPACEBAR || key == KEY_ENTER))
			pressed = 1;
	}
	if (pressed == 1 && flag == 0)
	{
		auto tmp = next;

		while (tmp != this)
		{
			if (tmp->kind==UI_GADGET_RADIO::s_kind)
			{
				auto tmpr = static_cast<UI_GADGET_RADIO *>(tmp);
				if ((tmpr->group == group ) && (tmpr->flag) )
				{
					tmpr->flag = 0;
					tmpr->pressed = 0;
				}
			}
			tmp = tmp->next;
		}
		flag = 1;
		rval = ui_gadget_send_event(dlg, event_type::ui_gadget_pressed, *this);
		if (rval == window_event_result::ignored)
			rval = window_event_result::handled;
	}

	if (event.type == event_type::window_draw)
		ui_draw_radio(dlg, *this);

	return rval;
}

void ui_radio_set_value(UI_GADGET_RADIO &radio, int value)
{
	value = value != 0;
	if (radio.flag == value)
		return;

	radio.flag = value;
	for (auto tmp = &radio; (tmp = static_cast<UI_GADGET_RADIO *>(tmp->next)) != &radio;)
	{
		if (tmp->kind == UI_GADGET_RADIO::s_kind && tmp->group == radio.group && tmp->flag)
		{
			tmp->flag = 0;
		}
	}
}

}
