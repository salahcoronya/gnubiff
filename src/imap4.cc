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
// File          : $RCSfile: imap4.cc,v $
// Revision      : $Revision: 1.46 $
// Revision date : $Date: 2004/12/27 00:00:22 $
// Author(s)     : Nicolas Rougier
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#include "support.h"

#include <string>
#include <sstream>
#include <glib.h>
#include <sys/stat.h>
#include <utime.h>

#include "ui-authentication.h"
#include "ui-applet.h"
#include "ui-popup.h"
#include "imap4.h"
#include "nls.h"

// ========================================================================
//  base
// ========================================================================	
Imap4::Imap4 (Biff *biff) : Mailbox (biff)
{
	protocol_ = PROTOCOL_IMAP4;
	socket_   = new Socket (this);
	idleable_ = false;
	idled_    = false;
}

Imap4::Imap4 (const Mailbox &other) : Mailbox (other)
{
	protocol_ = PROTOCOL_IMAP4;
	socket_   = new Socket (this);
	idleable_ = false;
	idled_    = false;
}

Imap4::~Imap4 (void)
{
	delete socket_;
}

// ========================================================================
//  main
// ========================================================================	
void
Imap4::threaded_start (guint delay)
{
	// Is there already a timeout ?
	if (timetag_)
		return;

	// Are we in idle state ?
	if (idled_)
		return;

	// Do we want to start using given delay ?
	if (delay) {
		timetag_ = g_timeout_add (delay*1000, start_delayed_entry_point, this);
#if DEBUG
		g_message ("[%d] Start fetch in %d second(s)", uin_, delay);
#endif
	}
	//  or internal delay ?
	else {
		timetag_ = g_timeout_add (delay_*1000, start_delayed_entry_point, this);
#if DEBUG
		g_message ("[%d] Start fetch in %d second(s)", uin_, delay_);
#endif
	}
}

void
Imap4::start (void)
{
	if (!g_mutex_trylock (monitor_mutex_))
		return;
	fetch ();
	g_mutex_unlock (monitor_mutex_);

	threaded_start (delay_);
}

void
Imap4::fetch (void)
{
	fetch_status();
	if ((status_ == MAILBOX_NEW) || (status_ == MAILBOX_EMPTY) || (idleable_))
		fetch_header();

	if (!GTK_WIDGET_VISIBLE (biff_->popup()->get())) {
		gdk_threads_enter();
		biff_->applet()->update();
		gdk_threads_leave();

		// If we have reported the new mail, then set the status to old
		if (status_ == MAILBOX_NEW)
			status_ = MAILBOX_OLD;
	}
}

