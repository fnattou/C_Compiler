#pragma once
#include <vector>
#include <cassert>
#include <unordered_map>
#include <set>
#include "token.h"
using std::vector;
using std::string_view;
using std::unordered_map;

//���͂��ꂽ�g�[�N�������͂��Ē��ۍ\���؂��쐬����N���X
class Parser {
public:
	enum class nodeType {
		Expr,
		Assign,			//���
		If_,
		For_,
		While_,
		Block,			//����
		CallFunc,		//�֐��Ăяo��
		DeclareFunc,	//�֐��錾
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
		LocalVal,		//���[�J���ϐ�
		Addr,			//�A�h���X���Z�q
		Deref,			//�A�h���X�O��
		Return,
		None,
	};
	struct FuncInfo;
	struct Node {
		nodeType type;
		Node* lhs;
		Node* rhs;
		Node* middle; //type��for�̎������g��
		int val; //type��Num�̂Ƃ������g��
		int offset; //type��LocalVal�̎������g��
		vector<Node*> innerBlockNodeTbl; // type��Block�̎������g��
		vector<Node*> argumentNodeTbl; // type��CallFunc�̎������g��
		FuncInfo* funcInfoPtr; //type��Func�̂Ƃ������g��
	};

	struct FuncInfo {
		string_view name;
		unordered_map<string_view, int> lValMap;
		vector<Node*> argumentNodeTbl; 
	};


	// ���[�J���ϐ��̃X�^�b�N����̃I�t�Z�b�g��ۑ����邽�߂̌^
	unordered_map<string_view, int> mLValMap;
	unordered_map<string_view, FuncInfo> mFuncInfoTbl;
	FuncInfo* mCurrentFuncInfoPtr;
	
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
	//			| func statement* 
	//function   = ident "(" (ident ",")* ident? ")" "{" statement* "}"
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
	//		    | "*" unary
	//			| "&" unary
	//primary    = num 
	//			| ident ( "(" (ident "," )* ident? ")"�@)?
	//			| "(" expr ")"

	void Program();
	Node* Function();
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
