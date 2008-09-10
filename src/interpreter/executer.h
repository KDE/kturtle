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

#ifndef _EXECUTER_H_
#define _EXECUTER_H_

#include <QHash>
#include <QObject>
#include <QStack>


#include "errormsg.h"
#include "token.h"
#include "treenode.h"



// some typedefs and a struct for the template classes used:

typedef QHash<QString, Value>     VariableTable;
typedef QHash<QString, TreeNode*> FunctionTable;
typedef struct {
	TreeNode*      function;      // pointer to the node of the function caller
	VariableTable* variableTable; // pointer to the variable table of the function
} CalledFunction;
typedef QStack<CalledFunction>    FunctionStack;



/**
 * @short Step-wise execution of a node tree, collecting errors in the ErrorList.
 *
 * The Executer 'executes' the node tree, as yielded by the Parser, step by step.
 * When errors occur they are put in the ErrorList as supplied to the constuctor.
 *
 * The Executer has a globalVariableTable where is stores the content of variables,
 * and a functionTable that contains pointer to the 'learned' functions.
 * When running into a function a local variable table and a pointer to the
 * functionCallNode are put onto the functionStack.
 *
 * Executer inherits from QObject for the SIGNALS/SLOTS mechanism.
 * Signals are emitted for all external things the Executer has to trigger (like
 * drawing).
 *
 * A large part of the code of this class is generated code.
 *
 * @author Cies Breijs
 */
class Executer : public QObject
{
	Q_OBJECT

	public:
		/**
		 * @short Constructor. Initialses the Executer.
		 * does nothing special. @see initialize().
		 */
		Executer(bool testing = false) : m_testing(testing) {}
		/**
		 * @short Destuctor. Does nothing at special.
		 */
		virtual ~Executer() {}


		/**
		 * @short Initialses (resets) the Executer.
		 * @param tree      pointer to the base node of the tree as provided
		 *                  by the Parser
		 * @param errorList pointer to a QList for ErrorMessage objects, when
		 *                  error occur they will be stored here
		 */
		void initialize(TreeNode* tree, ErrorList* _errorList);

		/**
		 * @short Executes one 'step' (usually a TreeNode).
		 * This methods scans over the node tree to find the next TreeNode to
		 * execute, and executes it. It starts by the leafs, and works it way to
		 * the root. @see isFinished
		 */
		void           execute();

		/**
		 * @short Reflects if the Executer has finished executing the node tree.
		 * @return TRUE when execution has finished, otherwise FALSE.
		 */
		bool           isFinished() const { return finished; }


	private slots:
		/// Used by the singleshot wait timer.
		void stopWaiting() { waiting = false; }


	private:
		/// Executes a single TreeNode, mainly a switch to the individual executer* functions.
		void           execute(TreeNode* node);

		/// Adds an error to the error list.
		void           addError(const QString& s, const Token& t, int code);

		/// Checks the parameter quantity of @p n matches @p quantity, if not it adds an error with @p errorCode
		bool           checkParameterQuantity(TreeNode* n, uint quantity, int errorCode);

		/// Checks the types of @p n 's parameters match the type @p valueType, if not it adds an error with @p errorCode
		bool           checkParameterType(TreeNode* n, int valueType, int errorCode);

		/// @returns the variable table of the current function, or the globalVariableTable if not running in a function
		VariableTable* currentVariableTable();



		/// QHash containing pointers to the 'learned' functions
		FunctionTable       functionTable;

		/// QHash containing the global variables
		VariableTable       globalVariableTable;

		/// Stores both pointers to functionNodes and accompanying local variable table using the predefined struct.
		FunctionStack       functionStack;

		/// Pointer to the error list as supplied to the constuctor
		ErrorList          *errorList;



		/// Pointer to the root node of the tree
		TreeNode*      rootNode;

		/// This points to a scope node when we have to get into a new scope, otherwise it should be zero
		TreeNode*      newScope;

		/// This point to the current node in the tree
		TreeNode*      currentNode;

		/// Zero except when returning from a function
		Value*         returnValue;

		/// TRUE when execution has finished
		bool           finished;

		/// TRUE when execution is waiting
		bool           waiting;

		/// TRUE when breaking from a loop
		bool           breaking;

		/// TRUE when returning from a function
		bool           returning;

		/// TRUE when the next call of execute() should staight execute the current node (not walk the tree first)
		bool           executeCurrent;

		bool           m_testing;



// Next you find individual execute functions as generated:

//BEGIN GENERATED executer_h CODE

/* The code between the line that start with "//BEGIN GENERATED" and "//END GENERATED"
 * is generated by "generate.rb" according to the definitions specified in
 * "definitions.rb". Please make all changes in the "definitions.rb" file, since all
 * all change you make here will be overwritten the next time "generate.rb" is run.
 * Thanks for looking at the code!
 */

