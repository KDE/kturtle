/*
 * KTurtle, Copyright (C) 2003-04 Cies Breijs <cies # kde ! nl>

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

#ifndef _CONSTANT_H_
#define _CONSTANT_H_

# include <qstring.h>


enum ConstantType {
	undefined = -1,
	stringConstant,
	intConstant,
	floatConstant
};
 
class Constant {
	public:
	Constant();
	~Constant() {}

	bool isString();
	bool isInt();
	bool isFloat();

	QString getString();
	int getInt();
	float getFloat();
	ConstantType getType();

	void setString(QString);
	void setInt(int);
	void setFloat(float);
	bool setType(ConstantType);

	private:    
	QString       stringContent;
	int           intContent;
	float         floatContent;
	ConstantType  type;
};

#endif // _CONSTANT_H_
