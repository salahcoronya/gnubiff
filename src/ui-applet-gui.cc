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
// File          : $RCSfile: ui-applet-gui.cc,v $
// Revision      : $Revision: 1.10.2.8 $
// Revision date : $Date: 2011/12/28 18:47:33 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#include "gtk_image_animation.h"
#include "ui-applet-gui.h"
#include "ui-authentication.h"
#include "ui-popup.h"
#include "ui-preferences.h"

/**
 *  Constructor.
 *
 *  @param  biff         Pointer to the biff object of the current gnubiff
 *                       session.
 *  @param  filename     Name of the glade file that contains the GUI
 *                       information.
 *  @param  callbackdata Pointer to be passed to the GUI callback functions.
 */
AppletGUI::AppletGUI (Biff *biff, std::string filename, gpointer callbackdata)
          : Applet (biff), GUI (filename)
{
	// Set default values
	widget_max_height_ = G_MAXUINT;
	widget_max_width_ = G_MAXUINT;
	tooltip_widget_ = NULL;

	GUI::create (callbackdata);

	// Create image animation
	GtkImageAnimation *anim = new GtkImageAnimation (GTK_IMAGE(get("image")));
	g_object_set_data (G_OBJECT(get("image")), "_animation_", anim);
	anim->open (biff_->value_string ("newmail_image"));
	anim->start ();

	// Create preferences dialog
	preferences_ = new Preferences (biff_);
	preferences_->create (preferences_);

	// Create popup dialog
	force_popup_ = false;
	popup_ = new Popup (biff_);
	popup_->create (popup_);

	// Create authentication dialog
	ui_auth_ = new Authentication ();

	// Connect signal for close button in about dialog (needed for gtk >=2.10)
	g_signal_connect (get ("gnubiffabout"), "response",
					  G_CALLBACK (gtk_widget_hide), get ("gnubiffabout"));
	// Hide the about dialog when the delete event is signaled
	g_signal_connect (get ("gnubiffabout"), "delete-event",
					  G_CALLBACK (gtk_widget_hide), get ("gnubiffabout"));
}

/// Destructor
AppletGUI::~AppletGUI (void)
{
}

/**
 *  Return pointer to the applet.
 *
 *  @return   pointer to the applet
 */
class AppletGUI * 
AppletGUI::appletgui_ptr (void)
{
	return this;
}

// ============================================================================
//  tools
// ============================================================================
/**
 *  Get the size of the image/animation currently in use for the applet.
 *
 *  @param  widget_image Name of the image's widget.
 *  @param  width        Width of the image (or 0 if there is no image).
 *  @param  height       Height of the image  (or 0 if there is no image).
 *  @return              False if there is currently no image available, true
 *                       otherwise.
 */
gboolean 
AppletGUI::get_image_size (std::string widget_image, guint &width,
						   guint &height)
{
	width = 0;
	height = 0;

	// Get widget (as GObject)
	GObject *widget = G_OBJECT (get (widget_image.c_str ()));
	if (!widget)
		return false;

	// Get animation
	GtkImageAnimation *anim;
	anim = static_cast<GtkImageAnimation *>( g_object_get_data (widget, "_animation_") );
	if (!anim)
		return false;

	// Get image's current size
	width = anim->scaled_width ();
	height = anim->scaled_height ();

	return true;
}

/**
 *  Resize the applet's image if it is currently too large.
 *
 *  @param widget_image Name of the image's widget.
 *  @param max_width    Maximum width of the image (default is G_MAXUINT).
 *  @param max_height   Maximum height of the image (default is G_MAXUINT).
 *  @return             True if the image has been rescaled, false if there
 *                      is no image or if the image was not too large.
 */
gboolean 
AppletGUI::resize_image (std::string widget_image, guint max_width,
						 guint max_height)
{
	guint cur_width = 0, cur_height = 0;

	// Get widget (as GObject)
	GObject *widget = G_OBJECT (get (widget_image.c_str ()));
	if (!widget)
		return false;

	// Get animation
	GtkImageAnimation *anim;
	anim = static_cast<GtkImageAnimation *>( g_object_get_data (widget, "_animation_") );
	if (!anim)
		return false;

	// Get image's current size
	cur_width = anim->scaled_width ();
	cur_height = anim->scaled_height ();

	// Determine new size of the image
	gboolean resize = false;
	if (cur_width > max_width) {
		cur_height = cur_height * max_width / cur_width;
		cur_width  = max_width;
		resize  = true;
	}
	if (cur_height > max_height) {
		cur_width  = cur_width * max_height / cur_height;
		cur_height = max_height;
		resize  = true;
	}

	// Resize the image if needed
	if (resize)
		anim->resize (cur_width, cur_height);
	return true;
}

// ============================================================================
//  main
// ============================================================================
/**
 *  Start the applet.
 *
 *  @param  showpref  If true and supported by the frontend the preferences
 *                    dialog is shown before monitoring starts (the default is
 *                    false).
 */
