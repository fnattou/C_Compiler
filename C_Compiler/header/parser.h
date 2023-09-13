#pragma once
#include <vector>
#include "token.h"
using std::vector;

//���͂��ꂽ�g�[�N�������͂��Ē��ۍ\���؂��쐬����N���X
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
		int val; //type��Num�̂Ƃ������g��
		int offset; //type��LocalVal�̎������g��
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
	//���ۍ\���؂̐������@
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

	//�쐬����Node���i�[����
	Node* PushBackNode(Node n);

	//�g�[�N���𒲂ׂĊY������m�[�h�^�C�v��Ԃ�
	nodeType GetNodeType(Token& token);
};
