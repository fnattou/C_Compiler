#pragma once
#include <vector>
#include <cassert>
#include <unordered_map>
#include <set>
#include "token.h"
using std::vector;
using std::string_view;
using std::unordered_map;

//入力されたトークン列を解析して抽象構文木を作成するクラス
class Parser {
public:
	enum class nodeType {
		Expr,
		Assign,			//代入
		If_,
		For_,
		While_,
		Block,			//複文
		CallFunc,		//関数呼び出し
		DeclareFunc,	//関数宣言
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
		LocalVal,		//ローカル変数
		Addr,			//アドレス演算子
		Deref,			//アドレス外し
		Return,
		None,
	};
	struct FuncInfo;
	struct Node {
		nodeType type;
		Node* lhs;
		Node* rhs;
		Node* middle; //typeがforの時だけ使う
		int val; //typeがNumのときだけ使う
		int offset; //typeがLocalValの時だけ使う
		vector<Node*> innerBlockNodeTbl; // typeがBlockの時だけ使う
		vector<Node*> argumentNodeTbl; // typeがCallFuncの時だけ使う
		FuncInfo* funcInfoPtr; //typeがFuncのときだけ使う
	};

	struct FuncInfo {
		string_view name;
		unordered_map<string_view, int> lValMap;
		vector<Node*> argumentNodeTbl; 
	};


	// ローカル変数のスタックからのオフセットを保存するための型
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
	//抽象構文木の生成文法
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
	//			| ident ( "(" (ident "," )* ident? ")"　)?
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
