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
	//���ۍ\���؂̐������@
	//------------------------------------------

	Node* Expr();
	Node* Relational();
	Node* Add();
	Node* Mul();
	Node* Primaly();
	Node* Unary();

	//�쐬����Node���i�[����
	Node* PushBackNode(Node n);
};
