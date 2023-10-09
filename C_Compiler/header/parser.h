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
		Sizeof,			//sizeof演算子
		Return,
		None,
	};
	struct FuncInfo;
	struct ValTypeInfo;
	struct Node {
		nodeType type;
		Node* lhs;
		Node* rhs;
		Node* middle; //typeがforの時だけ使う
		int val; //typeがNumのときだけ使う
		int offset; //typeがLocalValの時だけ使う
		vector<Node*> innerBlockNodeTbl; // typeがBlockの時だけ使う
		vector<Node*> argumentNodeTbl; // typeがCallFuncの時だけ使う実引数情報
		FuncInfo* funcInfoPtr; //typeがFuncのときだけ使う
		const ValTypeInfo* valTypeInfoPtr; //変数の時の型情報
	};

	struct FuncInfo {
		string_view name;
		unordered_map<string_view, int> lValOfsMap;
		unordered_map<string_view, const ValTypeInfo*> lValTypeMap;
		vector<Node*> argumentNodeTbl; //仮引数情報
		ValTypeInfo* returnValTypeInfoPtr; //返り値の型情報
		size_t totalBytes; //ローカル変数のサイズ合計

		//関数内のローカル変数情報を登録する。仮引数もここに含まれる
		size_t pushBackToValMap(string_view name, size_t byteSize, const ValTypeInfo* ptr) {
			lValTypeMap.emplace(name, ptr);
			totalBytes += byteSize;
			lValOfsMap.emplace(name, totalBytes);
			return totalBytes;
		}
	};

	struct ValTypeInfo {
		enum class ValType {
			Int,
			Ptr,
		} type;
		ValTypeInfo* toPtr;
		int getByteSize() const {
			return (type == ValType::Int) ? 4 : 8;
		}

		//ポインタがさす型のサイズを取得
		int getToTypeSize() const {
			if (!toPtr) return 0;
			return toPtr->getByteSize();
		}
	};

	// ローカル変数のスタックからのオフセットを保存するための型
	unordered_map<string_view, int> mlValOfsMap;
	unordered_map<string_view, FuncInfo> mFuncInfoTbl;
	//変数の型情報の保存場所
	vector<ValTypeInfo> mTypeInfoTbl;

	FuncInfo* mCurrentFuncInfoPtr;
	
	vector<Node> mNodeTbl;
	vector<Node*> mRootNodeTbl;


	Node& getLastNode() {
		return mNodeTbl[mNodeTbl.size() - 1];
	}

	size_t getTotalBytesOfLVal() {
		return mlValOfsMap.size() * 8;
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
	//			| "sizeof" unary
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

	//トークンを調べて該当するノードタイプを返す
	nodeType GetNodeType(const Token& token) const;

	/// <summary>
	/// 変数宣言を読み込んで、型情報を得る
	/// </summary>
	/// <returns>型情報リンクリストの頭。宣言でないときにはnullptr</returns>
	ValTypeInfo* ReadValueType();

	//!@brief 式の終わり";"かどうか
	bool isEndOfState() {
		assert(mCurrentPos < mTokenTbl.size());
		return mTokenTbl[mCurrentPos].isReserved(';');
	}

	vector<Token> mTokenTbl;
	size_t mCurrentPos;
	const Token& getCurTk() {
		return mTokenTbl[mCurrentPos];
	}

	//現在のトークンが入力と等しければ次に進む
	//そうでなければエラーを出力して停止する
	void expectAndNext(char op) {
		getCurTk().expect(op);
		++mCurrentPos;
	}

	//現在のトークンが入力と等しければ次にすすみtrueを返す
	//そうでなければfalseを返す
	bool nextIfIsReserved(string_view sv) {
		if (getCurTk().isReserved(sv)) {
			++mCurrentPos;
			return true;
		}
		return false;
	}
};
