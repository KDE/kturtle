/*
    SPDX-FileCopyrightText: 2003-2008 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "errordialog.h"

#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <KGuiItem>
#include <KHelpClient>
#include <KLocalizedString>

ErrorDialog::ErrorDialog(QWidget *parent)
    : QDialog(parent)
{
    errorList = nullptr;

    setWindowTitle(i18nc("@title:window", "Errors"));
    setModal(false);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QWidget *mainWidget = new QWidget(this);
    mainLayout->addWidget(mainWidget);

    QWidget *baseWidget = new QWidget(this);
    mainLayout->addWidget(baseWidget);
    baseLayout = new QVBoxLayout(baseWidget);
    mainLayout->addWidget(baseWidget);

    label = new QLabel(baseWidget);
    mainLayout->addWidget(label);
    label->setText(i18n("In this list you find the error(s) that resulted from running your code.\nGood luck!"));
    // \nYou can select an error and click the 'Help on Error' button for help.
    label->setScaledContents(true);
    baseLayout->addWidget(label);

    spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    baseLayout->addItem(spacer);

    errorTable = new QTableWidget(baseWidget);
    mainLayout->addWidget(errorTable);
    errorTable->setSelectionMode(QAbstractItemView::SingleSelection);
    errorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    errorTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    errorTable->setShowGrid(false);

    errorTable->setColumnCount(3);
    QStringList horizontalHeaderTexts;
    horizontalHeaderTexts << i18n("line") << i18n("description") << i18n("code");
    errorTable->setHorizontalHeaderLabels(horizontalHeaderTexts);
    errorTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    baseLayout->addWidget(errorTable);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Help);
    QPushButton *user1Button = new QPushButton;
    m_buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &ErrorDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &ErrorDialog::reject);
    connect(m_buttonBox, &QDialogButtonBox::helpRequested, this, &ErrorDialog::helpRequested);
    mainLayout->addWidget(m_buttonBox);
    KGuiItem::assign(user1Button, KGuiItem(i18n("Hide Errors")));
    // 	setButtonGuiItem(User1, i18n("Help on &Error"));  // TODO context help in the error dialog
    user1Button->setDefault(true);

    clear();
}

void ErrorDialog::clear()
{
    disable();
    errorList = nullptr;
    errorTable->clearContents();

    // put a friendly 'nothing to see here' notice in the empty table
    errorTable->setRowCount(1);
    QTableWidgetItem *emptyItem = new QTableWidgetItem(i18n("No errors occurred yet."));
    QFont emptyFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    emptyFont.setItalic(true);
    emptyItem->setFont(emptyFont);
    errorTable->setItem(0, 1, emptyItem);

    errorTable->resizeColumnsToContents();
    // 	errorTable->setColumnWidth(0, errorTable->fontMetrics().width("88888"));
    // 	errorTable->setColumnWidth(2, errorTable->fontMetrics().width("88888"));
    // 	errorTable->setColumnWidth(1, errorTable->width() - errorTable->verticalHeader()->width() - errorTable->columnWidth(0) - errorTable->columnWidth(2));
}

void ErrorDialog::enable()
{
    Q_ASSERT(errorList);
    errorTable->setEnabled(true);
    m_buttonBox->button(QDialogButtonBox::Help)->setEnabled(true);
    connect(errorTable, &QTableWidget::itemSelectionChanged, this, &ErrorDialog::selectedErrorChangedProxy);
    errorTable->selectRow(0);
}

void ErrorDialog::disable()
{
    disconnect(errorTable, &QTableWidget::itemSelectionChanged, this, &ErrorDialog::selectedErrorChangedProxy);
    errorTable->setEnabled(false);
    m_buttonBox->button(QDialogButtonBox::Help)->setEnabled(false);
    errorTable->clearSelection();
}

void ErrorDialog::setErrorList(ErrorList *list)
{
    errorList = list;
    errorTable->setRowCount(errorList->size());
    int row = 0;
    for (const ErrorMessage &error : *errorList) {
        int col = 0;
        QStringList itemTexts;
        itemTexts << QString::number(error.token().startRow()) << error.text() << QString::number(error.code());
        for (const QString &itemText : std::as_const(itemTexts)) {
            errorTable->setItem(row, col, new QTableWidgetItem(itemText));
            col++;
        }
        row++;
    }
    errorTable->clearSelection();
    errorTable->resizeColumnsToContents();
    enable();
}

void ErrorDialog::selectedErrorChangedProxy()
{
    Q_ASSERT(errorList);
    const Token *t = &errorList->at(errorTable->selectedItems().first()->row()).token();
    Q_EMIT currentlySelectedError(t->startRow(), t->startCol(), t->endRow(), t->endCol());
    // //qDebug() << "EMITTED: " << t->startRow() << ", " << t->startCol() << ", " << t->endRow() << ", " << t->endCol();
}

void ErrorDialog::helpRequested()
{
    KHelpClient::invokeHelp(QStringLiteral("reference"), QStringLiteral("kturtle"));
}

#include "moc_errordialog.cpp"
