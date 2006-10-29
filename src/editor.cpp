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


#include <QTextDocument>
#include <QTextBlock>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QToolTip>
#include <QTextStream>

#include <kdebug.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksavefile.h>
#include <ktemporaryfile.h>

#include <kio/netaccess.h>


#include "editor.h"


static const int CURSOR_WIDTH = 2;  // in pixels
static const int TAB_WIDTH    = 2;  // in character widths


Editor::Editor(QWidget *parent)
	: QFrame(parent)
{
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	setLineWidth(CURSOR_WIDTH);
	setCurrentUrl();

	// setup the main view
	editor = new TextEdit(this);
	editor->document()->setDefaultFont(QFont("Courier", 12));
	editor->setFrameStyle(QFrame::NoFrame);
	editor->installEventFilter(this);
	editor->setLineWrapMode(QTextEdit::WidgetWidth);
	editor->setTabStopWidth(editor->fontMetrics().width("0") * TAB_WIDTH);
	editor->setAcceptRichText(false);
	setFocusProxy(editor);
	connect(editor->document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(textChanged(int,int,int)));
	connect(editor->document(), SIGNAL(modificationChanged(bool)), this, SLOT(setModified(bool)));
	connect(editor, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));

	// setup the line number pane
	numbers = new LineNumbers(this, editor);
	numbers->setFont(editor->document()->defaultFont());
	numbers->setWidth(1);
	connect(editor->document()->documentLayout(), SIGNAL(update(const QRectF &)), numbers, SLOT(update()));
	connect(editor->verticalScrollBar(), SIGNAL(valueChanged(int)), numbers, SLOT(update()));

	// let the line numbers and the editor coexist
	box = new QHBoxLayout(this);
	box->setSpacing(0);
	box->setMargin(0);
	box->addWidget(numbers);
	box->addWidget(editor);

	// calculate the bg color for the highlighted line
	QColor bgColor = this->palette().brush(this->backgroundRole()).color();
	highlightedLineBackgroundColor.setHsv(
		LINE_HIGHLIGHT_COLOR.hue(),
		bgColor.saturation() + EXTRA_SATURATION,
		bgColor.value());

	// our syntax highlighter (this does not do any markings)
	highlighter = new Highlighter(editor->document());

	// set up the markers
	currentLineFormat.setBackground(QBrush(QColor(239,247,255)));  // light blue
	currentWordFormat.setBackground(QBrush(QColor(255,255,156)));  // light yellow
	currentErrorFormat.setBackground(QBrush(QColor(255,200,200)));  // light red

	newFile();  // sets some more default values
}

Editor::~Editor()
{
	delete highlighter;
// 	delete editor;  // shoudl go automagically through QObject ownerships
// 	delete numbers;
}

void Editor::setContent(const QString& s)
{
	editor->document()->setPlainText(s);
	editor->document()->setModified(false);
}

void Editor::openExample(const QString& example, const QString& exampleName)
{
	if (newFile()) {
		setContent(example);
		setCurrentUrl(KUrl(exampleName));
		editor->document()->setModified(false);
	}
}

void Editor::textChanged(int pos, int removed, int added)
{
	Q_UNUSED(pos);
// 	if ((removed == 0 && added == 0) || changingMarkings) return;  // save some cpu cycles
	if (removed == 0 && added == 0) return;  // save some cpu cycles

	removeMarkings();  // removes the character markings if there are any

	int lineCount = 1;
	for (QTextBlock block = editor->document()->begin(); block.isValid(); block = block.next()) lineCount++;
	numbers->setWidth(qMax(1, 1 + (int)floor(log10(lineCount - 1))));
}


bool Editor::newFile()
{
	if (maybeSave()) {
		changingMarkings = false;
		isMarked = false;  // there are no char markers in a new file
		editor->document()->clear();
		setCurrentUrl();
// 		markCurrentLine();  // current line marker
		return true;
	}
	return false;
}