gint Imap4::connect (void)
{
	std::string line;


	// Check standard port
	if (!use_other_port_)
		if (authentication_ == AUTH_USER_PASS)
			port_ = 143;
		else
			port_ = 993;

#ifdef DEBUG
	g_message ("[%d] Trying to connect to %s on port %d", uin_, address_.c_str(), port_);
#endif

	// connection
	if (authentication_ == AUTH_AUTODETECT) {
		guint port = port_;
		if (!use_other_port_)
			port = 993;
		if (!socket_->open (address_, port, AUTH_SSL)) {
			if (!use_other_port_)
				port = 143;
			if (!socket_->open (address_, port, AUTH_USER_PASS)) {
				status_ = MAILBOX_ERROR;
				return 0;
			}
			else {
				port_ = port;
				authentication_ = AUTH_USER_PASS;
				socket_->close();
			}
		}
		else {
			port_ = port;
			authentication_ = AUTH_SSL;
			socket_->close();
		}
	}

	if (!socket_->open (address_, port_, authentication_, certificate_, 3)) {
		status_ = MAILBOX_ERROR;
		return 0;
	}


#ifdef DEBUG
	g_message ("[%d] Connected to %s on port %d", uin_, address_.c_str(), port_);
#endif
	if (!(socket_->read (line, true))) return 0;

	reset_tag();

	// Get the IDLE capacity
	line = "CAPABILITY";
	if (!send (line, false)) return 0;
	if (!(socket_->read (line))) return 0;
	if (line.find ("IDLE") != std::string::npos)
		idleable_ = true;

	if (!(socket_->read (line))) return 0;
	if (line.find (tag()+"OK") != 0) {
		socket_->status (SOCKET_STATUS_ERROR);
		status_ = MAILBOX_ERROR;
		g_warning (_("[%d] Unable to get acknowledgment from %s on port %d"), uin_, address_.c_str(), port_);
		return 0;
	}


	// LOGIN
	line = "LOGIN \"" + username_ + std::string ("\" \"") + password_+ "\"";
	if (!send (line, false)) return 0;

	// Just in case send someone me the output: password won't be displayed
#ifdef DEBUG
	line = tag() + "LOGIN \"" + username_ + "\" (password) \r\n";
	g_message ("[%d] SEND(%s:%d): %s", uin_, address_.c_str(), port_,
			   line.c_str());
#endif

	if (!(socket_->read (line))) return 0;
	if (line.find (tag()+"OK") != 0) {
		socket_->status (SOCKET_STATUS_ERROR);
		status_ = MAILBOX_ERROR;
		g_warning (_("[%d] Unable to get acknowledgment from %s on port %d"), uin_, address_.c_str(), port_);
		return 0;
	}

	// SELECT
	gboolean check = false;
	gchar *folder_imaputf7=utf8_to_imaputf7(folder_.c_str(),-1);
	if (folder_imaputf7)
	{
		line=std::string("SELECT \"") + folder_imaputf7 + "\"";
		g_free(folder_imaputf7);
		if (!send(line)) return 0;

		// We need to set a limit to lines read (DoS Attacks).
		// According to RFC 3501 6.3.1 there must be exactly seven lines
		// before the "Axxx OK ..." line.
		gint cnt=8+preventDoS_additionalLines_;
		while ((socket_->read (line)) && (cnt--))
		{
			if (line.find (tag()+"OK") == 0)
			{
				check = true;
				break;
			}
			else if (line.find (tag()) == 0)
			{
				if (send("LOGOUT"))
					socket_->close ();
				break;
			}
		}
	}
	else
	{
		if (send("LOGOUT"))
			socket_->close ();
	}

	if (!socket_->status()||!check||!folder_imaputf7)
	{
		socket_->status (SOCKET_STATUS_ERROR);
		status_ = MAILBOX_ERROR;
		g_warning (_("[%d] Unable to select folder %s on host %s"),
				   uin_, folder_.c_str(), address_.c_str());
		return 0;
	}

	if (check)
		socket_->status(SOCKET_STATUS_OK);

	return 1;
}


void
Imap4::fetch_status (void)
{
	std::string line;
	std::vector<int> buffer;
  
	// By default we consider to have an error status
	status_ = MAILBOX_CHECK;

	// First try: try to guess password by looking at other mailboxes
	// Remark: It's important to block thread before looking at other mailboxes
	//         since one is maybe asking (using gui) for this password.
	if (password_.empty()) {
		g_static_mutex_lock (&ui_auth_mutex_);

		password_ = biff_->password (this);

		// Second try: ask to the user
		if (password_.empty()) {
			gdk_threads_enter ();
			ui_auth_->select (this);
			gdk_threads_leave ();
		}
		g_static_mutex_unlock (&ui_auth_mutex_);
	}

	// No way, user do not want to help us, we simply return
	if (password_.empty()) {
		status_ = MAILBOX_ERROR;
		return;
	}

	// Connection and authentification
	if (!connect ()) {
		status_ = MAILBOX_ERROR;
		return;
	}

	// SEARCH NOT SEEN
	if (!send ("SEARCH NOT SEEN")) return;

	// We need to set a limit to lines read (DoS Attacks).
	// Expected response "* SEARCH ..." should be in the next line.
	gint cnt=1+preventDoS_additionalLines_;
	while (((socket_->read(line) > 0)) && (cnt--))
		if (line.find ("* SEARCH") == 0)
			break;
	if ((!socket_->status()) || (cnt<0)) return;


	// Parse server answer
	// Should be something like
	// "* SEARCH 1 2 3 4" or "* SEARCH"
	// (9 is size of "* SEARCH ")
	buffer.clear();
	if (line.size() > 9) {
		line = line.substr (9);
		int n = 0;
		for (guint i=0; i<line.size(); i++) {
			if (line[i] >= '0' && line[i] <= '9')
				n = n*10 + int(line[i]-'0');
			else {
				buffer.push_back (n);
				n = 0;
			}
		}
	}

	// Determine new mailbox status
	if (buffer.empty())
		status_ = MAILBOX_EMPTY;
	else if (contains_new<int>(buffer, saved_))
		status_ = MAILBOX_NEW;
	else
		status_ = MAILBOX_OLD;
	saved_ = buffer;

	// We're done
	cnt=1+preventDoS_additionalLines_;
	while ((socket_->read (line) > 0) && (cnt--))
		if (line.find (tag()) != std::string::npos)
			break;
	if ((!socket_->status()) || (cnt<0)) return;

	// Closing connection
	if (!send("LOGOUT")) return;
	socket_->close ();
}

