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
// File          : $RCSfile: imap4.h,v $
// Revision      : $Revision: 1.10 $
// Revision date : $Date: 2004/12/29 21:27:02 $
// Author(s)     : Nicolas Rougier
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


class Imap4 : public Mailbox {

protected:
	class Socket *				socket_;		// socket to talk to server
	std::vector<int>			saved_;			// saved uidl's
	gboolean					idleable_;		// does server support the IDLE capability?
	gboolean					idled_;			// Is the  server currently idled

	std::string					tag_;			// Tag created for the last sent IMAP command
	guint						tagcounter_;	// Counter for creating the next tag
public:
	// ========================================================================
	//  base
	// ========================================================================	
	Imap4 (class Biff *owner);
	Imap4 (const Mailbox &other);
	~Imap4 (void);

	// ========================================================================
	//  main
	// ========================================================================	
	virtual void threaded_start (guint delay = 0);
	void start (void);
	void fetch (void);
	gint connect (void);
	void fetch_header (void);

	class imap_err : public std::exception {};	 // General Imap Exception
	class imap_socket_err : public imap_err {};	 // Socket connection Failure
												 // usually when reading or writing.
	class imap_command_err : public imap_err {}; // IMAP command not understood
												 // or not expected, or not
	                                             // responded by OK.
	class imap_dos_err : public imap_err {};	 // We've been attacked DoS style!
	
 private:
	// ========================================================================
	//	Internal stuff
	// ========================================================================	
	gboolean parse_bodystructure (std::string, class PartInfo &,
									gboolean toplevel=true);
	gboolean parse_bodystructure_parameters (std::string, class PartInfo &);
	std::vector<int> command_searchnotseen (void) throw (imap_err);
	void reset_tag();
	std::string tag();
	gint send(std::string,gboolean debug=true);
	std::string idle_renew_loop() throw (imap_err);	 // Renew IDLE state
													 // periodically. 
	void update_applet();						 // Update the applet to new IMAP state.
	void idle() throw (imap_err);		         // Begin idle IMAP mode.
	void close();								 // Cleanup and close IMAP connection.
};

class PartInfo
{
 public:
	std::string part;
	std::string mimetype;
	std::string encoding;
	std::string charset;
	gint size;
};

#endif