bool Editor::openFile(const KUrl &_url)
{
	KUrl url = _url;
	if (maybeSave()) {
		if (url.isEmpty()) {
			url = KFileDialog::getOpenUrl(KUrl(), QString("*.turtle|%1\n*|%2").arg(i18n("Turtle code files")).arg(i18n("All files")), this, i18n("Open"));
		}
		if (!url.isEmpty()) {
// 			if (!KIO::NetAccess::exists(url, true, this)) {
// 				KMessageBox::error(this, i18n("The given file could not be read, check if it exists and if it is readable for the current user."));
// 				return false;
// 			}
			QString fileString;  // tmp file or local file
			if (KIO::NetAccess::download(url, fileString, this)) {
				QFile file(fileString);
				if (!file.open(QFile::ReadOnly | QFile::Text)) {
					KMessageBox::error(this, i18n("Cannot read %1", fileString));
					return false;
				}
				QTextStream in(&file);
				setContent(in.readAll());
				KIO::NetAccess::removeTempFile(fileString);
				setCurrentUrl(url);
				editor->document()->setModified(false);
// 				statusbar
				return true;
			} else {
				KMessageBox::error(this, KIO::NetAccess::lastErrorString());
				return false;
			}
		}
	}
//	statusbar "Nothing opened"
	return false;
}

bool Editor::saveFile(const KUrl &targetUrl)
{
	KUrl url(targetUrl);
	bool result = false;
	if (url.isEmpty() && currentUrl().isEmpty()) {
		result = saveFileAs();
	} else {
		if (url.isEmpty()) url = currentUrl();
		KTemporaryFile tmp;  // only used for network export
		tmp.setAutoRemove(false);
		tmp.open();
		QString filename = url.isLocalFile() ? url.path() : tmp.fileName();
	
		KSaveFile *savefile = new KSaveFile(filename);
		if (savefile->open()) {
			QTextStream outputStream(savefile);
// 			outputStream.setEncoding(QTextStream::UnicodeUTF8);
			outputStream << editor->document()->toPlainText();
			outputStream.flush();
			savefile->finalize();  // check for error here?
		}
		delete savefile;
		if (!url.isLocalFile()) KIO::NetAccess::upload(filename, url, this);
		result = true;
	}
	if (result) {
		setCurrentUrl(url);
		editor->document()->setModified(false);
	}
	return result;
}

bool Editor::saveFileAs()
{
	KUrl url = KFileDialog::getSaveUrl(QString(), QString("*.turtle|%1\n*|%2").arg(i18n("Turtle code files")).arg(i18n("All files")), this, i18n("Save As"));
	if (url.isEmpty()) return false;
	if (KIO::NetAccess::exists(url, true, this) &&
		KMessageBox::warningContinueCancel(this,
			i18n("Are you sure you want to overwrite %1", url.fileName()),
			i18n("Overwrite Existing File"),KGuiItem(i18n("&Overwrite")),
			i18n("&Overwrite")
			) != KMessageBox::Continue
		) return false;
	bool result = saveFile(url);
	return result;
}

bool Editor::maybeSave()
{
	if (!editor->document()->isModified()) return true;
	int result = KMessageBox::warningContinueCancel(this,
		i18n("The program you are currently working on is not saved. "
		     "By continuing you may lose the changes you have made."),
		i18n("Unsaved File"), KGuiItem(i18n("&Discard Changes")), i18n("&Discard Changes"));
	if (result == KMessageBox::Continue) return true;
	return false;
}


void Editor::setModified(bool b)
{
	editor->document()->setModified(b);
	emit modificationChanged(b);
}

void Editor::setCurrentUrl(const KUrl& url)
{
	if (url == m_currentUrl) return;
	m_currentUrl = KUrl(url);
	emit currentUrlChanged(m_currentUrl);
}

void Editor::setInsertMode(bool b)
{
	editor->setOverwriteMode(!b);
	editor->setCursorWidth(b ? 2 : editor->fontMetrics().width("0"));
}

