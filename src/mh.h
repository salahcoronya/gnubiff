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
// File          : $RCSfile: mh.h,v $
// Revision      : $Revision: 1.3 $
// Revision date : $Date: 2005/04/05 16:17:48 $
// Author(s)     : Nicolas Rougier
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __MH_H__
#define __MH_H__

#include <glib.h>
#include <vector>
#include "mh_basic.h"


class Mh : public Mh_Basic {

protected:
	std::vector<guint> 		saved_;			// saved uild's

public:
	// ========================================================================
	//  base
	// ========================================================================
	Mh (class Biff *biff);
	Mh (const Mailbox &other);
	~Mh (void);

	// ========================================================================
	//  main
	// ========================================================================
	gboolean get_messagenumbers (std::vector<guint> &msn,
								 gboolean empty = true);
	std::string file_to_monitor (void);
};

#endif
