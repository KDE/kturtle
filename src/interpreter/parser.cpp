/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the Free
	Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.
*/

#include "parser.h"

#include <QDebug>

#include <KLocalizedString>


Parser::~Parser()
{
}

void Parser::initialize(Tokenizer* _tokenizer, ErrorList* _errorList)
{
	tokenizer    = _tokenizer;
	errorList    = _errorList;

	rootNode     = new TreeNode(new Token(Token::Root, QStringLiteral("root"), 0, 0, 0, 0));
	currentScope = rootNode;
    newScope     = nullptr;
	finished     = false;

	nextToken();
}

void Parser::parse()
{
	if (finished) return;

// 	//qDebug() << "Parser::parse() -- main parse loop called";
	TreeNode* resultNode = parseStatement();  // parse the next statement
	if (resultNode == nullptr) {  // no statement was found
		addError(i18n("Expected a command, instead got '%1'", currentToken->look()), *currentToken, 0);
		return;
	}

	if (resultNode->token()->type() == Token::ScopeClose ||
	    resultNode->token()->type() == Token::EndOfInput) {
		delete resultNode;
	} else {
		currentScope->appendChild(resultNode);
	}

	if (newScope != nullptr) {
		currentScope = newScope;
		newScope = nullptr;
	}
}

void Parser::nextToken()
{
	// get the next relevant token, and store it in currentToken

	// skip spaces and comments:
	currentToken = tokenizer->getToken();
// 	//qDebug() << "########### got token: " << currentToken->look();

	while (currentToken->type() == Token::WhiteSpace ||
	       currentToken->type() == Token::Comment) {
		delete currentToken;
		currentToken = tokenizer->getToken();
// 		//qDebug() << "########### got token: " << currentToken->look();
	}

	if (currentToken->type() == Token::Error)
		addError(i18n("Could not understand '%1'", currentToken->look()), *currentToken, 100);

// 	QString out = QString("Parser::nextToken(): \"%5\" [%6] @ (%1,%2)-(%3,%4)")
// 		.arg(currentToken->startRow())
// 		.arg(currentToken->startCol())
// 		.arg(currentToken->endRow())
// 		.arg(currentToken->endCol())
// 		.arg(currentToken->look())
// 		.arg(currentToken->type());
// 	//qDebug() << "Parser::nextToken():" << currentToken->look() << " [" << currentToken->type() << "]   on line" << currentToken->startRow();
	if (currentToken->type() == Token::EndOfInput)
		finished = true;
}


bool Parser::skipToken(int expectedTokenType)
{
	// if the expectedTokenType matches the currentToken's type, dispose the currentToken and get a new one
	if (currentToken->type() == expectedTokenType) {
		delete currentToken;
		nextToken();
		return true;
	}
	return false;
}

bool Parser::skipToken(int expectedTokenType, Token& byToken)
{
	// TODO have byToken, and "errorHappenedHereToken"
	//TODO: Translate the following QStrings?

	if (skipToken(expectedTokenType)) return true;

	switch (expectedTokenType) {
		case Token::ArgumentSeparator:
			addError(i18n("A comma was expected here..."), byToken, 0);
			break;
		case Token::EndOfInput:
			addError(i18n("Did not expect '%1', instead expected the line to end after %2",
                    currentToken->look(),
                    byToken.look()),
               byToken, 0);
			break;
		case Token::Assign:
			addError(i18n("Expected an assignment, '=', after the variable '%1'",
                    byToken.look()), byToken, 0);
			break;
		case Token::ParenthesisClose:
			addError(i18n("Did not expect '%1', instead expected a closing parenthesis, ')'",
                    currentToken->look()), byToken, 0);
			break;
		case Token::To:
			addError(i18n("Expected 'to' after 'for'"), byToken, 0);
			break;
		case Token::FunctionCall:
			addError(i18n("Expected a name for a command after 'learn' command"), byToken, 0);
			break;
	}

	return false;
}


void Parser::addError(const QString& s, const Token& t, int code)
{
//	if (m_testing)
// 		//qDebug() << "ERR> " << qPrintable(s) << " (parse error)";
	errorList->addError(s, t, code);
}

