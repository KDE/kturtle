/*=============================================================================
author        :Walter Schreppers
filename      :treenode.h
description   :A node in an n-ary tree
bugreport(log):/
=============================================================================*/

#ifndef _TREENODE_H_
#define _TREENODE_H_

#include <list>
#include <string>

#include <qstring.h>

#include "number.h"

//using namespace std;

//BUGS: prevSibling and nextSibling sometimes segfault and are not optimal, in short, don't use em! :)


enum NodeType { 
                Unknown = -1,
                programNode,
                functionNode,
                functionCallNode,
                funcReturnNode,
                returnNode,
                breakNode,
                idListNode,
                blockNode,
                forNode,
                forEachNode,
                whileNode,
                ifNode,
                printNode,
                inputNode,
                assignNode,
                expressionNode,
                idNode,
                constantNode,
                stringConstantNode,
            
                addNode,
                mulNode,
                divNode,
                subNode,
                minusNode,

                nodeGE,
                nodeGT,
                nodeLE,
                nodeLT,
                nodeNE,
                nodeEQ,

                andNode,
                orNode,
                notNode,
                
                runNode,
                writeNode,
                substrNode,

                ClearNode,
                GoNode,
                GoXNode,
                GoYNode,
                ForwardNode,
                BackwardNode,
                DirectionNode,
                TurnLeftNode,
                TurnRightNode,
                CenterNode,
                SetPenWidthNode,
                PenUpNode,
                PenDownNode,
                SetFgColorNode,
                SetBgColorNode,
                ResizeCanvasNode,
                SpriteShowNode,
                SpriteHideNode,
                SpritePressNode,
                SpriteChangeNode
                
              };


class TreeNode:public list<TreeNode*> {

  public:
  
    //constructors/destructor
    //=======================
    TreeNode();
    TreeNode(TreeNode*); //give parent

    TreeNode(NodeType);
    TreeNode(NodeType,int,int); //row and col
    
    
    virtual ~TreeNode();
  
    //public members
    //==============
    void init();
    void setParent(TreeNode* p);
    TreeNode* getParent() {return parent;}
    
    virtual void show(int indent=0);
    void showTree(TreeNode* node, int indent=0) const;
    
    void appendChild(TreeNode*);
    void appendSibling(TreeNode*); //works only if it has parent set!
    
    TreeNode* firstChild();
    TreeNode* secondChild();
    TreeNode* thirdChild();
    TreeNode* fourthChild();
    TreeNode* fifthChild();
    
    TreeNode* lastChild();    
    TreeNode* nextSibling();   
    TreeNode* prevSibling();
    
    NodeType getType();
    void setType(NodeType t);

    void setKey(const QString&);
    QString getKey();  
        
    void setName(const string&);
    string getName() const;  
    
    void setValue(const Number&);
    void setValue(double d){ value = d; }
    void setValue(const string& s){ value = s; }
    
    Number getValue();
    int getRow(){ return fRow; }
    int getCol(){ return fCol; }

    void setStrValue(const string&);
    string getStrValue();
    
    TreeNode::iterator lookup(); //gives location in parent list as iterator (used by prevSibling and nextSibling)
    bool hasChildren() { return size()!=0; }

    TreeNode& operator=(const TreeNode&);
  
  protected:
    TreeNode* parent;
    
        
  private:

    //private members
    //===============
    void destroy(TreeNode*);

    //private locals
    //==============
    QString key;
    string name;
    NodeType fType;
    Number value;
    string strValue;
    
    int fRow, fCol; //for runtime error messages.
    
};

#endif // _TREENODE_H_



