#pragma once
#include <vector>
#include <cassert>
#include <unordered_map>
#include "token.h"
using std::vector;
using std::string_view;
using std::unordered_map;

//���͂��ꂽ�g�[�N�������͂��Ē��ۍ\���؂��쐬����N���X
class Parser {
public:
	enum class nodeType {
		Expr,
		Assign,  // = 
		LocalVal,
		//���ʎq�Ƃ̌������̂��߂�_��ǉ����Ă���
		If_,
		For_,
		While_,
		Block,
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
		Return,
		None,
	};

	struct Node {
		nodeType type;
		Node* lhs;
		Node* rhs;
		Node* middle; //type��for�̎������g��
		int val; //type��Num�̂Ƃ������g��
		int offset; //type��LocalVal�̎������g��
		vector<Node*> innerBlockNodeTbl; // type��Block�̎������g��
	};


	// ���[�J���ϐ��̏���ۑ����邽�߂̌^
	unordered_map<string_view, int> mLValMap;
	
	vector<Node> mNodeTbl;
	vector<Node*> mRootNodeTbl;


	Node& getLastNode() {
		return mNodeTbl[mNodeTbl.size() - 1];
	}

	size_t getTotalBytesOfLVal() {
		return mLValMap.size() * 8;
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
	//program	 = statement*
	//statement	 = expr ";"
	//			| "if" "(" expr ")" statement ( "else" stmt )?
	//			| "while" "(" expr ")" statement
	//			| "for" "(" expr? ";" expr? ";" expr? ")" statement
	//			| "return" expr ";"
	//			| "{" statement* "}"


	//expr		 = assign
	//assign	 = equality ( "=" assign)?
	//equality   = relational ("==" relational | "!=" relational)*
	//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
	//add        = mul ("+" mul | "-" mul)*
	//mul        = unary ("*" unary | "/" unary)*
	//unary      = ("+" | "-")? primary
	//primary    = num | ident | "(" expr ")"

	void Program();
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
	nodeType GetNodeType(const Token& token) const ;

	bool isEndOfState() {
		assert(mCurrentPos < mTokenTbl.size());
		return mTokenTbl[mCurrentPos].isReserved(';');
	}

	vector<Token> mTokenTbl;
	size_t mCurrentPos;
	const Token& getCurTk() {
		return mTokenTbl[mCurrentPos];
	}
};
