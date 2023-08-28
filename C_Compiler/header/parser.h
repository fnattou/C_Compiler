#pragma once
#include <vector>
#include "token.h"
using std::vector;

//���͂��ꂽ�g�[�N�������͂��Ē��ۍ\���؂��쐬����N���X
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
		int val; //type��Num�̂Ƃ������g��
	};

	vector<Node> mNodeTbl;
	size_t mCurrentPos;
	vector<Token> mTokenTbl;

	Node& getFirstNode() {
		return mNodeTbl[0];
	}

	Parser()
		:mNodeTbl()
		,mCurrentPos()
		,mTokenTbl()
	{}

	Parser(vector<Token>& tbl)
		:mNodeTbl()
		,mCurrentPos()
		,mTokenTbl(tbl)
	{}

	void Parse(vector<Token>& tokenTbl);
private:
	Node* Expr();
	Node* Mul();
	Node* Primaly();
};
