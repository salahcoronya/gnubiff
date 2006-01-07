// ========================================================================
// gnubiff -- a mail notification program
// Copyright (c) 2000-2006 Nicolas Rougier, 2004-2006 Robert Sowada
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
// File          : $RCSfile: signals.cc,v $
// Revision      : $Revision: 1.3 $
// Revision date : $Date: 2006/01/01 16:44:53 $
// Author(s)     : Nicolas Rougier, Robert Sowada
// Short         : Handling of signals
//
// This file is part of gnubiff.
//
// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// ========================================================================

#include <signal.h>
#include "biff.h"
#include "signals.h"
#include "ui-applet.h"

/// Pointer to biff
class Biff *Signals::biff_ = NULL;

/**
 *  Initialize signal handling. After calling this function SIGUSR1 and
 *  SIGUSR2 signals will be caught.
 *
 *  @param  biff  Pointer to biff
 */
gboolean 
Signals::init_signals (class Biff *biff)
{
	biff_=biff;

	// Attach callback function to signals
	if (signal (SIGUSR1, Signals::signal_handler) == SIG_ERR)
		return false;
	if (signal (SIGUSR2, Signals::signal_handler) == SIG_ERR)
		return false;
	if (signal (SIGBUS, Signals::signal_handler) == SIG_ERR)
		return false;
	if (signal (SIGFPE, Signals::signal_handler) == SIG_ERR)
		return false;
	if (signal (SIGILL, Signals::signal_handler) == SIG_ERR)
		return false;
	if (signal (SIGSEGV, Signals::signal_handler) == SIG_ERR)
		return false;

	return true;
}

/**
 *  Callback function for handling signals.
 *
 *  @param  signal  Number of the signal that was caught.
 */
void 
Signals::signal_handler (int signum)
{
#ifdef DEBUG
	g_message ("Caught signal %d.", signum);
#endif
	if (!biff_)
		return;

	// What signal was caught?
	guint cmd;
	switch (signum) {
	case SIGUSR1:
		cmd = biff_->value_uint ("signal_sigusr1");
		break;
	case SIGUSR2:
		cmd = biff_->value_uint ("signal_sigusr2");
		break;
	case SIGBUS:
		Support::unknown_internal_error_ (NULL, 0, NULL, "SIGBUS");
		exit (EXIT_FAILURE);
	case SIGFPE:
		Support::unknown_internal_error_ (NULL, 0, NULL, "SIGFPE");
		exit (EXIT_FAILURE);
	case SIGILL:
		Support::unknown_internal_error_ (NULL, 0, NULL, "SIGILL");
		exit (EXIT_FAILURE);
	case SIGSEGV:
		Support::unknown_internal_error_ (NULL, 0, NULL, "SIGSEGV");
		exit (EXIT_FAILURE);
	default:
		return;
	}

	// What command has to be executed?
	switch (cmd) {
	case SIGNAL_NONE:
		break;
	case SIGNAL_MARK_AS_READ:
		biff_->mark_messages_as_read ();
		biff_->applet()->update ();
		break;
	case SIGNAL_START:
		biff_->start_monitoring ();
		break;
	case SIGNAL_STOP:
		biff_->stop_monitoring ();
		break;
	}
}
