/*
    SPDX-FileCopyrightText: 2003-2008 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <QWidgetAction>

class QComboBox;


class Console : public QWidgetAction
{
	Q_OBJECT

	public:
        explicit Console(QWidget *parent = nullptr);

		void disable();
		void enable();

		void showError(const QString&);

	public Q_SLOTS:
		void run();
		void clearMarkings();
		void executeActionTriggered();

	Q_SIGNALS:
		QString execute(const QString&);

	private:
		QComboBox *comboBox;
		QWidget   *baseWidget;
};

#endif  // _CONSOLE_H_
