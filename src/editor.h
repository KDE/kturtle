/*
    SPDX-FileCopyrightText: 2003-2008 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <cmath>

#include <QAbstractTextDocumentLayout>
#include <QFrame>
#include <QPainter>
#include <QScrollBar>
#include <QTextEdit>

#include <KFindDialog>

#include "highlighter.h"
#include "interpreter/token.h"
#include "interpreter/tokenizer.h"
#include "interpreter/treenode.h"

class QHBoxLayout;


static const QColor LINE_HIGHLIGHT_COLOR(239, 247, 255);
static const QColor WORD_HIGHLIGHT_COLOR(255, 255, 156);
static const QColor ERROR_HIGHLIGHT_COLOR(255, 200, 200);

static const int EXTRA_SATURATION = 30;   // used for drawing the highlighted background
static const int EDITOR_MARGIN = 2;       // some margin that can't be set to zero, yet painters should know it
static const int CURSOR_RECT_MARGIN = 5;  // another margin that cannot be traced
static const int LINENUMBER_SPACING = 2;  // sets the margin for the line numbers

const QString KTURTLE_MAGIC_1_0 = QStringLiteral("kturtle-script-v1.0");


//BEGIN LineNumbers class

class LineNumbers : public QWidget
{
	Q_OBJECT

	public:
		LineNumbers(QWidget *parent, QTextEdit *te) : QWidget(parent), editor(te), maxWidth(0) {}
        ~LineNumbers() Q_DECL_OVERRIDE {}
	
		void setFont(const QFont& f) { QWidget::setFont(f); }

		void setWidth(int w) {
			if (w == maxWidth) return;  // save some cpu cycles
			maxWidth = w;
			QString s;
			for (; w > 0; w--) s += QLatin1Char('0');
			setFixedWidth(fontMetrics().boundingRect(s).width() + 2*LINENUMBER_SPACING);
		}

		void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE {
			QAbstractTextDocumentLayout* layout = editor->document()->documentLayout();
			int contentsY = editor->verticalScrollBar()->value();
			qreal pageBottom = contentsY + editor->viewport()->height();
			const QFontMetrics fm = fontMetrics();
			const int ascent = fontMetrics().ascent() + 1;  // height = ascent + descent + 1
			int lineCount = 1;
			QPainter painter(this);
			for (QTextBlock block = editor->document()->begin(); block.isValid(); block = block.next(), ++lineCount) {
				const QRectF boundingRect = layout->blockBoundingRect(block);
				QPointF position = boundingRect.topLeft();
				if (position.y() + boundingRect.height() < contentsY) continue;
				if (position.y() > pageBottom) break;
				const QString txt = QString::number(lineCount);
				painter.drawText(width() - fm.boundingRect(txt).width() - LINENUMBER_SPACING, qRound(position.y()) - contentsY + ascent, txt);
			}
			painter.end();
		}

	private:
		QTextEdit *editor;
		int        maxWidth;
};

//END class LineNumbers



//BEGIN QTextEdit sub-class

class TextEdit : public QTextEdit
{
	Q_OBJECT

	public:
		explicit TextEdit(QWidget* parent = nullptr)
			: QTextEdit(parent) {}

		void markCurrentWord(int startRow, int startCol, int endRow, int endCol) {
			currentWord.setCoords(startRow, startCol, endRow, endCol);
			viewport()->update();
		}

		void removeCurrentWordMark() {
			currentWord = QRect();
			viewport()->update();
		}

		void markCurrentError(int startRow, int startCol, int endRow, int endCol) {
			currentError.setCoords(startRow, startCol, endRow, endCol);
			viewport()->update();
		}

		void removeCurrentErrorMark() {
			currentError = QRect();
			viewport()->update();
		}

		void highlightCurrentLine() { viewport()->update(); }

		QRect currentLineRect() {
			// this method is also used for highlighting the background of the numbers
			QTextCursor cursor = textCursor();
			cursor.movePosition(QTextCursor::StartOfBlock);
			QRect rect = cursorRect(cursor);
			cursor.movePosition(QTextCursor::EndOfBlock);
			rect |= cursorRect(cursor);  // get the bounding rectangle of both rects
			rect.setX(0);
			rect.setWidth(viewport()->width());
			return rect;
		}

	protected:
		void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
			QPainter painter(viewport());
			painter.fillRect(currentLineRect(), QBrush(LINE_HIGHLIGHT_COLOR));
			if (!currentWord.isNull())
				foreach (const QRect &rect, coordsToRects(currentWord))
					painter.fillRect(rect, QBrush(WORD_HIGHLIGHT_COLOR));
			if (!currentError.isNull())
				foreach (const QRect &rect, coordsToRects(currentError))
					painter.fillRect(rect, QBrush(ERROR_HIGHLIGHT_COLOR));
			painter.end();
			QTextEdit::paintEvent(event);
		}

	private:
		QVector<QRect> coordsToRects(QRect coords) {
			// this methods calculate the viewport rectangles that cover a (multi-line) word or error
            // after switching the tokenizer to use the QTextDocument we might optimize this methods
			int startRow, startCol, endRow, endCol;
			coords.getCoords(&startRow, &startCol, &endRow, &endCol);

			QTextCursor cursor(document());
			cursor.movePosition(QTextCursor::Start);
			QTextCursor endCursor(cursor);
			cursor.movePosition(QTextCursor::NextBlock,        QTextCursor::MoveAnchor, startRow - 1);
			cursor.movePosition(QTextCursor::NextCharacter,    QTextCursor::MoveAnchor, startCol - 1);
			endCursor.movePosition(QTextCursor::NextBlock,     QTextCursor::MoveAnchor, endRow - 1);
			endCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, endCol - 1);

			QRect rect = cursorRect(cursor).adjusted(CURSOR_RECT_MARGIN, 0, 0, 0);
			cursor.movePosition(QTextCursor::EndOfLine);
			QVector<QRect> rects;
			while (cursor < endCursor) {
				cursor.movePosition(QTextCursor::PreviousCharacter);
				rects << (rect | cursorRect(cursor).adjusted(0, 0, fontMetrics().boundingRect(QStringLiteral("0")).width() - CURSOR_RECT_MARGIN, 0));
				cursor.movePosition(QTextCursor::Down);
				cursor.movePosition(QTextCursor::StartOfLine);
				rect = cursorRect(cursor).adjusted(CURSOR_RECT_MARGIN, 0, 0, 0);
				cursor.movePosition(QTextCursor::EndOfLine);
			}
			rects << (rect | cursorRect(endCursor).adjusted(0, 0, -CURSOR_RECT_MARGIN, 0));
			return rects;
		}

		// stores the start/end row/col of currentWord and currentError in the coods of 2 rectangles
		QRect currentWord, currentError;
};

//END QTextEdit sub-class




class Editor : public QFrame
{
	Q_OBJECT

	public:
		explicit Editor(QWidget *parent = nullptr);
		~Editor();

		QTextEdit* view() const { return editor; }
		QTextDocument* document() const { return editor->document(); }

		void enable();
		void disable();

		const QUrl &currentUrl() { return m_currentUrl; }
		void setCurrentUrl(const QUrl &url = QUrl());

		bool maybeSave();

		bool isModified() { return editor->document()->isModified(); }
		QString content() { return editor->document()->toPlainText(); }
		QString toHtml(const QString& title, const QString& lang);

		int row() { return currentRow; }
		int col() { return currentCol; }
		Token* currentToken();

		void removeMarkings() {
			editor->removeCurrentWordMark();
			editor->removeCurrentErrorMark();
		}


	public slots:
		bool newFile();
		bool openFile(const QUrl &url = QUrl());
		void openExample(const QString& example, const QString& exampleName);
		bool saveFile(const QUrl &url = QUrl());
		bool saveFileAs();
		void toggleLineNumbers(bool b) { numbers->setVisible(b); }
		void setModified(bool);
		void setOverwriteMode(bool b);

		void markCurrentWord(TreeNode* node) {
			editor->markCurrentWord(
				node->token()->startRow(),
				node->token()->startCol(),
				node->token()->endRow(),
				node->token()->endCol());
		}
		void markCurrentError(int startRow, int startCol, int endRow, int endCol) {
			editor->markCurrentError(startRow, startCol, endRow, endCol);
		}
		void find();
		void findNext();
		void findPrev();
		void insertPlainText(const QString& txt);
		void rehighlight() { highlighter->rehighlight(); }


	signals:
		void contentNameChanged(const QString&);
		void fileOpened(const QUrl&);
		void fileSaved(const QUrl&);
		void modificationChanged();
		void contentChanged();
		void cursorPositionChanged();


	protected slots:
		void textChanged(int pos, int added, int removed);
// 		void cursorPositionChanged();

	protected:
		void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;


	private slots:
		void updateOnCursorPositionChange();
		void highlightCurrentLine() { this->update(); }

	private:
		void setContent(const QString&);

		TextEdit    *editor;  // TODO why pointers?
		Highlighter *highlighter;  // TODO could this class become a singleton? (shared with the inspector, errdlg)
		Tokenizer   *tokenizer;  // TODO could this class become a singleton? (shared with the highlighter, interpreter)
		LineNumbers *numbers;
		QHBoxLayout *box;  // TODO is this relly needed?
		KFindDialog *fdialog;
		QUrl         m_currentUrl;  // contains url to the currently load file or the exampleName
		QColor       highlightedLineBackgroundColor;  // the bg color of the current line's line number space
		QString      currentLine;
		int          currentRow;
		int          currentCol;
};


#endif  // _EDITOR_H_