void
Imap4::fetch_header (void)
{
	std::string line;
	std::vector<int> buffer;
	int saved_status = status_;
	gboolean idling = true;
  
	// Status will be restored in the end if no problem occured
	status_ = MAILBOX_CHECK;

	// Check password is not empty
	if (password_.empty())
		ui_auth_->select (this);

	if (password_.empty()) {
		status_ = MAILBOX_ERROR;
		return;
	}

	// Connection and authentification
	if (!connect ()) {
		status_ = MAILBOX_ERROR;
		return;
	}

	// This loop is for the idle state when we wait for new message
	do {
		// SEARCH NOT SEEN
		if (!send("SEARCH NOT SEEN")) return;

		// We need to set a limit to lines read (DoS Attacks).
		// Expected response "* SEARCH ..." should be in the next line.
		gint cnt=1+preventDoS_additionalLines_;
		while (((socket_->read(line) > 0)) && (cnt--))
			if (line.find ("* SEARCH") == 0)
				break;
		if ((!socket_->status()) || (cnt<0)) return;

		// Parse server answer
		// Should be something like
		// "* SEARCH 1 2 3 4" or "* SEARCH"
		// (9 is size of "* SEARCH ")
		buffer.clear();
		if (line.size() > 9) {
			line = line.substr (9);
			int n = 0;
			for (guint i=0; i<line.size(); i++) {
				if (line[i] >= '0' && line[i] <= '9')
					n = n*10 + int(line[i]-'0');
				else {
					buffer.push_back (n);
					n = 0;
				}
			}
		}

		cnt=1+preventDoS_additionalLines_;
		while ((socket_->read (line) > 0) && (cnt--))
			if (line.find (tag()) != std::string::npos)
				break;
		if ((!socket_->status()) || (cnt<0)) return;


		// FETCH NOT SEEN
		new_unread_.clear();
		new_seen_.clear();
		std::vector<std::string> mail;
		for (guint i=0; (i<buffer.size()) && (new_unread_.size() < (unsigned int)(biff_->max_mail_)); i++) {
			std::stringstream s;
			s << buffer[i];
			mail.clear();

			line="FETCH " + s.str();
			line+=" (BODY.PEEK[HEADER.FIELDS (DATE FROM SUBJECT)])";
			if (!send(line)) return;

			// Response should be: "* s FETCH ..." (see RFC 3501 7.4.2)
			cnt=1+preventDoS_additionalLines_;
			while (((socket_->read(line) > 0)) && (cnt--))
				if (line.find ("* "+s.str()+" FETCH") == 0)
					break;
			if ((!socket_->status()) || (cnt<0)) return;

			// Date, From, Subject
#ifdef DEBUG
			g_print ("** Message: [%d] RECV(%s:%d): (message) ", uin_,
					 address_.c_str(), port_);
#endif
			cnt=5+preventDoS_additionalLines_;
			while (((socket_->read(line, false) > 0)) && (cnt--)) {
				if (line.find (tag()) == 0)
					break;
				if (line.size() > 0) {
					mail.push_back (line.substr(0, line.size()-1));
#ifdef DEBUG
					g_print ("+");
#endif
				}
			}
#ifdef DEBUG
			g_print ("\n");
#endif
			// Did an error happen?
			if ((!socket_->status()) || (cnt<0) || (mail.size()==0)) return;

			// Remove last line (should contain a closing parenthesis). Note:
			// We need the (hopefully empty;-) line before because it separates
			// header and mail text
			mail.pop_back();


			// FETCH BODYSTRUCTURE
			if (!send("FETCH " + s.str() + " (BODYSTRUCTURE)")) return;

			// Response should be: "* s FETCH (BODYSTRUC..." (see RFC 3501 7.4.2)
			cnt=1+preventDoS_additionalLines_;
			while (((socket_->read(line) > 0)) && (cnt--))
				if (line.find ("* "+s.str()+" FETCH (BODYSTRUCTURE (") == 0)
					break;
			if ((!socket_->status()) || (cnt<0)) return;
			if (line.substr(line.size()-2) != ")\r")
				return;

			// Remove first and last part
			line=line.substr(25+s.str().size(),line.size()-28-s.str().size());

			// Get Part of Mail that contains "text/plain" (if any exists) and
			// size of this text, encoding, charset
			PartInfo partinfo;
			parse_bodystructure(line,partinfo);
			gint textsize=partinfo.size;
#ifdef DEBUG
			g_print("** Part %s size=%d, encoding=%s, charset=%s\n",
					partinfo.part.c_str(), partinfo.size,
					partinfo.encoding.c_str(), partinfo.charset.c_str());
#endif

			// Read end of command
			cnt=1+preventDoS_additionalLines_;
			while (((socket_->read(line, false) > 0)) && (cnt--))
				if (line.find (tag()) == 0)
					break;
			if ((!socket_->status()) || (cnt<0)) return;


			// FETCH BODY.PEEK
			// Is there any plain text?
			if (partinfo.part=="")
				mail.push_back(std::string(_("[This mail has no \"text/plain\" part]")));
			else if (textsize == 0)
				mail.push_back(std::string(""));
			else {
				if (partinfo.charset!="")
					mail.insert (mail.begin(), std::string("charset=") +  partinfo.charset + std::string(";"));

				// Note: We are only interested in the first lines, there
				// are at most 1000 characters per line (see RFC 2821 4.5.3.1),
				// so it is sufficient to get at most 1000*bodyLinesToBeRead_
				// bytes.
				if (textsize>1000*bodyLinesToBeRead_)
					textsize=1000*bodyLinesToBeRead_;
				std::stringstream textsizestr;
				textsizestr << textsize;
				line = "FETCH "+s.str()+" (BODY.PEEK["+partinfo.part+"]<0.";
				line+= textsizestr.str() + ">)";
				if (!send(line)) return;

				// Response should be: "* s FETCH ..." (see RFC 3501 7.4.2)
				cnt=1+preventDoS_additionalLines_;
				while (((socket_->read(line) > 0)) && (cnt--))
					if (line.find ("* "+s.str()+" FETCH") == 0)
						break;
				if ((!socket_->status()) || (cnt<0)) return;

#ifdef DEBUG
				g_print ("** Message: [%d] RECV(%s:%d): (message) ", uin_, address_.c_str(), port_);
#endif
				// Read text
				gint lineno=0,bytes=textsize+3; // ")\r\n" at end of mail
				while ((bytes>0) && ((socket_->read(line, false) > 0))) {
					bytes-=line.size()+1; // don't forget to count '\n'!
					if ((line.size() > 0) && (lineno++<bodyLinesToBeRead_)) {
						mail.push_back (line.substr(0, line.size()-1));
#ifdef DEBUG
						g_print ("+");
#endif
					}
				}
				if ((!socket_->status()) || (bytes<0)) return;
				// Remove ")\r" from last line ('\n' was removed before)
				mail.pop_back();
				if (line.size()>1)
					mail.push_back (line.substr(0, line.size()-2));
				// Read end of command
				if (!(socket_->read(line, false))) return;
				if (line.find (tag()+"OK") != 0) return;
			}
#ifdef DEBUG
			g_print ("\n");
#endif
			if (!socket_->status()) return;

			if (partinfo.part!="")
				decode_body (mail, partinfo.encoding);
			parse (mail, MAIL_UNREAD);
		}

		// We restore status
		status_ = saved_status;

		if (contains_new<header>(new_unread_, unread_))
			status_ = MAILBOX_NEW;
		else
			status_ = MAILBOX_OLD;

		unread_ = new_unread_;
		seen_ = new_seen_;

		// Is server idleable?
		if (idleable_) {
			// When in idle state, we won't exit this thread function
			// so we have to, update applet in the meantime
			if (!GTK_WIDGET_VISIBLE (biff_->popup()->get())) {
				gdk_threads_enter();
				biff_->applet()->update();
				gdk_threads_leave();

				// If we have reported the new mail, then set the status to old
				if (status_ == MAILBOX_NEW)
					status_ = MAILBOX_OLD;
			}

			if (timetag_)
				g_source_remove (timetag_);
			timetag_ = 0;
			// Entering idle state
			idled_ = true;
			line = std::string ("IDLE");
			if (!send (line)) break;
		
			// Read acknowledgement
			if (!socket_->read (line)) break;
			
			// Wait for new mail and block thread at this point
			if (!socket_->read (line)) break;

			// Did we lost lock?
			if (line.find ("* BYE") == 0) break;

			line = std::string ("DONE") +std::string ("\r\n");
			if (!socket_->write (line)) idling = false;
		
			// Either we got a OK or a BYE
			cnt=preventDoS_additionalLines_;
			do {
				if (!socket_->read (line)) break;
				// Did we lost lock?
				if (line.find ("* BYE") == 0) break;
			} while ((line.find (tag()+"OK") != 0) && (cnt--));
			if (!cnt)
				break;
		}
		else {
			// Closing connection
			if (!send ("LOGOUT")) break;
			socket_->close ();
			idling = false;
			idled_ = false;
			return;
		}
		if (!socket_->status()) break;
	} while (idling);

	idled_ = false;
	socket_->close ();
}

