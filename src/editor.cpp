/*
    SPDX-FileCopyrightText: 2003-2008 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "editor.h"


#include <QBoxLayout>
#include <QFileDialog>
#include <QFontDatabase>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QTextStream>
#include <QBuffer>

#include <KFind>
#include <KLocalizedString>
#include <KMessageBox>
#include <KIO/StatJob>
#include <KIO/StoredTransferJob>

static const int CURSOR_WIDTH = 2;  // in pixels
static const int TAB_WIDTH    = 2;  // in character widths


Editor::Editor(QWidget *parent)
	: QFrame(parent)
{
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	setLineWidth(CURSOR_WIDTH);
	setCurrentUrl();
	currentRow = 1;
	currentCol = 1;

	// setup the main view
	editor = new TextEdit(this);
	editor->document()->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	editor->setFrameStyle(QFrame::NoFrame);
	editor->installEventFilter(this);
	editor->setLineWrapMode(QTextEdit::WidgetWidth);
	editor->setTabStopDistance(editor->fontMetrics().boundingRect(QStringLiteral("0")).width() * TAB_WIDTH);
	editor->setAcceptRichText(false);
	setFocusProxy(editor);
	connect(editor->document(), &QTextDocument::contentsChange, this, &Editor::textChanged);
	connect(editor->document(), &QTextDocument::modificationChanged, this, &Editor::setModified);
	connect(editor, &TextEdit::cursorPositionChanged, this, &Editor::updateOnCursorPositionChange);

	// setup the line number pane
	numbers = new LineNumbers(this, editor);
	numbers->setFont(editor->document()->defaultFont());
	numbers->setWidth(1);
	connect(editor->document()->documentLayout(), SIGNAL(update(QRectF)), numbers, SLOT(update()));
	connect(editor->verticalScrollBar(), SIGNAL(valueChanged(int)), numbers, SLOT(update()));

	// let the line numbers and the editor coexist
	box = new QHBoxLayout(this);
	box->setSpacing(0);
	box->setContentsMargins(0, 0, 0, 0);
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
	tokenizer = new Tokenizer();
}

Editor::~Editor()
{
	delete highlighter;
	delete tokenizer;
}

void Editor::enable() {
	editor->viewport()->setEnabled(true);
	editor->setReadOnly(false);
}

void Editor::disable() {
	editor->viewport()->setEnabled(false);
	editor->setReadOnly(true);
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
		editor->document()->setModified(false);
		setCurrentUrl(QUrl::fromLocalFile(exampleName));
	}
}

void Editor::textChanged(int pos, int removed, int added)
{
	Q_UNUSED(pos);
	if (removed == 0 && added == 0) return;  // save some cpu cycles
	removeMarkings();  // removes the character markings if there are any
	int lineCount = 1;
	for (QTextBlock block = editor->document()->begin(); block.isValid(); block = block.next()) lineCount++;
	numbers->setWidth(qMax(1, 1 + static_cast<int>(std::floor(std::log10(static_cast<double>(lineCount) - 1)))));

	emit contentChanged();
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

bool Editor::openFile(const QUrl &_url)
{
	QUrl url = _url;
	if (maybeSave()) {
		if (url.isEmpty()) {
            url = QFileDialog::getOpenFileUrl(this, 
                                              i18n("Open"), 
                                              QUrl(), 
                                              QStringLiteral("%1 (*.turtle);;%2 (*)").arg(i18n("Turtle code files")).arg(i18n("All files"))
                    );
		}
		if (!url.isEmpty()) {
			KIO::StoredTransferJob *job = KIO::storedGet(url);
			if (job->exec()) {
				QByteArray data = job->data();
				QBuffer buffer(&data);
				if (!buffer.open(QIODevice::ReadOnly | QIODevice::Text)) {
					return false; // can't happen
				}
				QTextStream in(&buffer);
				// check for our magic identifier
				QString s;
				s = in.readLine();
				if (s != KTURTLE_MAGIC_1_0) {
					KMessageBox::error(this, i18n("The file you try to open is not a valid KTurtle script, or is incompatible with this version of KTurtle.\nCannot open %1", url.toDisplayString(QUrl::PreferLocalFile)));
					return false;
				}
				QString localizedScript = Translator::instance()->localizeScript(in.readAll());
				setContent(localizedScript);
				setCurrentUrl(url);
				editor->document()->setModified(false);
				emit fileOpened(url);
				return true;
			} else {
				KMessageBox::error(this, job->errorString());
				return false;
			}
		}
	}
//	statusbar "Nothing opened"
	return false;
}

bool Editor::saveFile(const QUrl &targetUrl)
{
	QUrl url(targetUrl);
	bool result = false;
	if (url.isEmpty() && currentUrl().isEmpty()) {
		result = saveFileAs();
	} else {
		if (url.isEmpty()) url = currentUrl();
		QTemporaryFile tmp;  // only used for network export
		tmp.setAutoRemove(false);
		tmp.open();
		QString filename = url.isLocalFile() ? url.toLocalFile() : tmp.fileName();
	
		QSaveFile *savefile = new QSaveFile(filename);
		if (savefile->open(QIODevice::WriteOnly)) {
			QTextStream outputStream(savefile);
			// store commands in their generic @(...) notation format, to be translatable when reopened
			// this allows sharing of scripts written in different languages
			Tokenizer tokenizer;
			tokenizer.initialize(editor->document()->toPlainText());
			const QStringList localizedLooks(Translator::instance()->allLocalizedLooks());
			QString untranslated;
			Token* t;
			bool pendingEOL = false;  // to avoid writing a final EOL token
			while ((t = tokenizer.getToken())->type() != Token::EndOfInput) {
				if (pendingEOL) {
					untranslated.append('\n');
					pendingEOL = false;
				}
				if (localizedLooks.contains(t->look())) {
					QString defaultLook(Translator::instance()->defaultLook(t->look()));
					untranslated.append(QStringLiteral("@(%1)").arg(defaultLook));
				} else {
					if (t->type() == Token::EndOfLine) 
						pendingEOL = true;
					else
						untranslated.append(t->look());
				}
			}
			outputStream << KTURTLE_MAGIC_1_0 << '\n';
			outputStream << untranslated;
			outputStream.flush();
			if (savefile->commit()) {
				result = true;
				if (!url.isLocalFile()) {
					KIO::StoredTransferJob *job = KIO::storedPut(savefile, url, -1, KIO::DefaultFlags);
					if (!job->exec()) {
						result = false;
						KMessageBox::error(this, i18n("Could not save file."));
					}
				}
			} else {
				// Error.
				result = false;
				KMessageBox::error(this, i18n("Could not save file."));
			}
			if (result) {
				setCurrentUrl(url);
				editor->document()->setModified(false);
				// MainWindow will add us to the recent file list
				emit fileSaved(url);
			}
		}
		delete savefile;
	}
	return result;
}

bool Editor::saveFileAs()
{
    QUrl url = QFileDialog::getSaveFileUrl(this,
                                           i18n("Save As"),
                                           QUrl(),
                                           QStringLiteral("%1 (*.turtle);;%2 (*)").arg(i18n("Turtle code files")).arg(i18n("All files"))
                                           );
    if (url.isEmpty()) return false;
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
	emit modificationChanged();
}

// TODO: improve find to be able to search within a selection
void Editor::find()
{
	// find selection, etc
	if (editor->textCursor().hasSelection()) {
		QString selectedText = editor->textCursor().selectedText();
		// If the selection is too big, then we don't want to automatically
		// populate the search text box with the selection text
		if (selectedText.length() < 30) {
			fdialog->setPattern(selectedText);
		}
	}
	if (fdialog->exec() == QDialog::Accepted && !fdialog->pattern().isEmpty()) {
		long kOpts = fdialog->options();
		QTextDocument::FindFlags qOpts = {};
		if (kOpts & KFind::CaseSensitive)  { qOpts |= QTextDocument::FindCaseSensitively; }
		if (kOpts & KFind::FindBackwards)  { qOpts |= QTextDocument::FindBackward; }
		if (kOpts & KFind::WholeWordsOnly) { qOpts |= QTextDocument::FindWholeWords; }
		editor->find(fdialog->pattern(), qOpts);
	}
}

void Editor::findNext()
{
	if (!fdialog->pattern().isEmpty()) {
		long kOpts = fdialog->options();
		QTextDocument::FindFlags qOpts = {};
		if (kOpts & KFind::CaseSensitive)  { qOpts |= QTextDocument::FindCaseSensitively; }
		if (kOpts & KFind::FindBackwards)  { qOpts |= QTextDocument::FindBackward; }
		if (kOpts & KFind::WholeWordsOnly) { qOpts |= QTextDocument::FindWholeWords; }
		editor->find(fdialog->pattern(), qOpts);
	}
}

void Editor::findPrev()
{
	if(!fdialog->pattern().isEmpty()) {
		long kOpts = fdialog->options();
		QTextDocument::FindFlags qOpts = {};
		if (kOpts & KFind::CaseSensitive)    { qOpts |= QTextDocument::FindCaseSensitively; }
		// search in the opposite direction as findNext()
		if (!(kOpts & KFind::FindBackwards)) { qOpts |= QTextDocument::FindBackward; }
		if (kOpts & KFind::WholeWordsOnly)   { qOpts |= QTextDocument::FindWholeWords; }
		editor->find(fdialog->pattern(), qOpts);
	}
}

void Editor::setCurrentUrl(const QUrl &url)
{
	m_currentUrl = url;
	emit contentNameChanged(m_currentUrl.fileName());
}

void Editor::setOverwriteMode(bool b)
{
	editor->setOverwriteMode(b);
	editor->setCursorWidth(b ? editor->fontMetrics().boundingRect(QStringLiteral("0")).width() : 2);
}


void Editor::updateOnCursorPositionChange()
{
	// convert the absolute pos into a row/col pair, and set current line aswell
	QString s = editor->toPlainText();
	int pos = editor->textCursor().position();
	int row = 1;
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
	if (next_break == 0)
		next_break = s.length();
	if (currentRow != row) {
		currentRow = row;
		highlightCurrentLine();
		editor->highlightCurrentLine();
	}
	currentCol = pos - last_break;
	currentLine = s.mid(last_break+1, next_break-last_break-1);
	emit cursorPositionChanged();
}

Token* Editor::currentToken()
{
	tokenizer->initialize(currentLine);
	Token* token = tokenizer->getToken();
	while (token->type() != Token::EndOfInput) {
		if (currentCol >= token->startCol() && currentCol <= token->endCol())
			return token;
		delete token;
		token = tokenizer->getToken();
	}
	delete token;
	return nullptr;
}


void Editor::insertPlainText(const QString& txt)
{
	editor->textCursor().insertText(txt);
}


void Editor::paintEvent(QPaintEvent *event)
{
	QRect rect = editor->currentLineRect();
	rect.setWidth(this->width() - EDITOR_MARGIN);  // don't draw too much
	rect.translate(0, EDITOR_MARGIN);  // small hack to nicely align the line highlighting
	//QColor bgColor = this->palette().brush(this->backgroundRole()).color();
	QPainter painter(this);
	const QBrush brush(highlightedLineBackgroundColor);
	painter.fillRect(rect, brush);
	painter.end();
	QFrame::paintEvent(event);
}

QString Editor::toHtml(const QString& title, const QString& lang)
{
	Tokenizer* tokenizer = new Tokenizer();
	tokenizer->initialize(editor->document()->toPlainText());
	QString html = QString();
	QTextCharFormat* format;
	Token* token = tokenizer->getToken();
	while (token->type() != Token::EndOfInput) {
		QString escaped;
		switch (token->type()) {
			case Token::EndOfLine:  escaped = QStringLiteral("<br />"); break;
			case Token::WhiteSpace: escaped = QLatin1String(""); for (int n = 0; n < token->look().length(); n++) { escaped += QLatin1String("&nbsp;"); } break;
			default:                escaped = token->look().toHtmlEscaped(); break;
		}
		format = highlighter->tokenToFormat(token);
		if (format) {
			bool bold = format->fontWeight() > 50;
			html += QStringLiteral("<span style=\"color: %1;%2\">%3</span>")
				.arg(format->foreground().color().name())
				.arg(bold ? " font-weight: bold;" : "")
				.arg(escaped);
		} else {
			html += escaped;
		}
		token = tokenizer->getToken();
	}
	delete tokenizer;
	return QString("<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"%1\" lang=\"%1\">"
	               "<head><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />"
	               "<title>%2</title></head>"
	               "<body style=\"font-family: monospace;\">%3</body></html>").arg(lang).arg(title).arg(html);
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
