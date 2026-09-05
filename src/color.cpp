#include "color.h"

QColor Color::colorString2RGB(QString colorString)
{
    // default to white
    QColor color = QColorConstants::White;
    if (isValidColorString(colorString)) {
        return colorMap[colorString];
    }
    return color;
}

bool Color::isValidColorString(QString colorString)
{
    static bool colorMapInitialized = false;
    if (!colorMapInitialized) {
        colorMapInitialized = true;
        initColorMap();
    }

    if (colorMap.contains(colorString)) {
        return true;
    }
    return false;
}

QColor Color::token2RGB(Token *token, ErrorList *errorList)
{
    QString colorString = token->look();
    colorString.removeFirst(); // removing quotes
    colorString.removeLast();
    QColor color = QColorConstants::White;
    if (isValidColorString(colorString)) {
        return colorMap[colorString];
    } else {
        errorList->addError(i18n("%1 is not a valid color.", colorString), *token, 20000 + Token::Color * 100 + 90);
    }
    return color;
}

void Color::initColorMap()
{
    colorMap[i18n("white")] = QColorConstants::Svg::white;

    colorMap[i18n("black")] = QColorConstants::Svg::black;

    colorMap[i18n("grey")] = QColorConstants::Svg::gray;

    colorMap[i18n("gray")] = QColorConstants::Svg::gray;

    colorMap[i18n("red")] = QColorConstants::Svg::red;

    colorMap[i18n("brown")] = QColorConstants::Svg::brown;

    colorMap[i18n("orange")] = QColorConstants::Svg::orange;

    colorMap[i18n("yellow")] = QColorConstants::Svg::yellow;

    colorMap[i18n("gold")] = QColorConstants::Svg::gold;

    colorMap[i18n("green")] = QColorConstants::Svg::green;

    colorMap[i18n("blue")] = QColorConstants::Svg::blue;

    colorMap[i18n("purple")] = QColorConstants::Svg::purple;

    colorMap[i18n("pink")] = QColorConstants::Svg::pink;
}
