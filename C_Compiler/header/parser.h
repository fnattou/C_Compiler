#pragma once
#include <vector>
#include <cassert>
#include <unordered_map>
#include "token.h"
using std::vector;
using std::string_view;
using std::unordered_map;

//入力されたトークン列を解析して抽象構文木を作成するクラス
class Parser {
public:
	enum class nodeType {
		Expr,
		Assign,  // = 
		LocalVal,
		//識別子との見分けのために_を追加している
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
		Node* middle; //typeがforの時だけ使う
		int val; //typeがNumのときだけ使う
		int offset; //typeがLocalValの時だけ使う
		vector<Node*> innerBlockNodeTbl; // typeがBlockの時だけ使う
	};


	// ローカル変数の情報を保存するための型
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
	//抽象構文木の生成文法
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

	//作成したNodeを格納する
	Node* PushBackNode(Node n);

	//トークンを調べて該当するノードタイプを返す
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
