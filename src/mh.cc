// ========================================================================
// gnubiff -- a mail notification program
// Copyright (c) 2000-2005 Nicolas Rougier
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
// File          : $RCSfile: mh.cc,v $
// Revision      : $Revision: 1.12 $
// Revision date : $Date: 2005/04/05 16:17:48 $
// Author(s)     : Nicolas Rougier
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#include <fstream>
#include <sstream>
#include "mh.h"

// ========================================================================
//  base
// ========================================================================	
/**
 * Constructor. The local mailbox for the mh protocol is created from
 * scratch.
 *
 * @param biff Pointer to the instance of Gnubiff.
 */
Mh::Mh (Biff *biff) : Mh_Basic (biff)
{
	value ("protocol", PROTOCOL_MH);
}

/**
 * Constructor. The local mailbox for the mh protocol is created by
 * taking the attributes of the existing mailbox {\em other}.
 *
 * @param other Mailbox from which the attributes are taken.
 */
Mh::Mh (const Mailbox &other) : Mh_Basic (other)
{
	value ("protocol", PROTOCOL_MH);
}

/// Destructor
Mh::~Mh (void)
{
}

// ========================================================================
//  main
// ========================================================================	

/**
 *  Get message numbers of the mails to be parsed. In the mh protocol the
 *  message numbers of unread mails are stored in the file ".mh_sequences".
 *
 *  @param  msn    Reference to a vector in which the message numbers are
 *                 returned
 *  @param  empty  Whether the vector shall be emptied before obtaining the
 *                 message numbers (the default is true)
 *  @return        Boolean indicating success
 */
gboolean 
Mh::get_messagenumbers (std::vector<guint> &msn, gboolean empty)
{
	// Empty the vector if wished for
	if (empty)
		msn.clear ();

	// Open file
	std::string filename = add_file_to_path (address (), ".mh_sequences");
	std::ifstream file;
	file.open (filename.c_str ());
	if (!file.is_open ())
		return false;

	// Parse mh sequences and try to find the unseen sequence
	while (!file.eof()) {
		std::string line;
		getline (file, line);

		// Got it!
		if (line.find("unseen:") == 0) {
			line = line.substr (7); // size of "unseen:" is 7
			if (!numbersequence_to_vector (line, msn))
				return false;
			break;
		}
	}

	// Close file
	file.close();

	return true;
}

/**
 *  Give the name of the file that shall be monitored by FAM. For the mh
 *  protocol this is the ".mh_sequences" file.
 *
 *  @return    Name of the file to be monitored.
 */
std::string 
Mh::file_to_monitor (void)
{
	return add_file_to_path (address(), std::string(".mh_sequences"));
}