void 
AppletGUI::start (gboolean showpref)
{
	if (showpref)
		show_dialog_preferences ();
	else {
		update (true, "image", "unread", "fixed");
		show ();
		Applet::start (false);
	}
}

/**
 *  Get the number of unread messages in all mailboxes as a string (with
 *  pango markup for the font).
 *
 *  @return    Number of unread messages as a string.
 */
std::string 
AppletGUI::get_number_of_unread_messages (void)
{
	std::string text;

	text = "<span font_desc=\"" + biff_->value_string ("applet_font") + "\">";
	text+= Applet::get_number_of_unread_messages ();
	text+= "</span>";

	return text;
}

/**
 *  Update the applet status. This includes showing the
 *  image/animation that corresponds to the current status of gnubiff
 *  (no new messages or new messages are present). Also the text with
 *  the current number of new messages is updated. If present a
 *  container widget that contains the widgets for the image and the
 *  text may be updated too. The status of the popup window is updated
 *  (this is not done when this function is called during the
 *  initialization of gnubiff).
 *
 *  @param  init             True if called when initializing gnubiff (the
 *                           default is false).
 *  @param  widget_image     Name of the widget that contains the image for
 *                           gnubiff's status or the empty string if
 *                           no image shall * be updated. The default
 *                           is the empty string.
 *  @param  widget_text      Name of the widget that contains the text for
 *                           gnubiff's status or the empty string if
 *                           no text shall be updated. The default is
 *                           the empty string.
 *  @param  widget_container Name of the widget that contains the image and
 *                           text widget. If it's an empty string the container
 *                           widget (if present) will not be updated. The
 *                           default is the empty string.
 *  @return                  True if new messages are present
 */
gboolean 
AppletGUI::update (gboolean init, std::string widget_image,
				   std::string widget_text, std::string widget_container)
{
	// Update applet's status: GUI-independent things to do
	gboolean newmail = Applet::update (init);

	// Get number of unread messages
	guint unread;
	biff_->get_number_of_unread_messages (unread);

	// Update popup
	if (!init && (popup_)) {
		// If there are no mails to display then hide popup
		if (!unread && (biff_->value_bool ("use_popup") || force_popup_))
			hide_dialog_popup ();

		// Update and display the popup
		if (unread && ((biff_->value_bool ("use_popup")) || force_popup_)
			&& (newmail || visible_dialog_popup () || force_popup_))
			show_dialog_popup ();
	}

	// Update applet's image
	GtkWidget *widget = NULL;
	guint i_height = 0, i_width = 0;
	if (widget_image != "") {
		GtkImageAnimation *anim;
		widget = get (widget_image.c_str ());
		anim = static_cast<GtkImageAnimation *>( g_object_get_data (G_OBJECT (widget),
												"_animation_") );

		// Determine image
		std::string image;
		if ((unread == 0) && biff_->value_bool ("use_nomail_image"))
			image = biff_->value_string ("nomail_image");
		if ((unread >  0) && biff_->value_bool ("use_newmail_image"))
			image = biff_->value_string ("newmail_image");

		// Show/hide image
		if (image != "") {
			anim->open (image);
			// Resize image (if needed)
			resize_image (widget_image, widget_max_width_, widget_max_height_);
			// Get image's size
			get_image_size (widget_image, i_width, i_height);

			// Start animation in updated widget
			gtk_widget_set_size_request (widget, i_width, i_height);
			gtk_widget_show (widget);
			anim->start();
		}
		else// Hide widget
			gtk_widget_hide (widget);
	}

	// Update applet's text
	GtkLabel *label = NULL;
	guint t_height = 0, t_width = 0;
	if (widget_text != "") {
		label = GTK_LABEL (get (widget_text.c_str ()));

		// Set label
		std::string text = get_number_of_unread_messages ();
		gtk_label_set_markup (label, text.c_str());

		if (((unread == 0) && biff_->value_bool ("use_nomail_text")) ||
			((unread >  0) && biff_->value_bool ("use_newmail_text"))) {
			// Show widget	
			gtk_widget_show (GTK_WIDGET (label));

			// Resize label
			gtk_widget_set_size_request (GTK_WIDGET (label), -1, -1);
			GtkRequisition req;
			gtk_widget_size_request (GTK_WIDGET (label), &req);
			t_width = req.width;
			t_height = req.height;
		}
		else// Hide widget
			gtk_widget_hide (GTK_WIDGET (label));
	}

	// Update the container widget
	guint c_width = 0, c_height = 0;
	if (widget_container != "") {
		GtkFixed *fixed = GTK_FIXED (get (widget_container.c_str ()));

		// Calculate size of container: Image and text should fit into it.
		c_width = std::max (i_width, t_width);
		c_height = std::max (i_height, t_height);

		// Resize container and move widgets inside it to the right position
		if ((c_width > 0) && (c_height > 0)) {
			gtk_widget_set_size_request (GTK_WIDGET(fixed), c_width, c_height);
			if (label)
				gtk_fixed_move (fixed, GTK_WIDGET (label), (c_width-t_width)/2,
								c_height-t_height);
			if (widget)
				gtk_fixed_move (fixed, widget, (c_width-i_width)/2, 0);
		}
	}

	// Reset the force popup boolean
	force_popup_ = false;
	return newmail;
}