/** 
 * Parse the body structure of a mail.
 * This function parses the result {\em structure} of a
 * "FETCH ... (BODYSTRUCTURE)" IMAP command. It returns (via the reference
 * parameter {\em partinfo} the part of the mail body containing the first
 * "text/plain" section and information about this part. If no such section
 * exists (or in case of an error) false is returned.
 *
 * @param  structure C++ String containing the result of the IMAP command
 *                   (without "* ... FETCH (BODYSTRUCTURE (" and the trailing
 *                   ')').
 * @param  partinfo  Reference to a PartInfo structure. If true is
 *                   returned the structure will contain the information about
 *                   the selected part (part, size, encoding, charset,
 *                   mimetype)
 * @param  toplevel  Boolean (default value is true). This is true if it is the
 *                   toplevel call of this function, false if it is called
 *                   recursively.
 * @return           C++ String containing the first "text/plain" part or an
 *                   empty string
 */
gboolean 
Imap4::parse_bodystructure (std::string structure, PartInfo &partinfo,
							gboolean toplevel)
{
	gint len=structure.size(),pos=0,block=1,nestlevel=0,startpos=0;
	gboolean multipart=false;

	// Multipart? -> Parse recursively
	if (structure.at(0)=='(')
		multipart=true;

	// Length is in the 7th block:-(
	while (pos<len)
	{
		gchar c=structure.at(pos++);

		// String (FIXME: '"' inside of strings?)
		if (c=='"')
		{
			// When in multipart only the last entry is allowed to be a string
			if ((multipart) && (nestlevel==0))
				return false;
			// Get the string
			gint oldpos=pos;
			while ((pos<len) && (structure.at(pos++)!='"'));
			if ((nestlevel==0) && (!multipart))
			{
				std::string value=structure.substr(oldpos,pos-oldpos-1);
				gchar *lowercase=g_utf8_strdown(value.c_str(),-1);
				value=std::string(lowercase);
				g_free(lowercase);
				switch (block)
				{
					case 1: // MIME type
						if (value!="text")
							return false;
						partinfo.mimetype=value;
						break;
					case 2: // MIME type
						if (value!="plain")
							return false;
						partinfo.mimetype+="/"+value;
						break;
					case 6:	// Encoding
						partinfo.encoding=value;
						break;
				}
			}
			continue;
		}

		// Next block
		if (c==' ')
		{
			if (nestlevel==0)
				block++;
			if ((block>7) && (!multipart))
				return false;
			while ((pos<len) && (structure.at(pos)==' '))
				pos++;
			continue;
		}

		// Nested "( ... )" block begins
		if (c=='(')
		{
			if (nestlevel==0)
				startpos=pos-1;
			nestlevel++;
			continue;
		}

		// Nested "( ... )" block ends
		if (c==')')
		{
			nestlevel--;
			if (nestlevel<0)
				return false;
			// Content of block
			std::string content=structure.substr(startpos+1,pos-startpos-2);
			// One part of a multipart message?
			if ((nestlevel==0) && (multipart))
			{
				if (!parse_bodystructure(content,partinfo,false))
					continue;
				std::stringstream ss;
				ss << block;
				if (toplevel)
					partinfo.part=ss.str();
				else
					partinfo.part=ss.str()+std::string(".")+partinfo.part;
				return true;
			}
			// List of parameter/value pairs? (3rd block)
			if ((nestlevel==0) && (!multipart) && (block==3))
				if (!parse_bodystructure_parameters(content,partinfo))
					return false;
			continue;
		}

		// Alphanumerical character
		if (g_ascii_isalnum(c))
		{
			if ((multipart) && (nestlevel==0))
				return false;
			if (!multipart)
				startpos=pos-2;
			while ((pos<len) && (g_ascii_isalnum(structure.at(pos))))
				pos++;
			// Block with size information?
			if ((block==7) && (nestlevel==0) && (!multipart))
			{
				std::stringstream ss;
				ss << structure.substr(startpos,pos-startpos).c_str();
				ss >> partinfo.size;
				partinfo.part=std::string("1");
				return true;
			}
			continue;
		}

		// Otherwise: Error!
		return false;
	}
	// At end and no length found: Error!
	return false;
}

