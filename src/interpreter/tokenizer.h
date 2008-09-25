/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

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


#include "token.h"
#include "translator.h"


/**
 * @short Generates Token objects from a QString using the Translator.
 *
 * The Tokenizer reads, one-by-one, characters from a QString (unicode text).
 * By trying to translate the tokens it tries to find out the type of
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
		~Tokenizer() {}

		/**
		 * @short Initializes (resets) the Tokenizer
		 * Use this method to reset the Tokenizer.
		 * @param inStream the QString that the Tokenizer will tokenize
		 */
		void initialize(const QString& inStream);

		/**
		 * Reads a bunch of characters of the input stream and tries to
		 * recognize them as a certain token type, and returns a Token of that type.
		 * If nothing is recognized a Token of the type Unknown is returned.
		 * The singleton Translator object is used to determain the type.
		 * @returns a pointer to a newly created token as read from the input stream
		 */
		Token* getToken();


	private:
		QChar getChar();    // gets a the next QChar and sets the row and col accordingly
		void  ungetChar();  // undoes a getChar() call
		static bool isBreak(const QChar& c);  // convenience functions
		static bool isSpace(const QChar& c);

		Translator *translator;
		QString     inputString;

		int at, row, col, prevCol;

		bool atEnd;  // true if the QString is fully tokenized
};


#endif  // _TOKENIZER_H_
