/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

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

#include "editor.h"

#include <QTextDocument>
#include <QTextBlock>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QPainter>
#include <QAbstractTextDocumentLayout>

#include <QTextStream>

#include <kdebug.h>

#include <kfiledialog.h>
#include <kfind.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksavefile.h>
#include <ktemporaryfile.h>

#include <kio/netaccess.h>


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

	// create a find dialog
	fdialog = new KFindDialog();
	fdialog->setSupportsRegularExpressionFind(false);
	fdialog->setHasSelection(false);
	fdialog->setHasCursor(false);

	// sets some more default values
	newFile();
}

Editor::~Editor()
{
	delete highlighter;
// 	delete editor;  // should go automagically through QObject ownerships
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
		setCurrentUrl();
		editor->document()->setModified(false);
	}
}

void Editor::textChanged(int pos, int removed, int added)
{
	Q_UNUSED(pos);
	if (removed == 0 && added == 0) return;  // save some cpu cycles
	removeMarkings();  // removes the character markings if there are any
	int lineCount = 1;
	for (QTextBlock block = editor->document()->begin(); block.isValid(); block = block.next()) lineCount++;
	numbers->setWidth(qMax(1, 1 + (int)floor(log10(lineCount - 1))));
}


bool Editor::newFile()
{
	if (maybeSave()) {
		editor->document()->clear();
		setCurrentUrl();
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
// 			if (!KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, this)) {
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
				QString localizedScript;
				localizedScript = Translator::instance()->localizeScript(in.readAll());
				setContent(localizedScript);
				KIO::NetAccess::removeTempFile(fileString);
				setCurrentUrl(url);
				editor->document()->setModified(false);
// 				statusbar
				emit fileOpened(url);
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
			//Store commands in their generic format, to be translatable when reopened
			//This allows sharing of scripts written in different languages
			Tokenizer tokenizer;
			tokenizer.initialize(editor->document()->toPlainText());
			QStringList defaultLooks(Translator::instance()->allDefaultLooks());
			QString unstranslated;
			Token* t;
			bool pendingEOL = false; //to avoid writing a final EOL token
			while ((t = tokenizer.getToken())->type() != Token::EndOfInput) {
				if (pendingEOL) {
					unstranslated.append('\n');
					pendingEOL=false;
				}
				if (defaultLooks.contains(t->look())) {
					unstranslated.append(QString("@(%1)").arg(t->look()));
				} else {
					if (t->type() == Token::EndOfLine) 		pendingEOL=true;
					else
						unstranslated.append(t->look());
				}
			}
			outputStream << unstranslated;
			outputStream.flush();
			savefile->finalize();  // check for error here?
		}
		delete savefile;
		if (!url.isLocalFile()) KIO::NetAccess::upload(filename, url, this);
		setCurrentUrl(url);
		editor->document()->setModified(false);
		result = true; // fix GUI for saveAs and saveExamples. TODO: check 5 lines above
	}
	return result;
}

bool Editor::saveFileAs()
{
	KUrl url = KFileDialog::getSaveUrl(QString(), QString("*.turtle|%1\n*|%2").arg(i18n("Turtle code files")).arg(i18n("All files")), this, i18n("Save As"));
	if (url.isEmpty()) return false;
	if (KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, this) &&
		KMessageBox::warningContinueCancel(this,
			i18n("Are you sure you want to overwrite %1?", url.fileName()),
			i18n("Overwrite Existing File"),KGuiItem(i18n("&Overwrite")),
			KStandardGuiItem::cancel(),
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
		i18n("Unsaved File"), KGuiItem(i18n("&Discard Changes")), KStandardGuiItem::cancel(), i18n("&Discard Changes"));
	if (result == KMessageBox::Continue) return true;
	return false;
}


void Editor::setModified(bool b)
{
	editor->document()->setModified(b);
	emit modificationChanged(b);
}

// TODO: improve find to be able to search within a selection
void Editor::find()
{
	// find selection, etc
	if(editor->textCursor().hasSelection())
	{
		QString selectedText = editor->textCursor().selectedText();
		// If the selection is too big, then we don't want to automatically
		// populate the search text box with the selection text
		if(selectedText.length() < 30)
		{
			fdialog->setPattern(selectedText);
		}
	}
	if(fdialog->exec()==QDialog::Accepted && !fdialog->pattern().isEmpty())
	{
		long kOpts = fdialog->options();
		QTextDocument::FindFlags qOpts = 0;
		if(kOpts & KFind::CaseSensitive)
		{ qOpts |= QTextDocument::FindCaseSensitively; }
		if(kOpts & KFind::FindBackwards)
		{ qOpts |= QTextDocument::FindBackward; }
		if(kOpts & KFind::WholeWordsOnly)
		{ qOpts |= QTextDocument::FindWholeWords; }
		editor->find(fdialog->pattern(), qOpts);
	}
}

void Editor::findNext()
{
	if(!fdialog->pattern().isEmpty())
	{
		long kOpts = fdialog->options();
		QTextDocument::FindFlags qOpts = 0;
		if(kOpts & KFind::CaseSensitive)
		{ qOpts |= QTextDocument::FindCaseSensitively; }
		if(kOpts & KFind::FindBackwards)
		{ qOpts |= QTextDocument::FindBackward; }
		if(kOpts & KFind::WholeWordsOnly)
		{ qOpts |= QTextDocument::FindWholeWords; }
		editor->find(fdialog->pattern(), qOpts);
	}
}

void Editor::findPrev()
{
	if(!fdialog->pattern().isEmpty())
	{
		long kOpts = fdialog->options();
		QTextDocument::FindFlags qOpts = 0;
		if(kOpts & KFind::CaseSensitive)
		{ qOpts |= QTextDocument::FindCaseSensitively; }
		// search in the opposite direction as findNext()
		if(!(kOpts & KFind::FindBackwards))
		{ qOpts |= QTextDocument::FindBackward; }
		if(kOpts & KFind::WholeWordsOnly)
		{ qOpts |= QTextDocument::FindWholeWords; }
		editor->find(fdialog->pattern(), qOpts);
	}
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