/** 
 * Parse the list of parameter/value pairs of a part of a mail.
 * This list is the third block of the body structure of this part. Currently
 * the only parameter we are interested in is the character set.
 * If the parameter is not in the list an empty string is returned as value for
 * this parameter.
 *
 * Note: Because we converted to lower case before the values of certain
 * parameters are not very useful (e.g. filenames).
 *
 * @param  list      C++ String containing the parameter/value list. This is a
 *                   (converted to lower case) substring of the result of the
 *                   IMAP "FETCH ... (BODYSTRUCTURE) command).
 * @param  partinfo  Reference to a PartInfo structure. If true is
 *                   returned the structure will contain the information about
 *                   the selected part (part, size, encoding, charset,
 *                   mimetype)
 * @return           Boolean indicating success or failure.
 */
gboolean 
Imap4::parse_bodystructure_parameters (std::string list, PartInfo &partinfo)
{
	gint len=list.size(),pos=0,stringcnt=1;
	std::string parameter;

	while (pos<len)
	{
		gchar c=list.at(pos++);

		// Next string
		if (c==' ')
		{
			while ((pos<len) && (list.at(pos)==' '))
				pos++;
			stringcnt++;
			continue;
		}

		// String (FIXME: '"' inside of strings?)
		if (c=='"')
		{
			gint startpos=pos;
			while ((pos<len) && (list.at(pos++)!='"'));
			std::string value=list.substr(startpos,pos-startpos-1);
			if (stringcnt%2)
			{
				// Parameter: convert to lower case
				gchar *lowercase=g_utf8_strdown(value.c_str(),-1);
				parameter=std::string(lowercase);
				g_free(lowercase);
				continue;
			}

			// Look for parameters we need
			if (parameter=="charset")
				partinfo.charset=value;
			continue;
		}

		// Otherwise: Error!
		return false;
	}
	return true;
}

