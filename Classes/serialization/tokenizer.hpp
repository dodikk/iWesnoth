/* $Id: tokenizer.hpp 31859 2009-01-01 10:28:26Z mordante $ */
/*
   Copyright (C) 2004 - 2009 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file serialization/tokenizer.hpp. */

#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include "util.hpp"

#include <istream>
#include <string>

#define TOKENIZER_BUFFER_SIZE	50 * 1024 * 1024

class config;

struct token
{
	token() :
		type(END),
		leading_spaces(),
		value()
		{
			reset();
		}

	enum token_type {
		STRING,
		QSTRING,
		UNTERMINATED_QSTRING,
		MISC,

		LF = '\n',
		EQUALS = '=',
		COMMA = ',',
		PLUS = '+',
		SLASH = '/',
		OPEN_BRACKET = '[',
		CLOSE_BRACKET = ']',
		UNDERSCORE = '_',
		END
	} type;

	void reset() {
		value.clear();
		leading_spaces.clear();
		/*
		value_ptr = 0;
		value_size = 0;
		leading_spaces_ptr = 0;
		leading_spaces_size = 0;
		 */
	}

	std::string leading_spaces;
	std::string value;
	
	/*
	// KP: creating std::strings all the time was a huge performance hit
	const char *leading_spaces_ptr;
	int leading_spaces_size;
	const char *value_ptr;
	int value_size;
	*/
};

/** Abstract baseclass for the tokenizer. */
class tokenizer
{
	public:
		tokenizer(std::istream& in);
		~tokenizer();

		const token& next_token();	// KP: moved to tokenizer.cpp

		void check_translatable()
		{
			if(token_.value == "_")
			//if(token_.value_ptr && token_.value_ptr[0] == '_')
				token_.type = token::token_type('_');
		}


		const token& current_token() const
		{
			return token_;
		}
#ifdef DEBUG
		const token& previous_token() const;
#endif

		std::string& textdomain()
		{
			return textdomain_;
		}

		const std::string& get_file() const
		{
			return file_;
		}

		int get_start_line() const
		{
			return startlineno_;
		}

	protected:
		tokenizer();
		int current_;
		size_t lineno_;
		size_t startlineno_;

		inline void next_char()
		{
			if (UNLIKELY(current_ == '\n'))
				lineno_++;
			this->next_char_fast();
		}

		inline void next_char_fast()
		{
			do {
				if (LIKELY(in_.good()))
				//if (curPos < numChars-1)
				{
					current_ = in_.get();
					//current_ = (unsigned char) in_buffer[curPos++];
				}
				else
				{
					current_ = EOF;
					return;
				}
			} while (UNLIKELY(current_ == '\r'));
						
#if 0
			// @todo: disabled untill campaign server is fixed
			if(LIKELY(in_.good())) {
				current_ = in_.get();
				if (UNLIKELY(current_ == '\r'))
				{
					// we assume that there is only one '\r'
					if(LIKELY(in_.good())) {
						current_ = in_.get();
					} else {
						current_ = EOF;
					}
				}
			} else {
				current_ = EOF;
			}
#endif
		}

		inline int peek_char() const
		{
			return in_.peek();
			//return in_buffer[curPos];
		}

	private:
		inline bool is_space(const int c) const
		{
			return c == ' ' || c == '\t';
		}
	/*inline*/ bool is_alnum(const int c) const;
		
		void skip_comment();

		std::string textdomain_;
		std::string file_;
		token token_;
#ifdef DEBUG
		token previous_token_;
#endif
		std::istream& in_;
	
		/*
		// KP: added for much faster tokenizing
		char *in_buffer;
		int curPos;
		int numChars;
		 */
};

#endif

