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
// File          : $RCSfile: decoding.h,v $
// Revision      : $Revision: 1.2 $
// Revision date : $Date: 2004/12/13 22:36:12 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : Various functions for decoding, converting ...
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __DECODING_H__
#define __DECODING_H__

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif
#include <glib.h>
#include <string>
#include <vector>

class Decoding
{
protected:
	gboolean decode_body (std::vector<std::string> &,std::string);

	// Encodings
	std::string decode_base64 (const std::string &);
	std::string decode_qencoding (const std::string &);
	std::string decode_quotedprintable (const std::string &);
	std::vector<std::string> decode_quotedprintable (const std::vector<std::string> &, guint pos=0);
};

#endif
