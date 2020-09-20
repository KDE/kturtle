/*
	SPDX-FileCopyrightText: 2003-2006 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _ERRORMSG_H_
#define _ERRORMSG_H_

#include "token.h"

#include <QDebug>

#include <QList>
#include <QStringList>




/**
 * @short An object for an error message.
 *
 * Basically it is a aggregation of a QString (the message text), a Token (the Token
 * that is related to the error) and a code (a simple integer).
 *
 * ErrorMessages are created by the Parser and the Executer.
 *
 * @TODO investigate if it will be better to replace this class by a struct for speed.
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
		QStringList asStringList()
		{
			QStringList result;
			ErrorList::iterator error;
			int i = 1;
			for (error = this->begin(); error != this->end(); ++error) {
				result.append(QStringLiteral("%1: %2 [by %3 on line %4], code %5")
					.arg(i)
					.arg((*error).text())
					.arg((*error).token().look())
					.arg((*error).token().startRow())
					.arg((*error).code()));
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
// 			//qDebug() <<
// 				QString("ErrorList::addError(): %1 [by %2 on line %3], code %4")
// 					.arg(error.text())
// 					.arg(error.token().look())
// 					.arg(error.token().startRow())
// 					.arg(error.code());
		}
};

#endif  // _ERRORMSG_H_
