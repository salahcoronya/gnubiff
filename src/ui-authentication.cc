// ========================================================================
// gnubiff -- a mail notification program
// Copyright (c) 2000-2011 Nicolas Rougier, 2004-2011 Robert Sowada
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
// File          : $RCSfile: ui-authentication.cc,v $
// Revision      : $Revision: 1.4.2.1 $
// Revision date : $Date: 2007/09/08 14:57:58 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#include "support.h"

#include <sstream>

#include "ui-authentication.h"
#include "mailbox.h"


Authentication::Authentication (void) : GUI (GNUBIFF_DATADIR"/authentication.ui")
{
	mailbox_ = 0;
	access_mutex_ = g_mutex_new ();
}

Authentication::~Authentication (void)
{
	GtkWidget *widget = get("dialog");
	if (GTK_IS_WIDGET (widget)) {
		hide ();
		gtk_widget_destroy (widget);
	}

	if (gtkbuilder_)
		g_object_unref (G_OBJECT(gtkbuilder_));
    gtkbuilder_ = NULL;
}

void
Authentication::select (Mailbox *mailbox)
{
	if (mailbox) {
		g_mutex_lock (access_mutex_);
		mailbox_ = mailbox;
		show ();
	}
}

void
Authentication::show (std::string name)
{
	if (!gtkbuilder_)
		create(this);

	// Try to identify mailbox by:
	// 1. using name
	std::string id = mailbox_->name();

	// 2. using hostname
	if (id.empty())
		id = mailbox_->address();

	// 3. using mailbox uin
	if (id.empty()) {
		std::stringstream s;
		s << mailbox_->uin();
		id = s.str();
	}

	gchar *text = g_strdup_printf (_("Please enter your username and password for mailbox '%s'"), id.c_str());
	gtk_label_set_text (GTK_LABEL(get("label")), text);
	g_free (text);
	gtk_entry_set_text (GTK_ENTRY (get("username_entry")), mailbox_->username().c_str());
	gtk_entry_set_text (GTK_ENTRY (get("password_entry")), mailbox_->password().c_str());
	gtk_widget_show_all (get("dialog"));
	gtk_main ();
}

void
Authentication::on_ok (GtkWidget *widget)
{
	mailbox_->username (gtk_entry_get_text (GTK_ENTRY (get("username_entry"))));
	mailbox_->password (gtk_entry_get_text (GTK_ENTRY (get("password_entry"))));
	hide();
	g_mutex_unlock (access_mutex_);
	gtk_main_quit ();
}

void
Authentication::on_cancel (GtkWidget *widget)
{
	hide();
	g_mutex_unlock (access_mutex_);
	gtk_main_quit ();
}

gboolean
Authentication::on_destroy (GtkWidget *widget,
							GdkEvent *event)
{
	hide();
	g_mutex_unlock (access_mutex_);
	gtk_main_quit ();
	return TRUE;
}

gboolean
Authentication::on_delete	(GtkWidget *widget,
							 GdkEvent *event)
{
	hide();
	g_mutex_unlock (access_mutex_);
	gtk_main_quit ();
	return TRUE;
}
