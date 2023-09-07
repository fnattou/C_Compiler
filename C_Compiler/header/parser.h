#pragma once
#include <vector>
#include "token.h"
using std::vector;

//入力されたトークン列を解析して抽象構文木を作成するクラス
class Parser {
public:
	enum class nodeType {
		Add, 
		Sub,
		Mul,
		Div,
		Num,
	};

	struct Node {
		nodeType type;
		Node* lhs;
		Node* rhs;
		int val; //typeがNumのときだけ使う
	};

	vector<Node> mNodeTbl;
	size_t mCurrentPos;
	vector<Token> mTokenTbl;

	Node& getLastNode() {
		return mNodeTbl[mNodeTbl.size() - 1];
	}

	Parser()
		:mNodeTbl()
		,mCurrentPos()
		,mTokenTbl()
	{}

	void Parse(vector<Token>& tokenTbl);
private:
	//------------------------------------------
	//抽象構文木の生成文法
	//------------------------------------------

	Node* Expr();
	Node* Relational();
	Node* Add();
	Node* Mul();
	Node* Primaly();
	Node* Unary();

	//作成したNodeを格納する
	Node* PushBackNode(Node n);
};
