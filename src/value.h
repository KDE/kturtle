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
 
 
 //typedef double Value;
//Value class to store variables ...

#ifndef _VALUE_H_
#define _VALUE_H_

#include <qstring.h>
#include <qtextstream.h>

using namespace std;

class Value {
  public:
    
    //constructor/destructor
    //======================
    Value();
    Value(const Value&);
    ~Value(){}
    
    
    //public members
    //==============
    void toString(); //explicit conversion to string strVal
    void toDouble(); //explicit conversion to double val
    
    //operators
    //=========
    Value& operator=(const Value&);
    Value& operator=(const QString&);
    Value& operator=(double);

    Value& operator+(const Value&);
    Value& operator-(const Value&);
    Value& operator*(const Value&);
    Value& operator/(const Value&);

    bool operator==(const Value&) const;
    bool operator!=(const Value&) const;
    bool operator<(const Value&) const;
    bool operator<=(const Value&) const;
    bool operator>(const Value&) const;
    bool operator>=(const Value&) const;

    //public members  
    bool bString; //set to true if Value represents a string...
    QString strVal;
    double val;

};

#endif // _VALUE_H_
