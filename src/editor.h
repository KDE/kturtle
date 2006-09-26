/*
	Copyright (C) 2003-2006 Cies Breijs <cies # kde ! nl>

	This program is free software; you can redistribute it and/or
	modify it under the terms of version 2 of the GNU General Public
	License as published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the Free
	Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.
*/

#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <cmath>

#include <QFrame>
#include <QPixmap>
#include <QTextCursor>
#include <QTextEdit>

#include <kurl.h>

#include "highlighter.h"

class QTextEdit;
class QHBoxLayout;


//BEGIN class LineNumbers

const int LINENUMBER_SPACING = 2;

class LineNumbers : public QWidget
{
	Q_OBJECT

	public:
		LineNumbers(QWidget *parent, QTextEdit *te) : QWidget(parent), editor(te), maxWidth(0) {}
		~LineNumbers() {}
	
		void setFont(const QFont& f) { QWidget::setFont(f); }

		void setWidth(int w) {
			if (w == maxWidth) return;  // save some cpu cycles
			maxWidth = w;
			QString s("");
			for (; w > 0; w--) s += "0";
			setFixedWidth(fontMetrics().width(s) + 2*LINENUMBER_SPACING);
		}

		void paintEvent(QPaintEvent*) {
			QAbstractTextDocumentLayout* layout = editor->document()->documentLayout();
			int contentsY = editor->verticalScrollBar()->value();
			qreal pageBottom = contentsY + editor->viewport()->height();
			const QFontMetrics fm = fontMetrics();
			const int ascent = fontMetrics().ascent() + 1;  // height = ascent + descent + 1
			int lineCount = 1;
			QPainter p(this);
		
			for (QTextBlock block = editor->document()->begin(); block.isValid(); block = block.next(), ++lineCount) {
				const QRectF boundingRect = layout->blockBoundingRect(block);
				QPointF position = boundingRect.topLeft();
				if (position.y() + boundingRect.height() < contentsY) continue;
				if (position.y() > pageBottom) break;
				const QString txt = QString::number(lineCount);
				p.drawText(width() - fm.width(txt) - LINENUMBER_SPACING, qRound(position.y()) - contentsY + ascent, txt);
			}
		}

	private:
		QTextEdit *editor;
		int        maxWidth;
};

//END class LineNumbers


class Editor : public QFrame
{
	Q_OBJECT

	public:
		Editor(QWidget *parent = 0);
		~Editor();

		QTextEdit* view() const { return editor; }
		QTextDocument* document() const { return editor->document(); }

		const KUrl& currentUrl() { return m_currentUrl; }
		void setCurrentUrl(const KUrl& url = KUrl());

		bool maybeSave();

		bool isModified() { return editor->document()->isModified(); }
		QString content() { return editor->document()->toPlainText(); }

// 		void setTranslator(Translator*);

		Token* currentToken(const QString& text, int cursorIndex) { return highlighter->formatType(text, cursorIndex); }


	public slots:
		bool newFile();
		bool openFile(const KUrl &url = KUrl());
		bool saveFile(const KUrl &url = KUrl());
		bool saveFileAs();
		void toggleLineNumbers(bool b) { numbers->setVisible(b); }
		void setModified(bool);
		void setInsertMode(bool b);

		void markCurrentLine();
		void markCurrentWord(int startRow, int startCol, int endRow, int endCol);
		void markCurrentError(int startRow, int startCol, int endRow, int endCol);
		void markChars(const QTextCharFormat& charFormat, int startRow, int startCol, int endRow, int endCol);

	signals:
		void currentUrlChanged(const KUrl&);
		void modificationChanged(bool);
		void cursorPositionChanged(int row, int col, const QString& line);

	protected slots:
		void textChanged(int pos, int added, int removed);
		void cursorPositionChanged();

	private:
		void setContent(const QString&);
		QTextEdit   *editor;
		Highlighter *highlighter;
		QTextCursor  highlight;
		LineNumbers *numbers;
		QHBoxLayout *box;
		KUrl         m_currentUrl;
		int          currentLine;


		QTextBlockFormat defaultBlockFormat;
		QTextBlockFormat currentLineFormat;
		QTextCharFormat defaultCharFormat;
		QTextCharFormat currentWordFormat;
		QTextCharFormat currentErrorFormat;
};


#endif  // _EDITOR_H_
