/*
    Copyright (C) 2003 by Walter Schreppers 
    Copyright (C) 2004 by Cies Breijs
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <kdebug.h>
#include <klocale.h>
 
#include "value.h"


Value::Value()
{
	init();
}

Value::Value(const Value& n)
{
	*this = n;
}



int Value::Type() const
{
	return type;
}

void Value::setType(int newType)
{
	if (type == newType) return; // dont change values when type is not changing
	else if (newType == boolValue)
	{
		init();
		type = newType;
		m_string = i18n("false");
	}
	else if (newType == numberValue)
	{
		init();
		type = newType;
	}
	else if (newType == stringValue)
	{
		init();
		type = newType;
	}
	else if (newType == emptyValue) init();
}


bool Value::Bool() const
{
	if (type == boolValue) return m_bool;
	else if (type == numberValue) return (m_double - 0.5 > 0);
	return false;
}

void Value::setBool(bool b)
{
	type = boolValue;
	m_bool = b;
	if (m_bool)
	{
		m_double = 1;
		m_string = i18n("true");
	}
	else
	{
		m_double = 0;
		m_string = i18n("false");
	}
}


double Value::Number() const
{
	if (type == boolValue)
	{
		if (m_bool) return 1;
		else return 0;
	}
	else if (type == numberValue) return m_double;
	return 0; // stringValue, emptyValue
}

void Value::setNumber(double d)
{
	type = numberValue;
	m_double = d;
	m_string.setNum(d);
}

bool Value::setNumber(const QString &s)
{
	type = numberValue;
	bool ok = true;
	double num = s.toDouble(&ok);
	if (ok)
	{
		m_double = num;
		m_string.setNum(num);
		return true;
	}
	return false; 
}


QString Value::String() const
{
	if (type == boolValue)
	{
		if (m_bool) return QString( i18n("true") );
		else return QString( i18n("false") );
	}
	else if (type == numberValue)
	{
		QString s;
		s.setNum(m_double);
		return s;
	}
	return m_string; // stringValue, emptyValue
}

void Value::setString(double d)
{
	type = stringValue;
	m_double = d;
	m_string.setNum(d);
}

void Value::setString(const QString &s)
{
	type = stringValue;
	m_string = s;
}



Value& Value::operator=(const Value& n)
{
	if (this != &n)
	{
		if (type == n.type)
		{
			type = n.Type();
			m_bool = n.Bool();
			m_double = n.Number();
			m_string = n.String();
		}
		else if (n.Type() == boolValue)
		{
			setBool( n.Bool() );
		}
		else if (n.Type() == numberValue)
		{
			setNumber( n.Number() );
		}
		else if (n.Type() == stringValue)
		{
			setString( n.String() );
		}
		else if (n.Type() == emptyValue)
		{
			init();
		}
	}
	return *this;
}


Value& Value::operator=(const QString& s)
{
	setString(s);
	return *this;
}

Value& Value::operator=(double n)
{
	setNumber(n);
	return *this;
}



Value& Value::operator+(const Value& n)
{
	if (type == numberValue && n.Type() == numberValue)
	{
		m_double += n.Number();
	}
	else
	{
		type = stringValue;
		m_string = String() + n.String();
	}
	return *this;
}


Value& Value::operator-(const Value& n)
{
	if (type == numberValue && n.Type() == numberValue)
	{
		m_double -= n.Number();
	}
	else
	{
		kdDebug(0)<<"Value::operator; cannot subtract strings"<<endl;
	}
	return *this;
}


Value& Value::operator*(const Value& n)
{
	if (type == numberValue && n.Type() == numberValue)
	{
		m_double *= n.Number();
	}
	else
	{
		kdDebug(0)<<"Value::operator; cannot multiply strings"<<endl; 
	}
	return *this;
}


Value& Value::operator/(const Value& n)
{
	if (type == numberValue && n.Type() == numberValue)
	{
		m_double /= n.Number();
	}
	else
	{
		kdDebug(0)<<"Value::operator; cannot divide strings"<<endl;
	}
	return *this;
}



bool Value::operator==(const Value& n) const
{
	if (type == boolValue && n.Type() == boolValue)     return m_bool == n.Bool(); 
	if (type == numberValue && n.Type() == numberValue) return m_double == n.Number();
	if (type == stringValue && n.Type() == stringValue) return m_string == n.String();
	if (type == emptyValue && n.Type() == emptyValue)   return true;
	return false;
}


bool Value::operator!=(const Value& n) const
{
	if (type == boolValue && n.Type() == boolValue)     return m_bool != n.Bool(); 
	if (type == numberValue && n.Type() == numberValue) return m_double != n.Number();
	if (type == stringValue && n.Type() == stringValue) return m_string != n.String();
	// if (type == emptyValue && n.Type() == emptyValue)   return false;
	return false;
}


bool Value::operator<(const Value& n) const
{
	if (type == boolValue && n.Type() == boolValue)     return m_bool < n.Bool(); 
	if (type == numberValue && n.Type() == numberValue) return m_double < n.Number();
	if (type == stringValue && n.Type() == stringValue) return m_string.length() < n.String().length();
	// if (type == emptyValue && n.Type() == emptyValue)   return false;
	return false;
}


bool Value::operator<=(const Value& n) const
{
	if (type == boolValue && n.Type() == boolValue)     return m_bool <= n.Bool(); 
	if (type == numberValue && n.Type() == numberValue) return m_double <= n.Number();
	if (type == stringValue && n.Type() == stringValue) return m_string.length() <= n.String().length();
	if (type == emptyValue && n.Type() == emptyValue)   return true;
	return false;
}


bool Value::operator>(const Value& n) const
{
	if (type == boolValue && n.Type() == boolValue)     return m_bool > n.Bool(); 
	if (type == numberValue && n.Type() == numberValue) return m_double > n.Number();
	if (type == stringValue && n.Type() == stringValue) return m_string.length() > n.String().length();
	// if (type == emptyValue && n.Type() == emptyValue)   return false;
	return false;
}


bool Value::operator>=(const Value& n) const
{
	if (type == boolValue   && n.Type() == boolValue)   return m_bool >= n.Bool(); 
	if (type == numberValue && n.Type() == numberValue) return m_double >= n.Number();
	if (type == stringValue && n.Type() == stringValue) return m_string.length() >= n.String().length();
	if (type == emptyValue  && n.Type() == emptyValue)  return true;
	return false;
}



// private

void Value::init()
{
	type = emptyValue; // init'ed values are empty by default
	m_bool = false;
	m_double = 0;
	m_string = "";
}


