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
// File          : $RCSfile: maildir.h,v $
// Revision      : $Revision: 1.3 $
// Revision date : $Date: 2005/04/13 12:48:58 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __MAILDIR_H__
#define __MAILDIR_H__

#include "local.h"
#include <time.h>


class Maildir : public Local {

public:
	// ========================================================================
	//  base
	// ========================================================================	
	Maildir (class Biff *biff);
	Maildir (const Mailbox &other);
	~Maildir (void);

	// ========================================================================
	//  main
	// ========================================================================	
	void fetch (void) throw (local_err);
};

#endif
