/*
	SPDX-FileCopyrightText: 2003-2008 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "colorpicker.h"
#include "interpreter/translator.h"  // for getting the translated ArgumentSeparator

#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KGuiItem>


ColorPicker::ColorPicker(QWidget* parent)
	: QDialog(parent)
{
	setWindowTitle(i18n("Color Picker"));
	setModal(false);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	setLayout(mainLayout);

	QWidget* baseWidget = new QWidget(this);
	mainLayout->addWidget(baseWidget);

	QVBoxLayout* baseLayout = new QVBoxLayout;
    baseLayout->setContentsMargins(0, 0, 0, 0);
    baseWidget->setLayout( baseLayout );
	QGridLayout* gridLayout = new QGridLayout;
	baseLayout->addLayout(gridLayout);

	redSlider = new QSlider(Qt::Horizontal, this);
	redSlider->setMaximum(255);
	redSpin = new QSpinBox(this);
	redSpin->setMaximum(255);
	QLabel* redLabel = new QLabel(i18n("Amount red:"));
	redLabel->setBuddy(redSpin);
	gridLayout->addWidget(redLabel, 0, 0);
	gridLayout->addWidget(redSlider, 0, 1);
	gridLayout->addWidget(redSpin, 0, 2);
	connect(redSlider, &QSlider::valueChanged, redSpin, &QSpinBox::setValue);
	connect(redSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), redSlider, &QSlider::setValue);
	connect(redSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ColorPicker::redChanged);


	greenSlider = new QSlider(Qt::Horizontal, this);
	greenSlider->setMaximum(255);
	greenSpin = new QSpinBox(this);
	greenSpin->setMaximum(255);
	QLabel* greenLabel = new QLabel(i18n("Amount green:"));
	greenLabel->setBuddy(greenSpin);
	gridLayout->addWidget(greenLabel, 1, 0);
	gridLayout->addWidget(greenSlider, 1, 1);
	gridLayout->addWidget(greenSpin, 1, 2);
	connect(greenSlider, &QSlider::valueChanged, greenSpin, &QSpinBox::setValue);
	connect(greenSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), greenSlider, &QSlider::setValue);
	connect(greenSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ColorPicker::greenChanged);

	blueSlider = new QSlider(Qt::Horizontal, this);
	blueSlider->setMaximum(255);
	blueSpin = new QSpinBox(this);
	blueSpin->setMaximum(255);
    QLabel* blueLabel = new QLabel(i18n("Amount blue:"));
	blueLabel->setBuddy(blueSpin);
	gridLayout->addWidget(blueLabel, 2, 0);
	gridLayout->addWidget(blueSlider, 2, 1);
	gridLayout->addWidget(blueSpin, 2, 2);
	connect(blueSlider, &QSlider::valueChanged, blueSpin, &QSpinBox::setValue);
	connect(blueSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), blueSlider, &QSlider::setValue);
	connect(blueSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ColorPicker::blueChanged);

	baseLayout->addSpacing(20);

	colorPatch = new ColorPatch(this);
	colorPatch->setMinimumWidth(100);
	colorPatch->setMinimumHeight(60);
	baseLayout->addWidget(colorPatch);

	baseLayout->addSpacing(20);

	QHBoxLayout* resultLayout = new QHBoxLayout;
	resultLayout->addStretch();

	baseLayout->addLayout(resultLayout);
	resultBox = new QLineEdit(this);
	resultBox->setReadOnly(true);
	resultBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	int width = QFontMetrics(QFontDatabase::systemFont(QFontDatabase::FixedFont)).boundingRect(QStringLiteral("255, 255, 255_000")).width();
	resultBox->setMinimumWidth(width);
	resultBox->setMaximumWidth(width);
	resultLayout->addWidget(resultBox);

	QPushButton* copyButton = new QPushButton(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("&Copy to clipboard"), baseWidget);
	mainLayout->addWidget(copyButton);
	resultLayout->addWidget(copyButton);
	connect(copyButton, &QPushButton::clicked, this, &ColorPicker::copyProxy);

	QPushButton* pasteButton = new QPushButton(QIcon::fromTheme(QStringLiteral("edit-paste")), i18n("&Paste to editor"), baseWidget);
	mainLayout->addWidget(pasteButton);
	resultLayout->addWidget(pasteButton);
	connect(pasteButton, &QPushButton::clicked, this, &ColorPicker::pasteProxy);

	QDialogButtonBox *buttonBox = new QDialogButtonBox();
	QWidget *mainWidget = new QWidget(this);
	mainLayout->addWidget(mainWidget);

	QPushButton *user1Button = new QPushButton;
	buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &ColorPicker::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &ColorPicker::reject);

	mainLayout->addWidget(buttonBox);
	user1Button->setDefault(true);
	KGuiItem::assign(user1Button, KStandardGuiItem::close());
	connect(user1Button, &QPushButton::clicked, this, &ColorPicker::hide);

	resultLayout->addStretch();

	updateResult(0, 0, 0);
}

void ColorPicker::redChanged(int value)
{
	updateResult(value, greenSlider->value(), blueSlider->value());
}

void ColorPicker::greenChanged(int value)
{
	updateResult(redSlider->value(), value, blueSlider->value());
}

void ColorPicker::blueChanged(int value)
{
	updateResult(redSlider->value(), greenSlider->value(), value);
}

void ColorPicker::updateResult(int r, int g, int b)
{
	QString separator(Translator::instance()->default2localized(QStringLiteral(",")));
	resultBox->setText(QStringLiteral("%2%1 %3%1 %4 \n").arg(separator).arg(r).arg(g).arg(b));
	colorPatch->setColor(QColor(r, g, b));
	colorPatch->repaint();
}

void ColorPicker::copyProxy()
{
	QApplication::clipboard()->setText(resultBox->text());
}

void ColorPicker::pasteProxy()
{
	emit pasteText(resultBox->text());
}
