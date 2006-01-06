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
// File          : $RCSfile: decoding.h,v $
// Revision      : $Revision: 1.17 $
// Revision date : $Date: 2006/01/01 16:44:52 $
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

#ifdef HAVE_AES
#   ifdef HAVE_LIBSSL
#       include <openssl/aes.h>
#   elif HAVE_LIBCRYPTO
#       include <crypto/aes.h>
#   endif
#endif

#include <glib.h>
#include <string>
#include <vector>

/**
 * This class is intended to provide functions needed for decoding,
 * encoding and converting encodings used in mails.
 *
 * @see In the class Support are various other support functions.
 */
class Decoding
{
protected:
	// Mail body and header
	gboolean decode_body (std::vector<std::string> &mail, std::string encoding,
						  std::string::size_type bodypos = 0,
						  gboolean skip_header = true);
	std::string decode_headerline (const std::string &line);
	gboolean parse_encoded_word (const std::string &line, std::string &charset,
								 std::string &encoding, std::string &text,
								 std::string::size_type &pos);
	gboolean get_quotedstring (std::string line, std::string &str,
							   std::string::size_type &pos,
							   gchar quoted = '"', gboolean test_start = true,
							   gboolean end_ok = false);
	gboolean get_mime_token (std::string line, std::string &str,
							 std::string::size_type &pos,
							 gboolean lowercase = true);

	// Encodings
	std::string decode_base64 (const std::string &);
	std::string decode_qencoding (const std::string &);
	std::string decode_quotedprintable (const std::string &);
	std::vector<std::string> decode_quotedprintable (
				const std::vector<std::string> &,
				std::string::size_type pos=0);

	// Converting
	std::string utf8_to_imaputf7 (std::string str);
	gchar* utf8_to_imaputf7 (const gchar *, gssize);
	std::string ascii_strdown (const std::string &str);
	gchar *charset_to_utf8 (std::string text, std::string charset);

public:
	// Password
	static std::string encrypt_password (const std::string &password,
										 const std::string &passtable);
	static std::string decrypt_password (const std::string &password,
										 const std::string &passtable);
	static std::string decrypt_aes (const std::string &passphrase,
									const std::string &data);
	static std::string encrypt_aes (const std::string &passphrase,
									const std::string &data);
};

#endif
