/*=============================================================================
author        :Walter Schreppers
filename      :parser.h
description   :Parse source code by calling lexer repeadetly and build a tree
               so that it can be executed by Executer
bugreport(log):/
=============================================================================*/
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
 
 
 //typedef double number;
//number class to store variables ...

#ifndef _NUMBER_H_
#define _NUMBER_H_

#include <iostream>
#include <sstream>

#include <qstring.h>

using namespace std;

class Number {
  public:
    
    //constructor/destructor
    //======================
    Number();
    Number(const Number&);
    ~Number(){}
    
    
    //public members
    //==============
    void toString(); //explicit conversion to string strVal
    void toDouble(); //explicit conversion to double val
    
    //operators
    //=========
    Number& operator=(const Number&);
    Number& operator=(const QString&);
    Number& operator=(double);

    Number& operator+(const Number&);
    Number& operator-(const Number&);
    Number& operator*(const Number&);
    Number& operator/(const Number&);

    bool operator==(const Number&) const;
    bool operator!=(const Number&) const;
    bool operator<(const Number&) const;
    bool operator<=(const Number&) const;
    bool operator>(const Number&) const;
    bool operator>=(const Number&) const;

    friend ostream& operator<<(ostream&, const Number&); //outputs double or string
    friend istream& operator>>(istream&, Number&); //reads double
  

    //public members  
    bool bString; //set to true if Number represents a string...
    QString strVal;
    double val;

};

#endif // _NUMBER_H_