		void executeUnknown(TreeNode* node);
		void executeRoot(TreeNode* node);
		void executeScope(TreeNode* node);
		void executeVariable(TreeNode* node);
		void executeFunctionCall(TreeNode* node);
		void executeExit(TreeNode* node);
		void executeIf(TreeNode* node);
		void executeElse(TreeNode* node);
		void executeRepeat(TreeNode* node);
		void executeWhile(TreeNode* node);
		void executeFor(TreeNode* node);
		void executeForTo(TreeNode* node);
		void executeBreak(TreeNode* node);
		void executeReturn(TreeNode* node);
		void executeWait(TreeNode* node);
		void executeAnd(TreeNode* node);
		void executeOr(TreeNode* node);
		void executeNot(TreeNode* node);
		void executeEquals(TreeNode* node);
		void executeNotEquals(TreeNode* node);
		void executeGreaterThan(TreeNode* node);
		void executeLessThan(TreeNode* node);
		void executeGreaterOrEquals(TreeNode* node);
		void executeLessOrEquals(TreeNode* node);
		void executeAddition(TreeNode* node);
		void executeSubstracton(TreeNode* node);
		void executeMultiplication(TreeNode* node);
		void executeDivision(TreeNode* node);
		void executePower(TreeNode* node);
		void executeAssign(TreeNode* node);
		void executeLearn(TreeNode* node);
		void executeArgumentList(TreeNode* node);
		void executeReset(TreeNode* node);
		void executeClear(TreeNode* node);
		void executeCenter(TreeNode* node);
		void executeGo(TreeNode* node);
		void executeGoX(TreeNode* node);
		void executeGoY(TreeNode* node);
		void executeForward(TreeNode* node);
		void executeBackward(TreeNode* node);
		void executeDirection(TreeNode* node);
		void executeTurnLeft(TreeNode* node);
		void executeTurnRight(TreeNode* node);
		void executePenWidth(TreeNode* node);
		void executePenUp(TreeNode* node);
		void executePenDown(TreeNode* node);
		void executePenColor(TreeNode* node);
		void executeCanvasColor(TreeNode* node);
		void executeCanvasSize(TreeNode* node);
		void executeSpriteShow(TreeNode* node);
		void executeSpriteHide(TreeNode* node);
		void executePrint(TreeNode* node);
		void executeFontSize(TreeNode* node);
		void executeRandom(TreeNode* node);
		void executeGetX(TreeNode* node);
		void executeGetY(TreeNode* node);
		void executeMessage(TreeNode* node);
		void executeAsk(TreeNode* node);
		void executePi(TreeNode* node);
		void executeTan(TreeNode* node);
		void executeSin(TreeNode* node);
		void executeCos(TreeNode* node);
		void executeArcTan(TreeNode* node);
		void executeArcSin(TreeNode* node);
		void executeArcCos(TreeNode* node);
		void executeSqrt(TreeNode* node);
		void executeExp(TreeNode* node);
		void executeRound(TreeNode* node);

//END GENERATED executer_h CODE

		TreeNode* getParentOfTokenTypes(TreeNode* child, QList<int>* types);
		inline void printExe();



	signals:
		void currentlyExecuting(TreeNode* node);
		void variableTableUpdated(const QString& name, const Value& value);
		void functionTableUpdated(const QString& name, const QStringList& parameters);

		void getX(double&);
		void getY(double&);
		void ask(QString& value);
		void message(const QString& text);

// next you find generated signals that can be emitted:

//BEGIN GENERATED executer_emits_h CODE

/* The code between the line that start with "//BEGIN GENERATED" and "//END GENERATED"
 * is generated by "generate.rb" according to the definitions specified in
 * "definitions.rb". Please make all changes in the "definitions.rb" file, since all
 * all change you make here will be overwritten the next time "generate.rb" is run.
 * Thanks for looking at the code!
 */

		void reset();
		void clear();
		void center();
		void go(double, double);
		void goX(double);
		void goY(double);
		void forward(double);
		void backward(double);
		void direction(double);
		void turnLeft(double);
		void turnRight(double);
		void penWidth(double);
		void penUp();
		void penDown();
		void penColor(double, double, double);
		void canvasColor(double, double, double);
		void canvasSize(double, double);
		void spriteShow();
		void spriteHide();
		void print(const QString&);
		void fontSize(double);

//END GENERATED executer_emits_h CODE

};

#endif  // _EXECUTER_H_
