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


#include "errormsg.h"


ErrorMessage::ErrorMessage(const QString& text, const Token& t, int code)
 : _errorText(text),
   _errorToken(t),
   _errorCode(code)
{
// 	_errorText  = text;
// 	// make a copy since the pointed token may be deleted:
// 	_errorToken = new Token(t.type(), t.look(), t.startRow(), t.startCol(), t.endRow(), t.endCol());
// 	_errorCode  = code;
}

bool ErrorMessage::operator==(const ErrorMessage& n) const
{
	// the 'operator==' mthod has to be implemented for the ErrorList wants to do searches
	if (n.text()  == _errorText  ||
	    n.token() == _errorToken ||
	    n.code()  == _errorCode) return true;
	return false;
}

ErrorMessage& ErrorMessage::operator=(const ErrorMessage& n)
{
	_errorText  = n.text();
	_errorToken = n.token();
	_errorCode  = n.code();
	return *this;
}
