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
// File          : $RCSfile: ui-authentication.h,v $
// Revision      : $Revision: 1.4 $
// Revision date : $Date: 2007/09/08 18:06:19 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __UI_AUTHENTICATION_H__
#define __UI_AUTHENTICATION_H__

#include "gui.h"

#define AUTHENTICATION(x)	(static_cast<Authentication *>(x))


class Authentication : public GUI {

protected:
	class Mailbox *	mailbox_;
	GMutex *		access_mutex_;

public:
	/* base */
	Authentication (void);
	~Authentication (void);

	/* main */
	void select (class Mailbox *mailbox);
	void show (std::string name = "dialog");

	/* callbacks */
	void on_ok			(GtkWidget *widget);
	void on_cancel		(GtkWidget *widget);
	gboolean on_destroy (GtkWidget *widget,
						 GdkEvent *event);
	gboolean on_delete	(GtkWidget *widget,
						 GdkEvent *event);	
};

#endif
