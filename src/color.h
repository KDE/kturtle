/*
    SPDX-FileCopyrightText: 2026 Omar Tamer <omar.t.shehata@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _COLOR_H_
#define _COLOR_H_

#include <KLocalizedString>
#include <QColor>
#include <QMap>

#include "interpreter/errormsg.h"
#include "interpreter/token.h"

namespace Color
{

static QMap<QString, QColor> colorMap{};

void initColorMap();
bool isValidColorString(QString colorString);
QColor colorString2RGB(QString colorString);
QColor token2RGB(Token *token, ErrorList *errorList);
}

#endif
