/*
 * File: findbar.cc
 *
 * Copyright (C) 2005-2007 Jorge Arellano Cid <jcid@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <fltk/events.h>
#include <fltk/Window.h>
#include "findbar.hh"

#include "msg.h"
#include "pixmaps.h"
#include "uicmd.hh"
#include "bw.h"

/*
 * Local sub class
 * (Used to handle escape in the findbar, may also avoid some shortcuts).
 */
class MyInput : public Input {
public:
   MyInput (int x, int y, int w, int h, const char* l=0) :
      Input(x,y,w,h,l) {};
   int handle(int e);
};

int MyInput::handle(int e)
{
   _MSG("findbar MyInput::handle()\n");
   int ret = 1, k = event_key();
   unsigned modifier = event_state() & (SHIFT | CTRL | ALT | META);
   if (modifier == 0) {
      if (e == KEY && k == EscapeKey) {
         _MSG("findbar CustInput: caught EscapeKey\n");
         ret = 0;
      }
   }
   if (ret)
      ret = Input::handle(e);
   return ret;
};

/*
 * Find next occurrence of input key
 */
void Findbar::search_cb(Widget *, void *vfb)
{
   Findbar *fb = (Findbar *)vfb;
   const char *key = fb->i->value();
   bool case_sens = fb->cb->value();

   if (key[0] != '\0')
      a_UIcmd_findtext_search((BrowserWindow *) fb->window()->user_data(),
                              key, case_sens);
}

/*
 * Find next occurrence of input key
 */
void Findbar::search_cb2(Widget *widget, void *vfb)
{
   /*
    * Somehow fltk even regards the first loss of focus for the
    * window as a WHEN_ENTER_KEY_ALWAYS event.
    */ 
   if (event_key() == ReturnKey)
      search_cb(widget, vfb);
}

/*
 * Hide the search bar
 */
void Findbar::hide_cb(Widget *, void *vfb)
{
   ((Findbar *)vfb)->hide();
}

/*
 * Construct text search bar
 */
Findbar::Findbar(int width, int height) :
   Group(0, 0, width, height)
{
   int button_width = 70;
   int gap = 2;
   int border = 2;
   int input_width = width - (2 * border + 3 * (button_width + gap));
   int x = border;
   height -= 2 * border;

   Group::hide();

   begin();
    hidebutton = new HighlightButton(x, border, 16, height, 0);
    hideImg = new xpmImage(new_s_xpm);
    hidebutton->image(hideImg);
    hidebutton->tooltip("Hide");
    x += 16 + gap;
    hidebutton->callback(hide_cb, this);
    hidebutton->clear_tab_to_focus();

    i = new MyInput(x, border, input_width, height);
    x += input_width + gap;
    resizable(i);
    i->color(206);
    i->when(WHEN_ENTER_KEY_ALWAYS);
    i->callback(search_cb2, this);

    // todo: search previous would be nice
    findb = new HighlightButton(x, border, button_width, height, "Next");
    x += button_width + gap;
    findb->tooltip("Find next occurrence of the search phrase");
    findb->add_shortcut(ReturnKey);
    findb->add_shortcut(KeypadEnter);
    findb->callback(search_cb, this);

    cb = new CheckButton(x, border, 2*button_width, height, "Case-sensitive");
    x += 2 * button_width + gap;

   end();
}

Findbar::~Findbar()
{
   delete hideImg;
}

/*
 * Handle events. Used to catch EscapeKey events.
 */
int Findbar::handle(int event)
{
   int ret = 0;
   int k = event_key();
   unsigned modifier = event_state() & (SHIFT | CTRL | ALT | META);

   if (event == KEY && modifier == 0 && k == EscapeKey) {
      hide();
      ret = 1;
   }

   if (ret == 0)
      ret = Group::handle(event);

   return ret;
}

/*
 * Show the findbar and focus the input field
 */
void Findbar::show()
{
   Group::show();
   /* select text even if already focused */
   i->take_focus();
   i->position(i->size(), 0);
}

/*
 * Hide the findbar and reset the search state
 */
void Findbar::hide()
{
   BrowserWindow *bw;

   Group::hide();
   if ((bw = (BrowserWindow *) this->window()->user_data()))
      a_UIcmd_findtext_reset(bw);
   a_UIcmd_focus_main_area(bw);
}
