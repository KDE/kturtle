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

 
#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include "settings.h"

#include "translate.h"



Translate::Translate()
{
	loadTranslations();
}


QString Translate::name2fuzzy(const QString &name)
{
	if ( !aliasMap[name].isEmpty() ) // translate the alias if any
	{
		return QString( i18n("'%1' (%2)").arg(keyMap[name]).arg(reverseAliasMap[name]) );
	}
	return QString( "'" + keyMap[name] + "'");
}

QString Translate::name2key(const QString &name)
{
	return keyMap[name];
}

QString Translate::alias2key(const QString &name)
{
	return aliasMap[name];
}


void Translate::loadTranslations() {
	QDomDocument KeywordsXML;

	kdDebug(0) << "Loading translation dictionary: "<< locate("data", "kturtle/data/logokeywords." + Settings::logoLanguage() + ".xml") <<endl;
  	// Read the specified translation file
	QFile xmlfile( locate("data", "kturtle/data/logokeywords." + Settings::logoLanguage() + ".xml") );

	if ( !xmlfile.open(IO_ReadOnly) ) return;
	
	if ( !KeywordsXML.setContent(&xmlfile) )
	{
		xmlfile.close();
		return;
	}
	xmlfile.close();

	// get into the first child of the root element (in our case a <command> tag)
	QDomElement rootElement = KeywordsXML.documentElement();
	QDomNode n = rootElement.firstChild();

	while ( !n.isNull() )
	{
		QString name, key, alias;
		name = n.toElement().attribute("name"); // get the name attribute of <command>
		QDomNode m = n.firstChild(); // get into the first child of a <command>
		while (true)
		{
			if( !m.toElement().text().isEmpty() )
			{
				if (m.toElement().tagName() == "keyword")
				{
					key = m.toElement().text();
					keyMap.insert(name, key);
				}
				if (m.toElement().tagName() == "alias")
				{
					alias = m.toElement().text();
					aliasMap.insert(alias, key);
					reverseAliasMap.insert(key, alias);
				}
			}
			// break when read the last child of the current <command>
			if ( m == n.lastChild() ) break;
			m = m.nextSibling(); // goto the next element in the current <command>
		}
		n = n.nextSibling(); // goto the next <command>
	}
	
	kdDebug(0) << "Translation dictionary loaded" <<endl;
}

