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

#include "number.h"


Number::Number() {
  strVal = "";
  val = 0;
  bString = false;
}

Number::Number( const Number& n ) {
  *this = n;
}

void Number::toString() {
  ostringstream os;
  os << val;
  strVal = os.str();
  bString = true;
}

void Number::toDouble() {
  istringstream is(strVal);
  is >> val;
  bString = false;
}
    

Number& Number::operator=( const Number& n ) {
  if(this != &n) {

    if( bString == n.bString ) {
      strVal = n.strVal;
      val = n.val;
      bString = n.bString;
    }
    
    if(n.bString) {
      bString=true;
      strVal=n.strVal;
      
      Number tmp;
      tmp.strVal=n.strVal;
      tmp.toDouble();
      val=tmp.val;
    }
    
    if(!n.bString) {
      bString=false;
      val=n.val;
      
      Number tmp;
      tmp.val=n.val;
      tmp.toString();
      strVal=tmp.strVal;
    }

  }
  return *this;
}


Number& Number::operator=( const QString& s ) {
  strVal=s.ascii();
  bString=true;
  return *this;
}


Number& Number::operator=( double n ) {
  val=n;
  bString=false;
  return *this;
}



Number& Number::operator+( const Number& n ) {
  if(!bString && !n.bString){
    val+=n.val;
  }
  else if(bString && n.bString) {
    strVal+=n.strVal;
  }
  else{
    if(!bString) {
      bool b=bString; 
      toString();
      bString=b;
    }
    
    if(!n.bString) {
      Number tmp=n;
      tmp.toString();
      strVal+=tmp.strVal;
    }
    else{
      strVal+=n.strVal;
    }
    
    val+=n.val;
  }
  return *this;
}


Number& Number::operator-( const Number& n ) {
  if(!bString && !n.bString){
    val-=n.val;
  }
  else if(bString && n.bString) {
    //strVal-=n.strVal;
    cerr<<"cannot subtract strings"<<endl;
  } else {
    val-=n.val;
    toString();
    bString=false;
  }

  return *this;
}


Number& Number::operator*( const Number& n ) {
  if(!bString && !n.bString) {
    val*=n.val;
  }
  else if(bString && n.bString) {
    //strVal*=n.strVal;
    cerr<<"cannot multiply strings"<<endl;
  } else{
    val*=n.val;
    toString();
    bString=false;
  }
  return *this;
}


Number& Number::operator/( const Number& n ) {
  if(!bString && !n.bString) {
    val/=n.val;
  }
  else if(bString && n.bString) {
    //strVal/=n.strVal;
    cerr<<"cannot divide strings"<<endl;
  } else{
    val/=n.val;
    toString();
    bString=false;
  }
  return *this;
}



bool Number::operator==( const Number& n ) const {
  if( bString && n.bString ) return strVal==n.strVal; 
  if( !bString && !n.bString ) return val == n.val;
  return false;
}


bool Number::operator!=( const Number& n ) const {
  if( bString && n.bString ) return strVal!=n.strVal; 
  if( !bString && !n.bString ) return val != n.val;
  return false;
}


bool Number::operator<( const Number& n ) const {
  if( bString && n.bString ) return strVal<n.strVal; 
  if( !bString && !n.bString ) return val < n.val;
  return false;
}


bool Number::operator<=( const Number& n ) const {
  if( bString && n.bString ) return strVal<=n.strVal; 
  if( !bString && !n.bString ) return val <= n.val;
  return false;
}


bool Number::operator>( const Number& n ) const {
  if( bString && n.bString ) return strVal>n.strVal; 
  if( !bString && !n.bString ) return val > n.val;
  return false;
}


bool Number::operator>=( const Number& n ) const {
  if( bString && n.bString ) return strVal>=n.strVal; 
  if( !bString && !n.bString ) return val >= n.val;
  return false;
}



//output double or string
ostream& operator<<(ostream& os, const Number& n) {
  if(n.bString){
    os<<n.strVal;
  } else {
    os<<n.val;
  }
  return os;
}

//read double
istream& operator>>(istream& is, Number& n) {
  n.bString=false;
  is>>n.val;
  return is;
}
