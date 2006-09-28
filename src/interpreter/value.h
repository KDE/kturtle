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


#ifndef _VALUE_H_
#define _VALUE_H_

#include <QString>



// maybe this class has to be slipt-up/sub-classed into (several) Constant's and a Variable class
// which is a lot of work for little gain :)

/**
 * @short An object for the dynamic variable value in the KTurtle language.
 *
 * This object represents a variable in the KTurtle programming language.
 * It can have 4 types: Empty (NULL), Bool (boolean), Number (double) and
 * String (QString).
 * It can easily be switched between types.
 *
 * By default a Value is Empty.
 *
 * @TODO a way to easily generate ErrorMessages from this class
 *
 * @author Cies Breijs
 */
class Value
{
	public:
		enum Type
		{
			Empty,
			Bool,
			Number,
			String
		};

		Value();
		Value(Value*);

		Value(bool b)           { setBool(b); }
		Value(double d)         { setNumber(d); }
		Value(const QString& s) { setString(s); }
		~Value() {}
	
		void     resetValue() { init(); }
		
		int      type() const { return m_type; }
		void     setType(int);
	
		bool     boolean() const;
		void     setBool(bool);
	
		double   number() const;
		void     setNumber(double);
		bool     setNumber(const QString&);
	
		QString  string() const;
		void     setString(double);
		void     setString(const QString&);
	
		Value&   operator=(Value*);
		Value&   operator=(const QString&);
		Value&   operator=(double);
	
		Value&   operator+(Value*);
		Value&   operator-(Value*);
		Value&   operator*(Value*);
		Value&   operator/(Value*);
	
		bool     operator==(Value*) const;
		bool     operator!=(Value*) const;
		bool     operator< (Value*) const;
		bool     operator<=(Value*) const;
		bool     operator> (Value*) const;
		bool     operator>=(Value*) const;
		
		
	private:
		void     init();
		
		int      m_type;
		bool     m_bool;
		double   m_double;
		QString  m_string;
};

#endif  // _VALUE_H_
