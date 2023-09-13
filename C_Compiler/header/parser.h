#pragma once
#include <vector>
#include "token.h"
using std::vector;

//入力されたトークン列を解析して抽象構文木を作成するクラス
class Parser {
public:
	enum class nodeType {
		Expr,
		Assign,  // = 
		LovalVal,
		Add,
		Sub,
		Mul,
		Div,
		Eq,
		Ne,
		Lt,
		Le,
		Gt,
		Ge,
		Num,
		None,
	};

	struct Node {
		nodeType type;
		Node* lhs;
		Node* rhs;
		int val; //typeがNumのときだけ使う
		int offset; //typeがLocalValの時だけ使う
	};

	vector<Node> mNodeTbl;
	vector<Node*> mRootNodeTbl;
	size_t mCurrentPos;
	vector<Token> mTokenTbl;

	Node& getLastNode() {
		return mNodeTbl[mNodeTbl.size() - 1];
	}

	Parser()
		:mNodeTbl()
		, mCurrentPos()
		, mTokenTbl()
	{}

	void Parse(vector<Token>& tokenTbl);
private:
	//------------------------------------------
	//抽象構文木の生成文法
	//------------------------------------------

	Node* Program();
	Node* Statement();
	Node* Expr();
	Node* Assign();
	Node* Equality();
	Node* Relational();
	Node* Add();
	Node* Mul();
	Node* Unary();
	Node* Primaly();

	//作成したNodeを格納する
	Node* PushBackNode(Node n);

	//トークンを調べて該当するノードタイプを返す
	nodeType GetNodeType(Token& token);
};