/**
 * Reset the counter for tagging imap commands.
 */
void 
Imap4::reset_tag()
{
	tag_=std::string("");
	tagcounter_=0;
}

/**
 * Give the tag (including the following space) of the last sent IMAP command.
 *
 * @return  a C++ string with the tag
 */
std::string 
Imap4::tag()
{
	return tag_;
}

/**
 * Send an IMAP command.
 * The given {\em command} is prefixed with a unique identifier (obtainable
 * via the tag() function) and postfixed with "\r\n" and then written to the
 * socket of the mailbox.
 *
 * @param command  IMAP command as a C++ string
 * @param debug    boolean that says if the command should be printed in debug
 *                 mode (default is true)
 * @return         return value of the socket write command
 */
gint 
Imap4::send(std::string command, gboolean debug)
{
	// Create new tag
	tagcounter_++;
	gint len=g_snprintf(NULL,0,"A%05d ",tagcounter_)+1;
	gchar *buffer=g_strnfill(len,'\0');
	if (buffer==NULL)
		return 0;
	if (g_snprintf(buffer,len,"A%05d ",tagcounter_)!=len-1)
		return 0;
	tag_=std::string(buffer);
	g_free(buffer);
	// Write line
	return socket_->write (tag_ + command + "\r\n", debug);
}
