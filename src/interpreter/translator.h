/*
    SPDX-FileCopyrightText: 2003-2006 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef _TRANSLATOR_H_
#define _TRANSLATOR_H_

#include <QChar>
#include <QHash>
#include <QString>
#include <QStringList>


static const char* DEFAULT_LANGUAGE_CODE = "en_US";

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

		/** @short   Sets the dictionary language.
		    By setting the dictionary KTurtle will use a different translation of TurtleScript.
		    Examples are managed by the Translator aswell. Easy.
		    TODO: is this redundant no KDE has a 'switch application langugae' in the Help(???) menu?
		    @param   lang_code the ISO language code of the dictionary (eg: "en_US", "fr", "pt_BR", "nl")
		    @returns TRUE is the loading was successful, otherwise FALSE */
		bool setLanguage(const QString &lang_code = QString(DEFAULT_LANGUAGE_CODE));

		/** @short Converts a unicode string to a token type.
		    Uses the dictionary to do so.
		    If the string could not translated to a Token type, Token::Unknown is returned.
		    @param   look the unicode string a bit of KTurtle code
		    @returns the token type, Token::Unknown if not recognised */
		int look2type(QString& look);

		/** @short Converts a unicode character to a token type.
		    Overloaded for convenience, behaves like the method it overloads.
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

		QString default2localized(const QString& defaultLook) { return default2localizedMap[defaultLook]; }

		/// returns all default looks that have a localized look (for translating examples in main.cpp)
		QStringList allDefaultLooks() { return QStringList(default2localizedMap.keys()); }
		
		QStringList allLocalizedLooks() { return QStringList(default2localizedMap.values()); }

		/// used by the MainWindow's context help logic, and main.cpp
		QString defaultLook(const QString& localizedEntry) { return default2localizedMap.key(localizedEntry); }

		QStringList exampleNames() const { return QStringList(examples.keys()); }

		QString example(const QString& name) const { return examples[name]; }

		QString localizeScript(const QString& untranslatedScript);


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

		QHash<QString, QString> examples;

		QHash<QString, int> look2typeMap;
		QHash<QString, QString> default2localizedMap;

		QStringList localizer;
};


#endif  // _TRANSLATOR_H_
