/*
    Copyright (C) 2003 by Walter Schreppers 
    Copyright (C) 2004 by Cies Breijs   
     
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

    
#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <qstring.h>

#include "value.h"


struct Pos // convenience, it codes a bit nicer with this, i.e.: "int row = tok.start.row"
{
	uint     row;
	uint     col;
};

struct Token
{
	int      type;  // filled with enumed tokenTypes
	QString  look;  // the way the token looks in the text editor
	Value    value; // if the token is a number it can be stored here
	Pos      start; // row'n'col of the starting/ending point of the token
	Pos      end;
};


enum tokenTypes
{
	tokNotSet  = -1, // inittial type of all tokens
	tokError   = 1,  // when there is an error
	tokUnknown = 0,  // when no token was found, a tokUnknown is often a variable, function or error
	
	tokIf, // the execution controlling tokens
	tokElse,
	tokWhile,
	tokFor,
	tokTo,
	tokStep,
	tokForEach,
	tokIn,
	tokBreak,
	tokReturn,

	tokBegin, // the scope delimiting tokens
	tokEnd,

	tokNumber, // the containers
	tokString,

	tokAssign, // for assignments
	
	tokAnd, // and, or, not
	tokOr,
	tokNot,
	
	tokEq, // ==, !=, >=, >, <=, <
	tokNe,
	tokGe,
	tokGt,
	tokLe,
	tokLt,
	
	tokBraceOpen, // (, ), +, -, *, /
	tokBraceClose,
	tokPlus,
	tokMinus,
	tokMul,
	tokDev,
	
	tokComma,
	
	tokEOL, // End Of Line token
	tokEOF, // End Of File token

	tokLearn, // learn command
	
	tokClear, // the 'regular' command tokens
	tokGo,
	tokGoX,
	tokGoY,
	tokForward,
	tokBackward,
	tokDirection,
	tokTurnLeft,
	tokTurnRight,
	tokCenter,
	tokSetPenWidth,
	tokPenUp,
	tokPenDown,
	tokSetFgColor,
	tokSetBgColor,
	tokResizeCanvas,
	tokSpriteShow,
	tokSpriteHide,
	tokSpritePress,
	tokSpriteChange,
	tokMessage,
	tokInputWindow,
	tokPrint,
	tokFontType,
	tokFontSize,
	tokRepeat,
	tokRandom,
	tokWait,
	tokWrapOn,
	tokWrapOff,
	tokReset,
	tokRun,

	tokDo // a dummy command
};


#endif // _TOKEN_H_
