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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
	if (type == newType) type = newType; // dont change values when type is not changing
	else if (newType == boolType)
	{
		init();
		type = newType;
		m_string = i18n("false");
	}
	else if (newType == numberType)
	{
		init();
		type = newType;
	}
	else if (newType == stringType)
	{
		init();
		type = newType;
	}
}


bool Value::Bool() const
{
	if (type == boolType) return m_bool;
	else if (type == numberType) return (m_double > 0);
	return true;
}

void Value::setBool(bool b)
{
	type = boolType;
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
	if (type == boolType)
	{
		if (m_bool) return 1;
		else return 0;
	}
	else if (type == numberType) return m_double;
	// else if (type == stringType)
	return 0;
}

void Value::setNumber(double d)
{
	type = numberType;
	m_double = d;
	m_string.setNum(d);
}

bool Value::setNumber(QString s)
{
	type = numberType;
	bool ok = true;
	double d = s.toDouble(&ok);
	if (ok)
	{
		m_double = d;
		m_string.setNum(d);
		return true;
	}
	else return false; 
}


QString Value::String() const
{
	if (type == boolType)
	{
		if (m_bool) return QString( i18n("true") );
		else return QString( i18n("false") );
	}
	else if (type == numberType)
	{
		QString s;
		s.setNum(m_double);
		return s;
	}
	//else if (type == stringType)
	return m_string;
}

void Value::setString(double d)
{
	type = stringType;
	m_double = d;
	m_string.setNum(d);
}

void Value::setString(QString s)
{
	type = stringType;
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
		else if (n.Type() == boolType)
		{
			setBool( n.Bool() );
		}
		else if (n.Type() == numberType)
		{
			setNumber( n.Number() );
		}
		else if (n.Type() == stringType)
		{
			setString( n.String() );
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
	if (type == numberType && n.Type() == numberType)
	{
		m_double += n.Number();
	}
	else
	{
		type = stringType;
		m_string = String() + n.String();
	}
	return *this;
}


Value& Value::operator-(const Value& n)
{
	if (type == numberType && n.Type() == numberType)
	{
		m_double -= n.Number();
	}
	else
	{
		kdDebug(0)<<"cannot subtract strings"<<endl;
	}
	return *this;
}


Value& Value::operator*(const Value& n)
{
	if (type == numberType && n.Type() == numberType)
	{
		m_double *= n.Number();
	}
	else
	{
		kdDebug(0)<<"cannot multiply strings"<<endl; 
	}
	return *this;
}


Value& Value::operator/(const Value& n)
{
	if (type == numberType && n.Type() == numberType)
	{
		m_double /= n.Number();
	}
	else
	{
		kdDebug(0)<<"cannot divide strings"<<endl;
	}
	return *this;
}



bool Value::operator==(const Value& n) const
{
	if (type == boolType && n.Type() == boolType)     return m_bool == n.Bool(); 
	if (type == numberType && n.Type() == numberType) return m_double == n.Number();
	if (type == stringType && n.Type() == stringType) return m_string == n.String();
	return false;
}


bool Value::operator!=(const Value& n) const
{
	if (type == boolType && n.Type() == boolType)     return m_bool != n.Bool(); 
	if (type == numberType && n.Type() == numberType) return m_double != n.Number();
	if (type == stringType && n.Type() == stringType) return m_string != n.String();
	return false;
}


bool Value::operator<(const Value& n) const
{
	if (type == boolType && n.Type() == boolType)     return m_bool < n.Bool(); 
	if (type == numberType && n.Type() == numberType) return m_double < n.Number();
	if (type == stringType && n.Type() == stringType) return m_string.length() < n.String().length();
	return false;
}


bool Value::operator<=(const Value& n) const
{
	if (type == boolType && n.Type() == boolType)     return m_bool <= n.Bool(); 
	if (type == numberType && n.Type() == numberType) return m_double <= n.Number();
	if (type == stringType && n.Type() == stringType) return m_string.length() <= n.String().length();
	return false;
}


bool Value::operator>(const Value& n) const
{
	if (type == boolType && n.Type() == boolType)     return m_bool > n.Bool(); 
	if (type == numberType && n.Type() == numberType) return m_double > n.Number();
	if (type == stringType && n.Type() == stringType) return m_string.length() > n.String().length();
	return false;
}


bool Value::operator>=(const Value& n) const
{
	if (type == boolType && n.Type() == boolType)     return m_bool >= n.Bool(); 
	if (type == numberType && n.Type() == numberType) return m_double >= n.Number();
	if (type == stringType && n.Type() == stringType) return m_string.length() >= n.String().length();
	return false;
}



// private

void Value::init()
{
	type = numberType; // init'ed values are numbers by default
	m_bool = false;
	m_double = 0;
	m_string = "";
}


