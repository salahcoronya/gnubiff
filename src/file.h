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
// File          : $RCSfile: file.h,v $
// Revision      : $Revision: 1.5 $
// Revision date : $Date: 2007/09/08 18:06:16 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __FILE_H__
#define __FILE_H__

#include "local.h"
#include <sys/stat.h>


class File : public Local {

public:
	// ========================================================================
	//  base
	// ========================================================================	
	File (class Biff *biff);
	File (const Mailbox &other);
	~File (void);

	// ========================================================================
	//  base
	// ========================================================================	
	void fetch (void) throw (local_err);
	void handle_message (std::vector<std::string> &msg, gint tmp_file = -1);
};

#endif
