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
// File          : $RCSfile: mh_basic.h,v $
// Revision      : $Revision: 1.3 $
// Revision date : $Date: 2006/01/01 16:44:53 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : Base class for all local protocols similar to mh
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __MH_BASIC_H__
#define __MH_BASIC_H__

#include <glib.h>
#include <vector>
#include "local.h"

/**
 *  Base class for all local protocols similar to mh. Can be used itself for
 *  mail notification but determining mail status is not very efficient. This
 *  is because message numbers of unread messages cannot be obtained.
 */
class Mh_Basic : public Local {

protected:

public:
	// ========================================================================
	//  base
	// ========================================================================
	Mh_Basic (class Biff *biff);
	Mh_Basic (const Mailbox &other);
	~Mh_Basic (void);

	// ========================================================================
	//  main
	// ========================================================================
	virtual void get_messagenumbers (std::vector<guint> &msn,
									 gboolean empty = true) throw (local_err);
	void fetch (void) throw (local_err);
};

#endif
