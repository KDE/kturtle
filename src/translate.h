/*
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

    
#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

#include <qmap.h>
#include <qstring.h>


class Translate
{
	public:
		Translate();
		~Translate() {}
		
		QString name2fuzzy(const QString&);
		QString name2key(const QString&);
		QString alias2key(const QString&);


	private:
		void loadTranslations();
		
		typedef QMap<QString, QString> StringMap;
		StringMap keyMap;
		StringMap aliasMap;
		StringMap reverseAliasMap;
};


#endif // _TRANSLATE_H_