void Editor::markCurrentWord(int startRow, int startCol, int endRow, int endCol)
{
	changingMarkings = true;
	removeMarkings();
	markChars(currentWordFormat, startRow, startCol, endRow, endCol);
	changingMarkings = false;
}

void Editor::markCurrentError(int startRow, int startCol, int endRow, int endCol)
{
	changingMarkings = true;
	removeMarkings();
	markChars(currentErrorFormat, startRow, startCol, endRow, endCol);
	changingMarkings = false;
}

void Editor::markChars(const QTextCharFormat& charFormat, int startRow, int startCol, int endRow, int endCol)
{
	changingMarkings = true;
	bool modified = editor->document()->isModified();  // save modification state

	removeMarkings();  // remove earlier markings

	// mark the selection
	QTextCursor cursor(editor->document());
	cursor.movePosition(QTextCursor::Start,         QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::NextBlock,     QTextCursor::MoveAnchor, startRow - 1);
	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, startCol - 1);
	cursor.movePosition(QTextCursor::NextBlock,     QTextCursor::KeepAnchor, endRow - startRow);
	cursor.movePosition(QTextCursor::StartOfBlock,  QTextCursor::KeepAnchor);
	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, endCol - 1);
	cursor.setCharFormat(charFormat);
	isMarked = true;

	editor->document()->setModified(modified);  // restore modification state
	changingMarkings = false;
}

void Editor::removeMarkings()
{
	changingMarkings = true;
	if (!isMarked) return;  // don't recurse on this method
	isMarked = false;

	// remove all char formatting (only the char formatting, so not the current line marking (a block))
	QTextCursor cursor(editor->document());
	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End,   QTextCursor::KeepAnchor);
	cursor.setCharFormat(defaultCharFormat);
	changingMarkings = false;
}


void Editor::cursorPositionChanged()
{
	// convert the absolute pos into a row/col pair, and return the current line aswell
	QString s = editor->toPlainText();
	int pos = editor->textCursor().position();
	int row = 0;
	int last_break = -1;
	int next_break = 0;
	for (int i = 0; i < s.length(); i++) {
		if (s.at(i) == '\n' && i < pos) {
			last_break = i;
			row++;
		} else if (s.at(i) == '\n' && i >= pos) {
			next_break = i;
			break;
		}
	}
	if (next_break == 0) next_break = s.length();
	if (currentLine != row) {
		currentLine = row;
		highlightCurrentLine();
		editor->highlightCurrentLine();
	}
	emit cursorPositionChanged(row+1, pos-last_break, s.mid(last_break+1, next_break-last_break-1));
}


void Editor::paintEvent(QPaintEvent *event)
{
	QRect rect = editor->currentLineRect();
// 	rect.setX(0);
	rect.setWidth(this->width() - EDITOR_MARGIN);  // don't draw too much
	rect.translate(0, EDITOR_MARGIN);  // small hack to nicely align the the line highlighting
	QColor bgColor = this->palette().brush(this->backgroundRole()).color();
	QPainter painter(this);
	const QBrush brush(highlightedLineBackgroundColor);
	painter.fillRect(rect, brush);
	painter.end();
	QFrame::paintEvent(event);
}



// bool Editor::eventFilter(QObject *obj, QEvent *event)
// {
// 	if (obj != editor) return QFrame::eventFilter(obj, event);
// 
// 	if (event->type() == QEvent::ToolTip) {
// 		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
// 
// 		QTextCursor cursor = editor->cursorForPosition(helpEvent->pos());
// 		cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
// 		cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
// 
// 		QString word = cursor.selectedText();
// 		emit mouseHover(word);
// 		emit mouseHover(helpEvent->pos(), word);
// 
// 		// QToolTip::showText(helpEvent->globalPos(), word); // For testing
// 	}
// 
// 	return false;
// }


#include "editor.moc"