/**
 *  Show the preferences dialog. Monitoring of the mailboxes will be stopped.
 */
void 
AppletGUI::show_dialog_preferences (void)
{
	// Hide the popup window
	if (popup_)
		popup_->hide();

	// Show the dialog
	if (preferences_)
		preferences_->show();

	// Stop monitoring mailboxes
	biff_->stop_monitoring ();
}

/**
 *  Hide the preferences dialog. Monitoring of the mailboxes will be started
 *  again (if automatic checking is enabled).
 */
void 
AppletGUI::hide_dialog_preferences (void)
{
	// Hide the preferences dialog
	if (preferences_)
		preferences_->hide();

	// Start monitoring of the mailboxes (if wanted)
	if (biff_->value_uint ("check_mode") == AUTOMATIC_CHECK)
		biff_->start_monitoring (3);

	// Update applet's status
	update (true, "image", "unread", "fixed");
	show();
}

/**
 *  Is the preferences dialog visible?
 *
 *  @return    True, if the preferences dialog is visible, false otherwise.
 */
gboolean 
AppletGUI::visible_dialog_preferences (void)
{
	return (preferences_ && gtk_widget_get_visible (preferences_->get ()));
}

/**
 *  Show the about dialog.
 */
void 
AppletGUI::show_dialog_about (void)
{
	// Hide the other dialogs
	if (popup_)
		popup_->hide ();
	if (preferences_)
		preferences_->hide ();

	// Show the dialog
	GUI::show ("gnubiffabout");
}

/**
 *  Hide the about dialog.
 */
void 
AppletGUI::hide_dialog_about (void)
{
	GUI::hide ("gnubiffabout");
}

/**
 *  Show the popup dialog.
 */
void 
AppletGUI::show_dialog_popup (void)
{
	if (popup_) {
		// First hide and update the popup
		hide_dialog_popup ();
		guint num =	popup_->update ();

		// Show it only if there is at least one header
		if (num)
			popup_->show();
	}
}

/**
 *  Hide the popup dialog.
 */
void 
AppletGUI::hide_dialog_popup (void)
{
	if (popup_)
		popup_->hide ();
}

/**
 *  Is the popup dialog visible?
 *
 *  @return    True, if the popup dialog is visible, false otherwise.
 */
gboolean 
AppletGUI::visible_dialog_popup (void)
{
	return (popup_ && gtk_widget_get_visible (popup_->get ("dialog")));
}

/**
 *  This function is called when the mailbox {\em from} is being replaced by
 *  the mailbox {\em to}.
 *
 *  If necessary the selection in the preferences dialog will be updated.
 *
 *  @param  from Mailbox to be replaced
 *  @param  to   Mailbox that replaces the mailbox {\em from}
 */
void 
AppletGUI::mailbox_to_be_replaced (class Mailbox *from, class Mailbox *to)
{
	// Is the to be deleted mailbox selected in the preferences dialog?
	if ((preferences_) && (preferences_->selected() == from))
		preferences_->selected (to);
}

/**
 *  The return value indicates whether the applet wants the mailboxes to be
 *  monitored.
 *
 *  If the preferences dialog is open, then there should be no monitoring.
 *
 *  @return true, if the applet thinks monitoring the mailboxes is okay
 */
gboolean 
AppletGUI::can_monitor_mailboxes (void)
{
	return !(visible_dialog_preferences ());
}

/**
 *  Ask the user for the user id and password of the mailbox {\em mb}.
 *
 *  @param  mb Mailbox
 */
void 
AppletGUI::get_password_for_mailbox (Mailbox *mb)
{
	ui_auth_->select (mb);
}

/**
 *  Attach a tooltip to an applet's widget. This tooltip contains information
 *  about the status of the applet's mailboxes.
 *
 *  @param  widget  Applet's widget that shall get the tooltip.
 */
void 
AppletGUI::tooltip_update (void)
{
	if (!tooltip_widget_)
		return;

	// Get text for tooltip
	std::string text = get_mailbox_status_text ();

	// Put text in tooltip
    gtk_widget_set_tooltip_text (tooltip_widget_, text.c_str());
}


/**
 *  Enable or disable the popup dialog. The display is updated according to
 *  the intended action.
 *
 *  @param  enable Boolean that indicates whether to enable (if true) or
 *                 disable (if false) the popup.
 */
void 
AppletGUI::enable_popup (gboolean enable)
{
	// Is there no change in the value?
	if (biff_->value_bool ("use_popup") == enable)
		return;

	// Change the value
	biff_->value ("use_popup", enable);

	// Update the preferences dialog
	if (visible_dialog_preferences ())
		preferences_->synchronize ();

	// Update the popup dialog
	if (!enable)
		hide_dialog_popup ();
}
