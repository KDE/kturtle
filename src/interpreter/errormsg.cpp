/*
    SPDX-FileCopyrightText: 2003-2006 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "errormsg.h"


ErrorMessage::ErrorMessage(const QString& text, const Token& t, int code)
 : _errorText(text),
   _errorToken(t),
   _errorCode(code)
{
}

bool ErrorMessage::operator==(const ErrorMessage& n) const
{
	// the 'operator==' method has to be implemented for the ErrorList wants to do searches
	if (n.text()  == _errorText  ||
	    n.token() == _errorToken ||
	    n.code()  == _errorCode) return true;
	return false;
}
