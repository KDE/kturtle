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


#ifndef _TRANSLATOR_H_
#define _TRANSLATOR_H_

#include <QChar>
#include <QHash>
#include <QString>
#include <QStringList>

#include <klocale.h>


/**
 * @short Uses an XML dictionary to translate unicode strings to Token types (if possible).
 *
 * This class can read XML dictionaries containing maps of international
 * strings and standard strings. Using the generated code in
 * @ref stringType2intType() the standard strings can be mapped to Token
 * types.
 *
 * When a dictionary is loaded by @ref loadDictionary() a private member map
 * called look2typeMap is created and filled where international strings
 * are mapped directly to the Token types.
 *
 * A some of the code of this class is generated code.
 *
 * @author Cies Breijs
 */
class Translator
{
	public:
		static Translator* instance();

// // //		/** @short   Tries to load an XML dictionary.
// // //		    @param   xmlFile the file of the dictionary
// // //		    @returns TRUE is the loading was successful, otherwise FALSE */
		bool setLanguage(const QString &lang_code);

		/** @short Converts a unicode string to a token type.
		    Uses the dictionary to do so.
		    If the string could not be turned into a Token type, the Token
		    type Unknown is returned.
		    @param   look the unicode string a bit of KTurtle code
		    @returns the token type, Token::Unknown if not recognised */
		int look2type(QString& look);

		/** @short Converts a unicode character to a token type.
		    Convenience method, essentially behaves like the above.
		    @param   look one unicode character of KTurtle code
		    @returns the token type, Token::Unknown if not recognised */
		int look2type(QChar& look);

		/** @short Converts a token type into a list of commands associated with it.
		    This method is slow compared to the inverse, look2type(), methods
		    because the internal representation of data is not optimized for this.
		    This methods is used by the highlighter class.
		    @param   type the token type as specified in the Token class
		    @returns a QList of QString objects containing all the looks of the
		             command in the current translation. */
		QList<QString> type2look(int type);

		QHash<int, QList<QString> > token2stringsMap();

		QString default2localized(QString& defaultLook) { return default2localizedMap[defaultLook]; }


		QStringList exampleNames() const { return QStringList(examples.keys()); }

		QString example(const QString& name) const { return examples[name]; }


	protected:
		/** @short Constructor. Does nothing special. */
		Translator();

		/** @short Destructor. Does nothing special. */
		virtual ~Translator();

		Translator(const Translator&);
		Translator& operator= (const Translator&);


	private:
		static Translator* m_instance;

		void setDictionary();
		void setExamples();

		QString parseExampleTranslations(const QString& halfTranslatedExample);
		QHash<QString, QString> examples;

		QHash<QString, int> look2typeMap;
		QHash<QString, QString> default2localizedMap;

		KLocale* localizer;

		QString currentLangugeCode;
};


#endif // _TRANSLATOR_H_
