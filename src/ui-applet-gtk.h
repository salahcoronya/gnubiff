// ========================================================================
// gnubiff -- a mail notification program
// Copyright (c) 2000-2007 Nicolas Rougier, 2004-2007 Robert Sowada
//
// This program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ========================================================================
//
// File          : $RCSfile: ui-applet-gtk.h,v $
// Revision      : $Revision: 1.15 $
// Revision date : $Date: 2006/01/01 16:44:53 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __APPLET_GTK_H__
#define __APPLET_GTK_H__

#include "ui-applet-gui.h"


class AppletGtk : public AppletGUI {

 public:
	// ========================================================================
	//  base
	// ========================================================================
	AppletGtk (class Biff *biff);
	AppletGtk (class Biff *biff, class Applet *applet);
	~AppletGtk (void);

	// ========================================================================
	//  main
	// ========================================================================
	gboolean update (gboolean init = false);
	void show (std::string name = "dialog");

	// ========================================================================
	//  callbacks
	// ========================================================================
	gboolean on_button_press (GdkEventButton *event);
	void on_menu_quit (void);
};

#endif
