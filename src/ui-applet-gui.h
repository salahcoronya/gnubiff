// ========================================================================
// gnubiff -- a mail notification program
// Copyright (c) 2000-2005 Nicolas Rougier, 2004-2005 Robert Sowada
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
// ========================================================================
//
// File          : $RCSfile: ui-applet.cc,v $
// Revision      : $Revision: 1.36 $
// Revision date : $Date: 2005/12/11 16:24:42 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __APPLET_GUI_H__
#define __APPLET_GUI_H__

#include "gui.h"
#include "ui-applet.h"

/**
 *  Generic GUI code common for all types of applets.
 */ 
class AppletGUI : public Applet, public GUI {
protected:
	/// Pointer to the gnubiff popup
	class Popup						*popup_;
	/// Pointer to the preferences dialog
	class Preferences				*preferences_;
	/** Pointer to the authentication dialog (needed for getting the user id
	 *  and password)
	 */
	class Authentication			*ui_auth_;
	/// Shall the popup be forced on the next update?
	gboolean						force_popup_;
public:
	// ========================================================================
	//  base
	// ========================================================================
	AppletGUI (class Biff *biff, std::string filename, gpointer callbackdata);
	virtual ~AppletGUI (void);
	virtual void start (gboolean showpref = false);

	// ========================================================================
	//  main
	// ========================================================================
	virtual gboolean update (gboolean init = false,
							 std::string widget_image = "",
							 std::string widget_text = "",
							 std::string widget_container = "",
							 guint m_width = G_MAXUINT,
							 guint m_height = G_MAXUINT);
	virtual std::string get_number_of_unread_messages (void);

	void mailbox_to_be_replaced (class Mailbox *from, class Mailbox *to);
	virtual void get_password_for_mailbox (class Mailbox *mb);
	virtual gboolean can_monitor_mailboxes (void);

	void show_dialog_preferences (void);
	void hide_dialog_preferences (void);
	gboolean visible_dialog_preferences (void);
	void show_dialog_about (void);
	void hide_dialog_about (void);
	gboolean visible_dialog_popup (void);
};

#endif
