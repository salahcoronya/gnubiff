// ========================================================================
// gnubiff -- a mail notification program
// Copyright (c) 2000-2004 Nicolas Rougier
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
// File          : $RCSfile: ui-preferences.h,v $
// Revision      : $Revision: 1.3 $
// Revision date : $Date: 2005/01/31 14:58:22 $
// Author(s)     : Nicolas Rougier
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __UI_PREFERENCES_H__
#define __UI_PREFERENCES_H__

#include "gui.h"
#include "ui-properties.h"

enum {
  COLUMN_UIN,
  COLUMN_MAILBOX_STOCK_ID,
  COLUMN_SSL_ICON,
  COLUMN_MAILBOX,
  COLUMN_FORMAT,
  COLUMN_STATUS_STOCK_ID,
  COLUMN_SECURITY_STOCK_ID,
  N_COLUMNS
};

enum { COL_EXP_ID, COL_EXP_NAME, COL_EXP_GROUPNAME, COL_EXP_TYPE,
	   COL_EXP_VALUE, COL_EXP_N};

#define PREFERENCES(x)	((Preferences *)(x))


class Preferences : public GUI {

protected:
	class Biff *		biff_;			// biff
	class Mailbox *		selected_;		// current selected mailbox
	class Mailbox *		added_;			// last added mailbox
	class Properties *	properties_;	// properties ui


public:
	/**
	 * Base
	 **/
	Preferences (class Biff *biff);
	~Preferences (void);
	gint create (void);

	/**
	 * Main
	 **/
	void show (std::string name = "dialog");
	void hide (std::string name = "dialog");
	void synchronize (void);
	void synchronize (class Mailbox *mailbox, GtkListStore *store, GtkTreeIter *iter);
	void apply (void);

	/**
	 * Access   
	 **/
	class Properties *properties (void)		{return properties_;}
	class Biff * biff (void) 				{return biff_;}
	class Mailbox *added (void)				{return added_;}
	void added (class Mailbox *mailbox)		{added_ = mailbox;}
	class Mailbox *selected (void)			{return selected_;}
	void selected (class Mailbox *mailbox)	{
		selected_ = mailbox;
		if (properties_)
			properties_->select (mailbox);
	}

	/**
	 * Callbacks
	 **/
	void on_add					(GtkWidget *widget);
	void on_remove				(GtkWidget *widget);
	void on_properties			(GtkWidget *widget);
	void on_stop				(GtkWidget *widget);
	void on_close				(GtkWidget *widget);
	void on_browse_newmail_image(GtkWidget *widget);
	void on_browse_nomail_image	(GtkWidget *widget);
	void on_selection			(GtkTreeSelection *selection);
	void on_check_changed		(GtkWidget *widget);
	gboolean on_destroy			(GtkWidget *widget,
								 GdkEvent *event);
	gboolean on_delete			(GtkWidget *widget,
								 GdkEvent *event);

	/**
	 * Expert dialog
	 **/
	void expert_create (void);
	void expert_add_option_list (void);
	void expert_on_selection (GtkTreeSelection *selection);
};

#endif
