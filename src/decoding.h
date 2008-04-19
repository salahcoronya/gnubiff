// ========================================================================
// gnubiff -- a mail notification program
// Copyright (c) 2000-2008 Nicolas Rougier, 2004-2008 Robert Sowada
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
// File          : $RCSfile: decoding.h,v $
// Revision      : $Revision: 1.20.2.1 $
// Revision date : $Date: 2007/09/08 18:06:30 $
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
	gboolean parse_encoded_word_search (const std::string &line,
										const std::string::size_type &pos,
										std::string::size_type &iter,
										gboolean searchLast = false);
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
	std::string decode_base64 (const std::vector<std::string> &,
							   std::string::size_type pos = 0);
	std::string decode_qencoding (const std::string &);
	std::string decode_quotedprintable (const std::string &);
	std::vector<std::string> decode_quotedprintable (
				const std::vector<std::string> &,
				std::string::size_type pos = 0);

	// Converting
	std::string utf8_to_imaputf7 (std::string str);
	gchar* utf8_to_imaputf7 (const gchar *, gssize);
	std::string ascii_strdown (const std::string &str);
	gchar *charset_to_utf8 (std::string text, std::string charset,
							guint retries = 0);

public:
	// Decryption/Encryption
	static std::string decrypt_password_legacy (const std::string &passphrase,
												const std::string &password);
	static std::string decrypt_aes (const std::string &passphrase,
									const std::string &data);
	static std::string encrypt_aes (const std::string &passphrase,
									const std::string &data);
};

#endif
