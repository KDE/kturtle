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

#ifndef _ERRORMSG_H_
#define _ERRORMSG_H_


#include <QtDebug>  // no kde stuff in the interpreter


#include <QList>

#include "token.h"



/**
 * @short An object for an error message.
 *
 * Basiaclly it is a agregation of a QString (the message text), a Token (the Token
 * that is related to the error) and a code (a simple interger).
 *
 * ErrorMessages are created by the Parser and the Executer.
 *
 * @TODO investigate if it will be beter to replace this class by a struct for speed.
 *
 * @author Cies Breijs
 */
class ErrorMessage
{
	public:
		ErrorMessage() {}
		ErrorMessage(const QString&, const Token& t, int code);
		~ErrorMessage() {}

		const QString&  text()  const { return _errorText; }
		const Token&    token() const { return _errorToken; }
		int             code()  const { return _errorCode; }

		bool operator==(const ErrorMessage&) const;
		ErrorMessage& operator=(const ErrorMessage& n);

	private:
		QString  _errorText;
		Token    _errorToken;
		int      _errorCode;
};


/**
 * @short The simple subclass of QList to represent list of ErrorMessages.
 */
class ErrorList : public QList<ErrorMessage>
{
	public:
		QString asString()
		{
			QString result = "";
			ErrorList::iterator error;
			int i = 1;
			for (error = this->begin(); error != this->end(); ++error) {
				result += QString("%1: %2 [by %3 on line %4], code %5")
					.arg(i)
					.arg((*error).text())
					.arg((*error).token().look())
					.arg((*error).token().startRow())
					.arg((*error).code());
				i++;
			}
			return result;
		}

		void addError(const QString& s, const Token& t, int code)
		{
			ErrorMessage error = ErrorMessage(s, t, code);
			addError(error);
		}

		void addError(ErrorMessage error)
		{
			if (!this->contains(error)) this->append(error);
			qDebug() <<
				QString("ErrorList::addError(): %1 [by %2 on line %3], code %4")
					.arg(error.text())
					.arg(error.token().look())
					.arg(error.token().startRow())
					.arg(error.code());
		}
};

#endif  // _ERRORMSG_H_
