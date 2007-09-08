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
// File          : $RCSfile: imap4.h,v $
// Revision      : $Revision: 1.49 $
// Revision date : $Date: 2006/01/01 16:44:53 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __IMAP4_H__
#define __IMAP4_H__

#include "mailbox.h"
#include "socket.h"

#define IMAP4(x)				((Imap4 *)(x))

/**
 * Mailbox for the IMAP4 protocol. 
 */
class Imap4 : public Mailbox {

 protected:
	/// Socket to talk to the server
	class Socket *				socket_;

	/// Does the server support the IDLE capability?
	gboolean					idleable_;

	/// Is the server currently idled?
	gboolean					idled_;

	/// Tag created for the last sent IMAP command.
	std::string					tag_;

	/** Counter for creating the tag of the next IMAP command to be sent to
	 *  the server. */
	guint						tagcounter_;

	/** Value of UIDVALIDITY as sent by the IMAP4 server. This is a response
	 *  code to the SELECT command. The string is empty if the server does
	 *  not send it. See also RFC 3501 2.3.1.1.
	 */
	std::string					uidvalidity_;

	/** Map of pairs (atom, arg) that represent the last sent server response
	 *  codes via untagged "* OK" server responses. */
	std::map<std::string, std::string> ok_response_codes_;

	/// Was the last line sent by the server an untagged response?
	gboolean					last_untagged_response_;

	/// Contents of the last untagged response. This value may be empty.
	std::string					last_untagged_response_cont_;

	/// Keyword of the last untagged response
	std::string					last_untagged_response_key_;

	/** Message sequence number of the last untagged response. This value is
	 *  0 if there was no message sequence number. */
	guint						last_untagged_response_msn_;

	/// Map of message sequence numbers and corresponding unique ids.
	std::map<guint,std::string> msn_uid_;

 public:
	// ========================================================================
	//  base
	// ========================================================================

	Imap4 (class Biff *owner);
	Imap4 (const Mailbox &other);
	~Imap4 (void);

	// ========================================================================
	//  exceptions
	// ========================================================================
	/** Generic exception for IMAP mailboxes serving as a base for more
	 *  specific exceptions. */
	class imap_err : public mailbox_err
	{
	public:
		/** Constructor. 
		 *
		 * @param mailboxerror Whether this exception should imply a mailbox
		 *                     error status or not. The default is true. */
		imap_err (gboolean mailboxerror=true) : mailbox_err (mailboxerror) {}
	};
	/** Exception for a socket connection failure. Usually this is thrown when
	 *  reading or writing. */
	class imap_socket_err : public imap_err {};
	/** Exception for a problem with a IMAP command. This exception may be
	 *  thrown in the following situations:
	 *  \begin{itemize}
	 *     \item There is an error when creating the line that is to be sent to
	 *           the server
	 *     \item There is an unexpected response by the server to the command
	 *     \item The command is not responded by OK
	 *  \end{itemize} */
	class imap_command_err : public imap_err
	{
	public:

		/** Constructor. 
		 *
		 * @param mailboxerror Whether this exception should imply a mailbox
		 *                     error status or not. The default is true. */
		imap_command_err(gboolean mailboxerror=true):imap_err(mailboxerror) {}
	};

	/// This exception is thrown when a DoS attack is suspected. 
	class imap_dos_err : public imap_err {};

	/** This exception is thrown when login isn't possible. This can happen in
	 *  the following situations:
	 *  \begin{itemize}
	 *     \item The server doesn't want us to login (via the LOGINDISABLED
	 *           capability)
	 *     \item The user doesn't provide a password
	 *  \end{itemize} */
	class imap_nologin_err : public imap_err {};

	// ========================================================================
	//  main
	// ========================================================================

	virtual void threaded_start (guint delay = 0);
	void start (void);
	void fetch (void) throw (imap_err);
	void connect (void) throw (imap_err);
	void fetch_mails (void) throw (imap_err);
	
 private:
	// ========================================================================
	//	Internal stuff
	// ========================================================================

	guint isfinished_fetchbodystructure(std::string,guint) throw (imap_err);
	gboolean parse_bodystructure (std::string, class PartInfo &,
								  gboolean toplevel=true);
	gboolean parse_bodystructure_parameters (std::string, class PartInfo &);
	void command_capability (gboolean check_rc = false) throw (imap_err);
	void command_fetchbody (guint, class PartInfo &,
							std::vector<std::string> &) throw (imap_err);
	PartInfo command_fetchbodystructure (guint) throw (imap_err);
	std::vector<std::string> command_fetchheader (guint) throw (imap_err);
	std::set<std::string> command_fetchuid (std::set<guint>) throw (imap_err);
	std::string command_idle(gboolean &) throw (imap_err);
	void command_login (void) throw (imap_err);
	void command_logout (void) throw (imap_err);
	std::set<guint> command_searchnotseen (void) throw (imap_err);
	void command_select (void) throw (imap_err);
	void waitfor_ack (std::string msg=std::string(""),
					  gint num = 0) throw (imap_err);
	gboolean waitfor_ack_untaggedresponse (std::string,
				std::string contbegin = std::string(""),
				gint num = 0) throw(imap_err);
	void waitfor_untaggedresponse (guint, std::string,
								   std::string contbegin=std::string(""),
								   gint num = 0) throw (imap_err);
	void reset_tag();
	std::string tag();
	gint sendline (const std::string command, gboolean print = true,
				   gboolean check = true) throw (imap_err);
	gint sendline (const std::string command, guint msn, const std::string arg,
				   gboolean print = true, gboolean check = true)
				   throw (imap_err);
	gint readline (std::string &line, gboolean print = true,
				   gboolean check = true, gboolean checkline = true)
				   throw (imap_err);
	gint readline_ignoreinfo (std::string &line, gboolean print = true,
							  gboolean check = true, gboolean checkline = true)
							  throw (imap_err);
	void save_response_code (std::map<std::string, std::string> &)
							 throw (imap_err);
	void save_untagged_response (std::string &) throw (imap_err);
	gboolean test_untagged_response (guint, std::string,
									 std::string contbegin = std::string (""));
	gboolean test_untagged_response (std::string,
									 std::string contbegin = std::string (""));
	void update_applet();
	void idle() throw (imap_err);
};

#endif
