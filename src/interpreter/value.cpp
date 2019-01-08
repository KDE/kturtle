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

#include "value.h"

#include <QDebug>
//#include <QLocale>

#include "translator.h"  // for the boolean (true and false) to string translation


Value::Value()
{
	init();
}

Value::Value(Value* n) :
	m_type(n->type()),
	m_bool(n->boolean()),
	m_double(n->number()),
	m_string(n->string())
{
}


void Value::setType(int newType)  // maybe someday we have to do some type casting logic here
{
	if (m_type == newType) {
		return;  // don't change values when type is not changing
	} else {
		switch (newType)
		{
			case Value::Bool:
				init();
				m_type = newType;
				break;
	
			case Value::Number:
				init();
				m_type = newType;
				break;
	
			case Value::String:
				init();
				m_type = newType;
				break;
	
			case Value::Empty:
				init();
				break;
		}
	}
}


bool Value::boolean() const
{
	switch (m_type) {
		case Value::Bool:
			return m_bool;
		case Value::Empty:
			return false;
		case Value::Number:
			return (m_double==0 ? false : true);
	}
	return true;  // numbers and strings
}

void Value::setBool(bool b)
{
	m_type = Value::Bool;
	m_bool = b;
}


double Value::number() const
{
	switch (m_type) {
		case Value::Bool:
			return (m_bool ? 1 : 0);

		case Value::Number:
			return m_double;

		case Value::String:
			bool ok = true;
			double num = m_string.toDouble(&ok);
			if (ok) return num;
	}
	return 0;  // Value::String, Value::Empty
}

void Value::setNumber(double d)
{
	m_type = Value::Number;
	m_double = d;
}

bool Value::setNumber(const QString &s)
{
	m_type = Value::Number;
	bool ok = true;
	double num = s.toDouble(&ok);
	if (ok) {
		m_double = num;
		return true;
	}
	m_double = 0;
	return false;
}


QString Value::string() const
{
	if (m_type == Value::Bool) {
		if (m_bool)
			return QString(Translator::instance()->default2localized(QStringLiteral("true")));
		else
			return QString(Translator::instance()->default2localized(QStringLiteral("false")));
	} else if (m_type == Value::Number) {
		QString s;
		s.setNum(m_double);
		return s;
	}
	return m_string;  // Value::String, Value::Empty
}

void Value::setString(double d)
{
	m_type = Value::String;
	m_double = d;
	m_string.setNum(d);
}

void Value::setString(const QString &s)
{
	m_type = Value::String;
	m_string = s;
}



Value& Value::operator=(Value* n)
{
	switch (n->type()) {
		case Value::Bool:
			setBool(n->boolean());
			break;

		case Value::Number:
			setNumber(n->number());
			break;

		case Value::String:
			setString(n->string());
			break;

		case Value::Empty:
			init();
			break;

		default:
			init();
			break;
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



Value& Value::operator+(Value* n)
{
	if (m_type == Value::Number && n->type() == Value::Number) {
		m_double += n->number();
	} else {
		m_type = Value::String;
		m_string = string() + n->string();
	}
	return *this;
}


Value& Value::operator-(Value* n)
{
	if (m_type == Value::Number && n->type() == Value::Number) {
		m_double -= n->number();
	} else {
		//qDebug() << "cannot subtract strings" << endl;
	}
	return *this;
}


Value& Value::operator*(Value* n)
{
	if (m_type == Value::Number && n->type() == Value::Number) {
		m_double *= n->number();
	} else {
		//qDebug() << "cannot multiply strings" << endl; 
	}
	return *this;
}


Value& Value::operator/(Value* n)
{
	if (m_type == Value::Number && n->type() == Value::Number) {
		m_double /= n->number();
	} else {
		//qDebug() << "cannot divide strings" << endl;
	}
	return *this;
}



bool Value::operator==(Value* n) const
{
	if (m_type == Value::Bool   && n->type() == Value::Bool)   return m_bool == n->boolean();
	if (m_type == Value::Number && n->type() == Value::Number) return m_double == n->number();
	if (m_type == Value::String && n->type() == Value::String) return m_string == n->string();
	if (m_type == Value::Empty  && n->type() == Value::Empty)  return true;
	return false;
}


bool Value::operator!=(Value* n) const
{
	if (m_type == Value::Bool   && n->type() == Value::Bool)   return m_bool != n->boolean();
	if (m_type == Value::Number && n->type() == Value::Number) return m_double != n->number();
	if (m_type == Value::String && n->type() == Value::String) return m_string != n->string();
	// if (m_type == Value::Empty && n->type() == Value::Empty)   return false;
	return false;
}


bool Value::operator<(Value* n) const
{
	if (m_type == Value::Bool   && n->type() == Value::Bool)   return m_bool < n->boolean();
	if (m_type == Value::Number && n->type() == Value::Number) return m_double < n->number();
	if (m_type == Value::String && n->type() == Value::String) return m_string.length() < n->string().length();
	// if (m_type == Value::Empty && n->type() == Value::Empty)   return false;
	return false;
}


bool Value::operator<=(Value* n) const
{
	if (m_type == Value::Bool   && n->type() == Value::Bool)   return m_bool <= n->boolean();
	if (m_type == Value::Number && n->type() == Value::Number) return m_double <= n->number();
	if (m_type == Value::String && n->type() == Value::String) return m_string.length() <= n->string().length();
	if (m_type == Value::Empty  && n->type() == Value::Empty)  return true;
	return false;
}


bool Value::operator>(Value* n) const
{
	if (m_type == Value::Bool   && n->type() == Value::Bool)   return m_bool > n->boolean();
	if (m_type == Value::Number && n->type() == Value::Number) return m_double > n->number();
	if (m_type == Value::String && n->type() == Value::String) return m_string.length() > n->string().length();
	// if (m_type == Value::Empty && n->type() == Value::Empty)   return false;
	return false;
}


bool Value::operator>=(Value* n) const
{
	if (m_type == Value::Bool   && n->type() == Value::Bool)   return m_bool >= n->boolean();
	if (m_type == Value::Number && n->type() == Value::Number) return m_double >= n->number();
	if (m_type == Value::String && n->type() == Value::String) return m_string.length() >= n->string().length();
	if (m_type == Value::Empty  && n->type() == Value::Empty)  return true;
	return false;
}



// private

void Value::init()
{
	m_type = Value::Empty;  // init'ed values are empty by default
	m_bool = false;
	m_double = 0;
	m_string = QLatin1String("");
}


