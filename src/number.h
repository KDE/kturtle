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
