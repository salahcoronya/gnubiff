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
// File          : $RCSfile: apop.h,v $
// Revision      : $Revision: 1.3 $
// Revision date : $Date: 2005/01/03 17:24:37 $
// Author(s)     : Nicolas Rougier
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __APOP_H__
#define __APOP_H__

#include "pop.h"

/**
 * Mailbox for the APOP protocol. 
 */
class Apop : public Pop {

public:
	// ========================================================================
	//  base
	// ========================================================================	
	Apop (class Biff *biff);
	Apop (const Mailbox &other);
	~Apop (void);

	// ========================================================================
	//  main
	// ========================================================================	
	void connect (void) throw (pop_err);
};

#endif
