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

#include "constant.h"

 
Constant::Constant() {
	stringContent = "";
	intContent = 0;
	floatContent = 0;
	type = undefined;
}

bool Constant::isString() {
	if (type == stringConstant) {
		return true;
	} else {
		return false;
	}
}

bool Constant::isInt() {
	if (type == intConstant) {
		return true;
	} else {
		return false;
	}
}

bool Constant::isFloat() {
	if (type == floatConstant) {
		return true;
	} else {
		return false;
	}
}



QString Constant::getString() {
	QString r;
	if (type == intConstant) {
		r.setNum(intContent);
		return r;
	}
	if (type == floatConstant) {
		r.setNum(floatContent);
		return r;
	}
	return stringContent;
}

int Constant::getInt() {
	int r = intContent;
	if (type == floatConstant) {
		r = (int)(floatContent + .5);
	}
	return r;
}

float Constant::getFloat() {
	float r = intContent;
	if (type == intConstant) {
		r = (float)(intContent);
	}
	return r;
}

ConstantType Constant::getType() {
	return type;
}


void Constant::setString(QString s) {
	stringContent = s;
	intContent = 0;
	floatContent = 0;
	type = stringConstant;
}

void Constant::setInt(int i) {
	intContent = i;
	stringContent = "";
	floatContent = 0;
	type = intConstant;
}

void Constant::setFloat(float f) {
	floatContent = f;
	stringContent = "";
	intContent = 0;
	type = floatConstant;
}

bool Constant::setType(ConstantType t) {
	// Only string cannot be converted to int/float's so then return false...
	if (type == stringConstant && ( t == intConstant || t == floatConstant ) ) {
		return false;
	}
	if (t == stringConstant && ( type == intConstant || type == floatConstant ) ) {
		if (t == intConstant) {
			stringContent.setNum(intContent);
		}
		if (t == floatConstant) {
			stringContent.setNum(floatContent);
		}
		return true;
	}
	if (type == intConstant && t == floatConstant ) {
		floatContent = (float)intContent;
		type = floatConstant;
		return true;
	}
	if (type == floatConstant && t == intConstant ) {
		intContent = (int)(floatContent + .5);
		type = intConstant;
		return true;
	}
	return false; // fallback
}
