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
 
#include "value.h"


Value::Value() {
  strVal = "";
  val = 0;
  bString = false;
}

Value::Value( const Value& n ) {
  *this = n;
}

void Value::toString() {
  strVal.setNum(val);
  bString = true;
}

void Value::toDouble() {
  val = strVal.toDouble();
  bString = false;
}
    

Value& Value::operator=( const Value& n ) {
  if(this != &n) {

    if( bString == n.bString ) {
      strVal = n.strVal;
      val = n.val;
      bString = n.bString;
    }
    
    if(n.bString) {
      bString=true;
      strVal=n.strVal;
      
      Value tmp;
      tmp.strVal=n.strVal;
      tmp.toDouble();
      val=tmp.val;
    }
    
    if(!n.bString) {
      bString=false;
      val=n.val;
      
      Value tmp;
      tmp.val=n.val;
      tmp.toString();
      strVal=tmp.strVal;
    }

  }
  return *this;
}


Value& Value::operator=( const QString& s ) {
  strVal=s.ascii();
  bString=true;
  return *this;
}


Value& Value::operator=( double n ) {
  val=n;
  bString=false;
  return *this;
}



Value& Value::operator+( const Value& n ) {
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
      Value tmp=n;
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


Value& Value::operator-( const Value& n ) {
  if(!bString && !n.bString){
    val-=n.val;
  }
  else if(bString && n.bString) {
    //strVal-=n.strVal;
    kdDebug(0)<<"cannot subtract strings"<<endl;
  } else {
    val-=n.val;
    toString();
    bString=false;
  }

  return *this;
}


Value& Value::operator*( const Value& n ) {
  if(!bString && !n.bString) {
    val*=n.val;
  }
  else if(bString && n.bString) {
    //strVal*=n.strVal;
    kdDebug(0)<<"cannot multiply strings"<<endl; 
  } else{
    val*=n.val;
    toString();
    bString=false;
  }
  return *this;
}


Value& Value::operator/( const Value& n ) {
  if(!bString && !n.bString) {
    val/=n.val;
  }
  else if(bString && n.bString) {
    //strVal/=n.strVal;
    kdDebug(0)<<"cannot divide strings"<<endl;
  } else{
    val/=n.val;
    toString();
    bString=false;
  }
  return *this;
}



bool Value::operator==( const Value& n ) const {
  if( bString && n.bString ) return strVal==n.strVal; 
  if( !bString && !n.bString ) return val == n.val;
  return false;
}


bool Value::operator!=( const Value& n ) const {
  if( bString && n.bString ) return strVal!=n.strVal; 
  if( !bString && !n.bString ) return val != n.val;
  return false;
}


bool Value::operator<( const Value& n ) const {
  if( bString && n.bString ) return strVal<n.strVal; 
  if( !bString && !n.bString ) return val < n.val;
  return false;
}


bool Value::operator<=( const Value& n ) const {
  if( bString && n.bString ) return strVal<=n.strVal; 
  if( !bString && !n.bString ) return val <= n.val;
  return false;
}


bool Value::operator>( const Value& n ) const {
  if( bString && n.bString ) return strVal>n.strVal; 
  if( !bString && !n.bString ) return val > n.val;
  return false;
}


bool Value::operator>=( const Value& n ) const {
  if( bString && n.bString ) return strVal>=n.strVal; 
  if( !bString && !n.bString ) return val >= n.val;
  return false;
}
