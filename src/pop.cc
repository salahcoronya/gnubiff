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
// File          : $RCSfile: pop.cc,v $
// Revision      : $Revision: 1.11 $
// Revision date : $Date: 2005/01/03 20:32:21 $
// Author(s)     : Nicolas Rougier
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#include <sstream>
#include <sys/stat.h>
#include <utime.h>

#include "ui-applet.h"
#include "ui-popup.h"
#include "pop.h"
#include "socket.h"
#include "nls.h"

// ========================================================================
//  base
// ========================================================================	
Pop::Pop (Biff *biff) : Mailbox (biff)
{
	socket_ = new Socket (this);
}

Pop::Pop (const Mailbox &other) : Mailbox (other)
{
	socket_ = new Socket (this);
}

Pop::~Pop (void)
{
}

// ========================================================================
//  main
// ========================================================================	
void
Pop::threaded_start (guint delay)
{
	stopped_ = false;

	// Is there already a timeout ?
	if (timetag_)
		return;

	// Do we want to start using given delay ?
	if (delay)
		timetag_ = g_timeout_add (delay*1000, start_delayed_entry_point, this);
	//  or internal delay ?
	else
		timetag_ = g_timeout_add (delay_*1000, start_delayed_entry_point, this);
}

void
Pop::start (void)
{
	if (!g_mutex_trylock (monitor_mutex_))
		return;
	fetch ();
	g_mutex_unlock (monitor_mutex_);

	threaded_start (delay_);
}

void
Pop::fetch (void)
{
	try {
		fetch_status();
		if ((status_ == MAILBOX_NEW) || (status_ == MAILBOX_EMPTY))
			fetch_header();
	}
	catch (pop_err& err) {
		// Catch all errors that are un-recoverable and result in
		// closing the connection, and resetting the mailbox status.
#if DEBUG
		g_warning ("[%d] Pop exception: %s", uin_, err.what());
#endif
		status_ = MAILBOX_ERROR;
		unread_.clear ();
		seen_.clear ();
		socket_->close ();
	}

	if (!GTK_WIDGET_VISIBLE (biff_->popup()->get())) {
		gdk_threads_enter();
		biff_->applet()->update();
		gdk_threads_leave();
	}
}

void
Pop::fetch_status (void)
{
	std::string line;
	std::vector<std::string> buffer;
	status_ = MAILBOX_CHECK;

	// Connection and authentification
	if (!connect())	return;

	// Get total number of messages into total
	sendline ("STAT");
	readline (line);

	guint total;
	sscanf (line.c_str()+4, "%ud\n", &total);

	// We want to retrieve a maximum of _max_collected_mail uidl
	// so we have to check  total number and find corresponding
	// starting index (start).
	guint n;
	guint start;
	if (total > biff_->max_mail_) {
		n = biff_->max_mail_;
		start = 1 + total -  biff_->max_mail_;
	}
	else {
		n = total;
		start = 1;
	}

	// Retrieve uidl one by one to avoid to get all of them
	buffer.clear();
	char uidl[71];
	guint dummy;
	for (guint i=0; i< n; i++) {
		std::stringstream s;
		s << (i+start);
		sendline ("UIDL " + s.str());
		readline (line);
		sscanf (line.c_str()+4, "%ud %70s\n", &dummy, (char *) &uidl);
		buffer.push_back (uidl);
	}

	// Determine new mailbox status
	if (buffer.empty())
		status_ = MAILBOX_EMPTY;
	else if (contains_new<std::string>(buffer, saved_))
		status_ = MAILBOX_NEW;
	else
		status_ = MAILBOX_OLD;
	saved_ = buffer;

	// QUIT
	sendline ("QUIT");
	readline (line, true, true, false);

	socket_->close();
}


