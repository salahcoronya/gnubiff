// ========================================================================
// gnubiff -- a mail notification program
// Copyright (c) 2000-2011 Nicolas Rougier, 2004-2011 Robert Sowada
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
// File          : $RCSfile: nls.h,v $
// Revision      : $Revision: 1.4.2.1 $
// Revision date : $Date: 2007/09/08 18:06:30 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : 
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#ifndef __NLS_H__
#define __NLS_H__

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#ifdef ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext(GETTEXT_PACKAGE,String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop(String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define _(String) (String)
#  define N_(String) (String)
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,String) (String)
#  define dcgettext(Domain,String,Type) (String)
#  define bindtextdomain(Domain,Directory) (Domain)
#endif

#endif
