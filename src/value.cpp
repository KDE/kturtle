/*
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
	m_double = d;
	m_string.setNum(d);
}

bool Value::setNumber(QString s)
{
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
	m_double = d;
	m_string.setNum(d);
}

void Value::setString(QString s)
{
	m_string = s;
}



Value& Value::operator=(const Value& n)
{
	if (this != &n)
	{
		if (type == n.type)
		{
			m_bool = n.Bool();
			m_double = n.Number();
			m_string = n.String();
		}
		else if (n.Type() == boolType)
		{
			setType(boolType);
			setBool( n.Bool() );
		}
		else if (n.Type() == numberType)
		{
			setType(numberType);
			setNumber( n.Number() );
		}
		else if (n.Type() == stringType)
		{
			setType(stringType);
			setString( n.String() );
		}
	}
	return *this;
}


Value& Value::operator=(const QString& s)
{
	setType(stringType);
	setString(s);
	return *this;
}

Value& Value::operator=(double n)
{
	setType(numberType);
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


// privates!

void Value::init()
{
	m_bool = false;
	m_double = 0;
	m_string = "";
}






// Value& Value::operator=( const Value& n ) {
// if(this != &n) {
// 
// 	if( bString == n.bString ) {
// 		strVal = n.strVal;
// 		val = n.val;
// 		bString = n.bString;
// 	}
// 	
// 	if(n.bString) {
// 		bString=true;
// 		strVal=n.strVal;
// 		
// 		Value tmp;
// 		tmp.strVal=n.strVal;
// 		tmp.toDouble();
// 		val=tmp.val;
// 	}
// 	
// 	if(!n.bString) {
// 		bString=false;
// 		val=n.val;
// 		
// 		Value tmp;
// 		tmp.val=n.val;
// 		tmp.toString();
// 		strVal=tmp.strVal;
// 	}
// 
// }
// return *this;
// }
// 
// 
// Value& Value::operator=( const QString& s ) {
// strVal=s.ascii();
// bString=true;
// return *this;
// }
// 
// 
// Value& Value::operator=( double n ) {
// val=n;
// bString=false;
// return *this;
// }
// 
// 
// 
// Value& Value::operator+( const Value& n ) {
// if(!bString && !n.bString){
// 	val+=n.val;
// }
// else if(bString && n.bString) {
// 	strVal+=n.strVal;
// }
// else{
// 	if(!bString) {
// 		bool b=bString; 
// 		toString();
// 		bString=b;
// 	}
// 	
// 	if(!n.bString) {
// 		Value tmp=n;
// 		tmp.toString();
// 		strVal+=tmp.strVal;
// 	}
// 	else{
// 		strVal+=n.strVal;
// 	}
// 	
// 	val+=n.val;
// }
// return *this;
// }
// 
// 
// Value& Value::operator-( const Value& n ) {
// if(!bString && !n.bString){
// 	val-=n.val;
// }
// else if(bString && n.bString) {
// 	//strVal-=n.strVal;
// 	kdDebug(0)<<"cannot subtract strings"<<endl;
// } else {
// 	val-=n.val;
// 	toString();
// 	bString=false;
// }
// 
// return *this;
// }
// 
// 
// Value& Value::operator*( const Value& n ) {
// if(!bString && !n.bString) {
// 	val*=n.val;
// }
// else if(bString && n.bString) {
// 	//strVal*=n.strVal;
// 	kdDebug(0)<<"cannot multiply strings"<<endl; 
// } else{
// 	val*=n.val;
// 	toString();
// 	bString=false;
// }
// return *this;
// }
// 
// 
// Value& Value::operator/( const Value& n ) {
// if(!bString && !n.bString) {
// 	val/=n.val;
// }
// else if(bString && n.bString) {
// 	//strVal/=n.strVal;
// 	kdDebug(0)<<"cannot divide strings"<<endl;
// } else{
// 	val/=n.val;
// 	toString();
// 	bString=false;
// }
// return *this;
// }
// 
// 
// 
// bool Value::operator==( const Value& n ) const {
// if( bString && n.bString ) return strVal==n.strVal; 
// if( !bString && !n.bString ) return val == n.val;
// return false;
// }
// 
// 
// bool Value::operator!=( const Value& n ) const {
// if( bString && n.bString ) return strVal!=n.strVal; 
// if( !bString && !n.bString ) return val != n.val;
// return false;
// }
// 
// 
// bool Value::operator<( const Value& n ) const {
// if( bString && n.bString ) return strVal<n.strVal; 
// if( !bString && !n.bString ) return val < n.val;
// return false;
// }
// 
// 
// bool Value::operator<=( const Value& n ) const {
// if( bString && n.bString ) return strVal<=n.strVal; 
// if( !bString && !n.bString ) return val <= n.val;
// return false;
// }
// 
// 
// bool Value::operator>( const Value& n ) const {
// if( bString && n.bString ) return strVal>n.strVal; 
// if( !bString && !n.bString ) return val > n.val;
// return false;
// }
// 
// 
// bool Value::operator>=( const Value& n ) const {
// if( bString && n.bString ) return strVal>=n.strVal; 
// if( !bString && !n.bString ) return val >= n.val;
// return false;
// }