void
Pop::fetch_header (void)
{
	std::string line;
	static std::vector<std::string> buffer;
	int saved_status = status_;
  
	// Status will be restored in the end if no problem occured
	status_ = MAILBOX_CHECK;

	// Connection and authentification
	if (!connect()) return;

	// STAT
	sendline ("STAT");
	readline (line);

	guint total;
	sscanf (line.c_str()+4, "%ud", &total);

	// We want to retrieve a maximum of _max_collected_mail 
	// so we have to check total number and find corresponding
	// starting index (start).
	guint n;
	guint start;
	if (total > biff_->max_mail_) {
		n = biff_->max_mail_;
		start = 1 + total - biff_->max_mail_;
	}
	else {
		n = total;
		start = 1;
	}

	// Fetch mails
	new_unread_.clear();
	new_seen_.clear();
	std::vector<std::string> mail;
	for (guint i=0; i < n; i++) {
		std::stringstream s;
		s << (i+start) << " " << bodyLinesToBeRead_;
		mail.clear();
		// Get header and first lines of mail (see constant bodyLinesToBeRead_)
		sendline ("TOP " + s.str());
		readline (line, false); // + OK response to TOP
#ifdef DEBUG
		g_print ("** Message: [%d] RECV(%s:%d): (message) ", uin_,
				 address_.c_str(), port_);
#endif

		gint cnt = preventDoS_headerLines_ + bodyLinesToBeRead_ + 1;
		do {
			readline (line, false, true, false);
			if (line.size() > 1) {
				mail.push_back (line.substr(0, line.size()-1));
#ifdef DEBUG
				g_print ("+");
#endif
			}
			else
				mail.push_back ("");
		} while ((line != ".\r") && (cnt--));
		if (cnt < 0) throw pop_dos_err();
#ifdef DEBUG
		g_print("\n");
#endif
		mail.pop_back();
		parse (mail, MAIL_UNREAD);
	}
	
	// QUIT
	sendline ("QUIT");
	readline (line, true, true, false);

	socket_->close();

	// Restore status
	status_ = saved_status;

	// Last check for mailbox status
	if ((unread_ == new_unread_) && (unread_.size() > 0))
		status_ = MAILBOX_OLD;

	unread_ = new_unread_;
	seen_ = new_seen_;
}

/**
 * Send a line to the POP server.
 * The given {\em line} is postfixed with "\r\n" and then written to
 * the socket of the mailbox.
 *
 * If {\em check} is true the return value of the call to Socket::write() is
 * checked and an pop_socket_err exception is thrown if it was not successful.
 * So this function always returns SOCKET_STATUS_OK if {\em check} is true,
 * otherwise (if {\em check} is false) error handling is left to the caller of
 * this function.
 *
 * @param line     line to be sent
 * @param print    Shall the sent command be printed in debug mode?
 *                 The default is true.
 * @param check    Shall the return value of the Socket::write() command be
 *                 checked? The default is true.
 * @return         Return value of the Socket::write() command, this is always
 *                 SOCKET_STATUS_OK if {\em check} is true.
 * @exception pop_socket_err
 *                 This exception is thrown if a network error occurs.
 */
gint 
Pop::sendline (std::string line, gboolean print, gboolean check)
			   throw (pop_err)
{
	gint status=socket_->write (line + "\r\n", print);
	if ((status!=SOCKET_STATUS_OK) && check) throw pop_socket_err();
	return status;
}

/**
 * Read one line from the server. If {\em check} is true the return value of
 * the call to Socket::read() is checked and a pop_socket_err exception is
 * thrown if it was not successful. So this function always returns
 * SOCKET_STATUS_OK if {\em check} is true, otherwise (if {\em check} is
 * false) error handling is left to the caller of this function.
 *
 * If {\em checkline} is true then the read line is checked for an negative
 * response ("-ERR"). If such a response is found an error message is printed,
 * the "QUIT" command is sent (see remark below) and a pop_command_err
 * exception is thrown.
 *
 * Remark: The parameter {\em checkline} must be false if reading the response
 * to the "QUIT" command.
 *
 * @param line      String that contains the read line if the call was
 *                  successful (i.e. the return value is SOCKET_STATUS_OK),
 *                  the value is undetermined otherwise
 * @param print     Shall the read line be printed in debug mode?
 *                  The default is true.
 * @param check     Shall the return value of the Socket::read() command be
 *                  checked? The default is true.
 * @param checkline Shall {\em line} be checked for an error response?
 *                  The default is true.
 * @return          Return value of the Socket::read() command, this is always
 *                  SOCKET_STATUS_OK if {\em check} is true.
 * @exception pop_command_err
 *                  This exception is thrown if {\em line} contains a negative
 *                  response and {\em checkline} is true.
 * @exception pop_socket_err
 *                  This exception is thrown if a network error occurs.
 */
gint 
Pop::readline (std::string &line, gboolean print, gboolean check,
			   gboolean checkline) throw (pop_err)
{
	// Read line
	gint status=socket_->read(line, print, check);
	if (check && (status!=SOCKET_STATUS_OK)) throw pop_socket_err();

	// Only "+OK" and "-ERR" are valid responses (see RFC 1939 3.)
	if (!checkline)
		return status;
	if (line.find ("-ERR") == 0) {
		g_warning (_("[%d] Error message from POP3 server:%s"), uin_,
					 line.substr(4,line.size()-4).c_str());
		// We are still able to QUIT
		sendline ("QUIT");
		readline (line, true, true, false);
		throw pop_command_err();
	}
	if (line.find ("+OK") != 0) {
		g_warning (_("[%d] Did not get a positive response from POP3 server"),
				   uin_);
		throw pop_command_err();
	}
	return status;
}
