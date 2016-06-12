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
// File          : $RCSfile: ui-applet-gnome.h,v $
// Revision      : $Revision: 1.17 $
// Revision date : $Date: 2011/12/29 18:39:07 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __APPLET_GNOME_H__
#define __APPLET_GNOME_H__

#include <panel-applet.h>
#include "ui-applet-gui.h"


class AppletGnome : public AppletGUI {

protected:
	// Pointer to the GtkWidget of the applet
	GtkWidget						*applet_;

public:
	// ========================================================================
	//  base
	// ========================================================================
	AppletGnome (class Biff *biff);
	~AppletGnome (void);

	// ========================================================================
	//  tools
	// ========================================================================
	gboolean get_orientation (GtkOrientation &orient);

	// ========================================================================
	//  main
	// ========================================================================
	PanelApplet *panelapplet() {return (PANEL_APPLET (applet_));}
	void dock (GtkWidget *applet);
	gboolean update (gboolean init = false);
	void show (std::string name = "dialog");
	void hide (std::string name = "dialog");
	gboolean calculate_size (GtkAllocation *allocation);
			
	// ========================================================================
	//  callbacks
	// ========================================================================
	gboolean on_button_press (GdkEventButton *event);
	static gboolean gnubiff_applet_factory (PanelApplet *applet,
											const gchar *iid, gpointer data);
};

#endif
