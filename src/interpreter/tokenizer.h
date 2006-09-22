/*
	Copyright (C) 2003-2006 Cies Breijs <cies # kde ! nl>

	This program is free software; you can redistribute it and/or
	modify it under the terms of version 2 of the GNU General Public
	License as published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the Free
	Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.
*/


#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <QChar>
#include <QString>
#include <QTextStream>

#include "token.h"
#include "translator.h"


/**
 * @short Generates Token objects from a QTextStream using a Translator.
 *
 * The Tokenizer reads characters off a QTextStream (unicode text).
 * By trying to translate the the tokens it tries to find out the type of
 * the tokens, since KTurtle code can be in many different languages
 * the programming commands are not known on forehand.
 *
 * @author Cies Breijs
 */
class Tokenizer
{
	public:
		/**
		 * @short Constructor. Initialses a Tokenizer.
		 * Does nothing special. @see initialize().
		 */
		Tokenizer() {}

		/** @short Destructor. Does nothing special. */
		virtual ~Tokenizer() {}

		/**
		 * @short Initializes (resets) the Tokenizer
		 * Use this method to reset the Tokenizer.
		 * @param inStream   the QTextStream that the Tokenizer will work on
		 * @param translator the Translator that the Tokenizer will use
		 */
		void initialize(QTextStream& inStream);

		/**
		 * Reads a bunch of characters of the input stream and tries to
		 * recognize them as a certain token type. Else type NotSet is used.
		 * The Translator object is used to determain the type.
		 * @returns a pointer to a newly created token as read from the input stream
		 */
		Token*          getToken();

	private:
		/// @returns the temporarily stored character or, if none, a character off the stream.
		QChar           getChar();

		/// Temporarily stores a character.
		void            ungetChar(QChar c);

		/// Convenience function used by getChar().
		void            updatePosition(QChar c);



		/// pointer to the Translator object.
		Translator     *translator;

		/// pointer to the input stream object.
		QTextStream    *inputStream;

		/// The current row number.
		int             row;

		/// The current column number.
		int             col;



		/// The temporarily stored character (doing this since we cannot write back to the stream).
		QChar           prevChar;

		/// The column of temporarily stored character.
		int             prevCol;
};


#endif // _TOKENIZER_H_
