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
// Revision      : $Revision: 1.2 $
// Revision date : $Date: 2004/11/16 13:26:36 $
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


class Imap4 : public Mailbox {

private:
	std::string tag_;
	guint tagcounter_;
protected:
	class Socket *			socket_;
	std::vector<int>		saved_;
	std::string parse_bodystructure (std::string, gint &,
									 gboolean toplevel=true);
	void reset_tag();
	std::string tag();
	gint send(std::string,gboolean debug=true);
public:
	/* Base */
	Imap4 (class Biff *owner);
	Imap4 (const Mailbox &other);
	~Imap4 (void);

	/* Main */
	gint connect (void);

	/* Mailbox inherited methods */
	void get_status (void);
	void get_header (void);
};

#endif