void Parser::printTree() const
{
	const char* prefix = m_testing ? "NTR> " : "";
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	foreach (const QString &line, rootNode->toString().split('\n', QString::SkipEmptyParts)) {
#else
	foreach (const QString &line, rootNode->toString().split('\n', Qt::SkipEmptyParts)) {
#endif
		qDebug() << prefix << qPrintable(line.trimmed());
	}
}



TreeNode* Parser::parseStatement()
{
	// in addition to whitespace and comments (skipped by nextToken()), also skip newlines before statements
	while (currentToken->type() == Token::EndOfLine) {
		delete currentToken;
		nextToken();
	}

	if (finished)
		return new TreeNode(currentToken);  // TODO check if all scopes are closed before returning + throw error

	switch (currentToken->type()) {

//BEGIN GENERATED parser_statements_cpp CODE

/* The code between the line that start with "//BEGIN GENERATED" and "//END GENERATED"
 * is generated by "generate.rb" according to the definitions specified in
 * "definitions.rb". Please make all changes in the "definitions.rb" file, since all
 * all change you make here will be overwritten the next time "generate.rb" is run.
 * Thanks for looking at the code!
 */

		case Token::Variable            : return parseVariable();
		case Token::FunctionCall        : return parseFunctionCall();
		case Token::ScopeOpen           : return parseScopeOpen();
		case Token::ScopeClose          : return parseScopeClose();
		case Token::Exit                : return parseExit();
		case Token::If                  : return parseIf();
		case Token::Repeat              : return parseRepeat();
		case Token::While               : return parseWhile();
		case Token::For                 : return parseFor();
		case Token::Break               : return parseBreak();
		case Token::Return              : return parseReturn();
		case Token::Wait                : return parseWait();
		case Token::Assert              : return parseAssert();
		case Token::Learn               : return parseLearn();
		case Token::Reset               : return parseReset();
		case Token::Clear               : return parseClear();
		case Token::Center              : return parseCenter();
		case Token::Go                  : return parseGo();
		case Token::GoX                 : return parseGoX();
		case Token::GoY                 : return parseGoY();
		case Token::Forward             : return parseForward();
		case Token::Backward            : return parseBackward();
		case Token::Direction           : return parseDirection();
		case Token::TurnLeft            : return parseTurnLeft();
		case Token::TurnRight           : return parseTurnRight();
		case Token::PenWidth            : return parsePenWidth();
		case Token::PenUp               : return parsePenUp();
		case Token::PenDown             : return parsePenDown();
		case Token::PenColor            : return parsePenColor();
		case Token::CanvasColor         : return parseCanvasColor();
		case Token::CanvasSize          : return parseCanvasSize();
		case Token::SpriteShow          : return parseSpriteShow();
		case Token::SpriteHide          : return parseSpriteHide();
		case Token::Print               : return parsePrint();
		case Token::FontSize            : return parseFontSize();
		case Token::Random              : return parseRandom();
		case Token::GetX                : return parseGetX();
		case Token::GetY                : return parseGetY();
		case Token::Message             : return parseMessage();
		case Token::Ask                 : return parseAsk();
		case Token::Pi                  : return parsePi();
		case Token::Tan                 : return parseTan();
		case Token::Sin                 : return parseSin();
		case Token::Cos                 : return parseCos();
		case Token::ArcTan              : return parseArcTan();
		case Token::ArcSin              : return parseArcSin();
		case Token::ArcCos              : return parseArcCos();
		case Token::Sqrt                : return parseSqrt();
		case Token::Round               : return parseRound();
		case Token::GetDirection        : return parseGetDirection();
		case Token::Mod                 : return parseMod();

//END GENERATED parser_statements_cpp CODE
		case Token::Error		: return new TreeNode(currentToken);
		default : {
			//Token type is something else...
			////qDebug() << "Parser::parseStatement(): I don't know this Token type.";
			////qDebug() << "Look: " << currentToken->look() << " type: " << currentToken->type();
			addError(i18n("You cannot put '%1' here.", currentToken->look()), *currentToken, 0);
			finished = true;
			return new TreeNode(currentToken);
		}
	}
}


TreeNode* Parser::parseFactor()
{
// 	//qDebug() << "Parser::parseFactor()";
	Token* rememberedToken;
	TreeNode* node;
	switch (currentToken->type()) {
		case Token::ParenthesisOpen:
			rememberedToken = currentToken;
			nextToken();
			node = parseExpression();
			skipToken(Token::ParenthesisClose, *rememberedToken);
			break;

		case Token::FunctionCall:
			node = parseFunctionCall();
			break;

		case Token::Variable:
			node = new TreeNode(currentToken);
			nextToken();
			break;

		case Token::String:
			node = new TreeNode(currentToken);
			{ // extra scope to localize the QString 'str'
				QString str = currentToken->look();
				if (!currentToken->look().endsWith('\"')) {
					str += QLatin1String("\"");
					addError(i18n("Text string was not properly closed, expected a double quote, ' \" ', to close the string."), *currentToken, 0);
				}
				node->value()->setString(str.mid(1, str.length() - 2));
			}
			nextToken();
			break;

		case Token::Number:
			node = new TreeNode(currentToken);
			{
				bool ok = true;
				double num = currentToken->look().toDouble(&ok);
				if (ok) {
					node->value()->setNumber(num);
					nextToken();
				} else {
					qCritical("Parser::parseFactor(): could not parse a number");
				}
			}
			break;

		case Token::True:
			node = new TreeNode(currentToken);
			node->value()->setBool(true);
			nextToken();
			break;

		case Token::False:
			node = new TreeNode(currentToken);
			node->value()->setBool(false);
			nextToken();
			break;

// 		case Token::Run:
// 			node = ExternalRun();
// 			break;
// 
// 		case tokInputWindow:
// 			node = InputWindow();
// 			break;
// 
// 		case tokRandom:
// 			node = Random();
// 			break;
// 
// 		case Token::EndOfLine:
// 			node = new TreeNode(currentToken);
// 			break;


			default:
				// TODO maybe precheck if this is a statement that returns something
				node = parseStatement();
				break;


// 		default:
// 			QString s = currentToken->look();
// 			if ( s.isEmpty() || currentToken->type() == Token::EndOfInput )
// 			{
// // 				Error(currentToken, i18n("INTERNAL ERROR NR %1: please sent this Logo script to KTurtle developers").arg(1), 1020);
// 				// if this error occurs the see the Parser::Repeat for the good solution using 'preservedToken'
// 			}
// 			else
// 			{
// // 				Error(currentToken, i18n("Cannot understand '%1', expected an expression after the '%2' command").arg(s).arg(preservedToken->look()), 1020);
// 			}
// 			node = new TreeNode(currentToken);
// 			nextToken();
// 			break;
	}
	return node;
}


TreeNode* Parser::parseSignedFactor()
{
// 	//qDebug() << "Parser::parseSignedFactor()";
	// see if there is a plus, minus or 'not' in front of a factor
	Token* rememberedToken;
	TreeNode* node;
	switch (currentToken->type()) {
		case Token::Addition:
			nextToken();
			return parseFactor();
			break;

		case Token::Subtraction:
			rememberedToken = currentToken;
			nextToken();
			node = parseFactor();
			if (node->token()->type() == Token::Number) {
				// in case of just a constant (-3) situation
				node->value()->setNumber(-1 * node->value()->number());
				return node;
			} else {
				// in case of a variable or other situation (-$a)
				TreeNode* minusNode = new TreeNode(rememberedToken);
				minusNode->appendChild(node);
				return minusNode;
			}
			break;

		case Token::Not:
			rememberedToken = currentToken;
			nextToken();
			node = parseFactor();
			{ // extra scope needed to localize not_Node
				TreeNode* notNode = new TreeNode(rememberedToken);
				notNode->appendChild(node);
				return notNode;
			}
			break;

		default:
			// no 'sign', no problem; just had to check if there was any... continue with the real work:
			return parseFactor();
			break;
	}
}



TreeNode* Parser::parseTerm()
{
// 	//qDebug() << "Parser::parseTerm()";
	TreeNode* termNode = parseSignedFactor();
	TreeNode* pos = termNode;
	TreeNode* left = nullptr;
	TreeNode* right = nullptr;

	while ( (currentToken->type() == Token::Multiplication) ||
	        (currentToken->type() == Token::Division) ||
	        (currentToken->type() == Token::Power)  ) {
		// while we find a multiplicative operator do...
		left = pos;
		pos = new TreeNode(currentToken);
		pos->appendChild(left);
		nextToken();
		right = parseSignedFactor();
		if (right != nullptr)
			pos->appendChild(right);
		termNode = pos;
	}
	return termNode;
}


TreeNode* Parser::parseExpression()
{
// 	//qDebug() << "Parser::parseExpression()";
	TreeNode* expressionNode = parseTerm();
	TreeNode* pos = expressionNode;
	TreeNode* left = nullptr;
	TreeNode* right = nullptr;

	Token* prevToken = nullptr;

	while ((currentToken->type() == Token::Addition) ||
	       (currentToken->type() == Token::Subtraction) ||
	       (currentToken->type() == Token::GreaterThan) ||
	       (currentToken->type() == Token::GreaterOrEquals) ||
	       (currentToken->type() == Token::LessThan) ||
	       (currentToken->type() == Token::LessOrEquals) ||
	       (currentToken->type() == Token::Equals) ||
	       (currentToken->type() == Token::NotEquals) ||
	       (currentToken->type() == Token::Or) ||
	       (currentToken->type() == Token::And)) {
		left = pos;
		pos = new TreeNode(currentToken);
		pos->appendChild(left);
		prevToken = currentToken;
		nextToken();
		
		if(prevToken->type() == Token::And ||
		   prevToken->type() == Token::Or)
			right = parseExpression();
		else
			right = parseTerm();
		if (right != nullptr) pos->appendChild(right);
		expressionNode = pos;
	}
	return expressionNode;
}



void Parser::appendArguments(TreeNode* node)
{
	// appends the nodes of arguments to the node, does not do any type/quantity checking.
	// this because it's also used for the user defined 'functionCall' where we don't know type nor quantity
	// we also don't know if this node is (part of) an argument itself
	if (currentToken->type() == Token::EndOfLine ||
	    currentToken->type() == Token::EndOfInput ||
	    currentToken->type() == Token::ArgumentSeparator ||
	    currentToken->type() == Token::ParenthesisClose ||
	    currentToken->type() == Token::ScopeOpen ||
	    currentToken->type() == Token::ScopeClose) return;
	node->appendChild(parseExpression()); // append the first parameter
	while (skipToken(Token::ArgumentSeparator)) {  // pushes through the comma
		if (currentToken->type() == Token::EndOfLine ||
		    currentToken->type() == Token::EndOfInput)
			return; // catch forgotten expressions (go 10,)
		node->appendChild(parseExpression());
	}
}



//BEGIN GENERATED parser_cpp CODE

/* The code between the line that start with "//BEGIN GENERATED" and "//END GENERATED"
 * is generated by "generate.rb" according to the definitions specified in
 * "definitions.rb". Please make all changes in the "definitions.rb" file, since all
 * all change you make here will be overwritten the next time "generate.rb" is run.
 * Thanks for looking at the code!
 */

TreeNode* Parser::parseVariable() {
	// This is called to the variable is the first token on a line.
	// There has to be an assignment token right after it...
	TreeNode* variableNode = new TreeNode(currentToken);
	nextToken();
	Token* remeberedToken = new Token(*currentToken);
	skipToken(Token::Assign, *variableNode->token());
	TreeNode* assignNode = new TreeNode(remeberedToken);
	assignNode->appendChild(variableNode);      // the LHV; the symbol
	assignNode->appendChild(parseExpression()); // the RHV; the expression
	return assignNode;
}

TreeNode* Parser::parseFunctionCall() {
	TreeNode* node = new TreeNode(currentToken);
	node->token()->setType(Token::FunctionCall);
	nextToken();
	appendArguments(node);
	return node;
}
TreeNode* Parser::parseScopeOpen() {

	TreeNode* scopeNode = new TreeNode(new Token(Token::Scope, QStringLiteral("{...}"), currentToken->startRow(), currentToken->startCol(), 0, 0));
	delete currentToken;
	nextToken();
	// cannot change the currentScope to this scope cauz we still have to work in the currentScope
	// so we set the newScope, which the currentScope will become after parsing the current statement
	newScope = scopeNode;
	return scopeNode;
}
TreeNode* Parser::parseScopeClose() {

	TreeNode* node = new TreeNode(currentToken);
	int endRow = currentToken->endRow();
	int endCol = currentToken->endCol();
	nextToken();
	while (currentToken->type() == Token::EndOfLine) {  // allow newlines before else
		delete currentToken;
		nextToken();
	}
	if (currentScope->parent() && 
		currentScope->parent()->token()->type() == Token::If && 
		currentToken->type() == Token::Else) {
		currentScope->parent()->appendChild(parseElse());
	}
	if (currentScope != rootNode) {
		// find the parent scope of this scope, and make it current
		TreeNode* parentScope = currentScope;
		do {
			parentScope = parentScope->parent();
		} while (parentScope != rootNode && parentScope->token()->type() != Token::Scope);
		currentScope->token()->setEndRow(endRow);
		currentScope->token()->setEndCol(endCol);
		currentScope = parentScope;
	}
	return node;
}
TreeNode* Parser::parseExit() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseIf() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	node->appendChild(parseExpression());
	if (currentToken->type() == Token::ScopeOpen) {
		node->appendChild(parseScopeOpen());  // if followed by a scope
	}
	return node;
}
TreeNode* Parser::parseElse() {

	nextToken();
	if (currentToken->type() == Token::ScopeOpen) {
		return parseScopeOpen();  // if followed by a scope
	}
	return parseStatement();    // if followed by single statement
}
TreeNode* Parser::parseRepeat() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	node->appendChild(parseExpression());
	if (currentToken->type() == Token::ScopeOpen) {
		node->appendChild(parseScopeOpen());  // if followed by a scope
	} else {
		node->appendChild(parseStatement());  // if followed by single statement
	}
	return node;
}
TreeNode* Parser::parseWhile() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	node->appendChild(parseExpression());
	if (currentToken->type() == Token::ScopeOpen) {
		node->appendChild(parseScopeOpen());  // if followed by a scope
	} else {
		node->appendChild(parseStatement());  // if followed by single statement
	}
	return node;
}
TreeNode* Parser::parseFor() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	Token* firstToken = currentToken;
	nextToken();
	if (firstToken->type() == Token::Variable && currentToken->type() == Token::Assign) {
		node->token()->setType(Token::ForTo);
		node->appendChild(new TreeNode(firstToken));
		nextToken();
		node->appendChild(parseExpression());
		skipToken(Token::To, *node->token());
		node->appendChild(parseExpression());
		if (currentToken->type() == Token::Step) {
			nextToken();
			node->appendChild(parseExpression());
		} else {
			TreeNode* step = new TreeNode(new Token(Token::Number, QStringLiteral("1"), 0,0,0,0));
			step->setValue(Value(1.0));
			node->appendChild(step);
		}
//	} else if (currentToken->type() == Token::In) {
//		// @TODO something for a for-in loop
	} else {
		addError(i18n("'for' was called wrongly"), *node->token(), 0);
		finished = true;  // abort after this error
		return node;
	}

	if (currentToken->type() == Token::ScopeOpen) {
		node->appendChild(parseScopeOpen());  // if followed by a scope
	} else {
		node->appendChild(parseStatement());  // if followed by single statement
	}

	return node;
}
TreeNode* Parser::parseBreak() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseReturn() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseWait() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseAssert() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseLearn() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	node->appendChild(new TreeNode(new Token(*currentToken)));
	skipToken(Token::FunctionCall, *node->token());
	
	TreeNode* argumentList = new TreeNode(new Token(Token::ArgumentList, QStringLiteral("arguments"), 0, 0, 0, 0));
	while (currentToken->type() == Token::Variable) {
		// cannot just call appendArguments here because we're only appending Token::Variable tokens
		argumentList->appendChild(new TreeNode(currentToken));
		nextToken();
		if (currentToken->type() != Token::ArgumentSeparator) break;
		nextToken();
	}
	node->appendChild(argumentList);
	
	//Skip all the following EndOfLine's
	while (currentToken->type() == Token::EndOfLine) {
		delete currentToken;
		nextToken();
	}

	if (currentToken->type() == Token::ScopeOpen) {
		node->appendChild(parseScopeOpen());  // if followed by a scope
	} else {
		addError(i18n("Expected a scope after the 'learn' command"), *node->token(), 0);
	}
	return node;
}
TreeNode* Parser::parseReset() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseClear() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseCenter() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseGo() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseGoX() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseGoY() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseForward() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseBackward() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseDirection() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseTurnLeft() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseTurnRight() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parsePenWidth() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parsePenUp() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parsePenDown() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parsePenColor() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}

TreeNode* Parser::parseCanvasColor() {
	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}

TreeNode* Parser::parseCanvasSize() {
	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}

TreeNode* Parser::parseSpriteShow() {
	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseSpriteHide() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parsePrint() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseFontSize() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseRandom() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseGetX() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseGetY() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseMessage() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseAsk() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parsePi() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseTan() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseSin() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseCos() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseArcTan() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseArcSin() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseArcCos() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseSqrt() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseRound() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseGetDirection() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	skipToken(Token::EndOfLine, *node->token());
	return node;
}
TreeNode* Parser::parseMod() {

	TreeNode* node = new TreeNode(currentToken);
	nextToken();
	appendArguments(node);
	skipToken(Token::EndOfLine, *node->token());
	return node;
}

//END GENERATED parser_cpp CODE



