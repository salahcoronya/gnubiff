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
// File          : $RCSfile: apop.cc,v $
// Revision      : $Revision: 1.10 $
// Revision date : $Date: 2005/01/03 20:32:21 $
// Author(s)     : Nicolas Rougier
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#include "support.h"

#ifdef HAVE_CRYPTO
#  include <openssl/md5.h>
#endif

#include <string>
#include <sys/stat.h>
#include <utime.h>

#include "ui-authentication.h"
#include "socket.h"
#include "apop.h"

// ========================================================================
//  base
// ========================================================================	
Apop::Apop (Biff *biff) : Pop (biff)
{
	protocol_ = PROTOCOL_APOP;
}

Apop::Apop (const Mailbox &other) : Pop (other)
{
	protocol_ = PROTOCOL_APOP;
}

Apop::~Apop (void)
{
}

// ========================================================================
//  main
// ========================================================================	
void 
Apop::connect (void) throw (pop_err)
{
	std::string line;

	// Open the socket
	Pop::connect ();

	// Does the server support the apop protocol?
	//  if so, answer should be something like:
	//  +OK POP3 server ready <1896.697170952@dbc.mtview.ca.us>
	readline (line);
	guint lt=line.find ("<"), gt=line.find (">");
	if ((lt == std::string::npos) || (gt == std::string::npos) || (gt < lt)) {
		g_warning (_("[%d] Your pop server does not seem to accept apop protocol (no timestamp provided)"), uin_);
		throw pop_command_err ();
	}

	// Get time stamp from server
	std::string timestamp = line.substr (lt, gt-lt+1);

	// Build message if MD5 library available
	char hex_response[33];
#ifdef HAVE_CRYPTO
	unsigned char response[16];
	MD5_CTX ctx;
	MD5_Init (&ctx);
	MD5_Update (&ctx, timestamp.c_str(), timestamp.size());
	MD5_Update (&ctx, password_.c_str(), password_.size());
	MD5_Final (response, &ctx);
	for (guint i = 0; i < 16; i++)
		sprintf (&hex_response[i*2], "%02x", response[i]);
	hex_response[32] = '\0';
#else
	g_warning (_("[%d] Problem with crypto that should have been detected at configure time"), uin_);
	return 0;
#endif

	// LOGIN
	sendline ("APOP " + username_ + " " + std::string (hex_response));
	readline (line); // +OK response
}
