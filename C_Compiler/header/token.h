#pragma once
#include <string>
using std::string_view;

struct Token
{
	enum class TokenType
	{
		Reserved, //記号
		Num,      //整数
		Ident,    //識別子
		Return,   //returen宣言
		Eof,      //入力の終わり
	};
	TokenType mType; //トークンの種類
	int mVal;        //種類が整数の場合、その数値
	string_view mStr;      //トークン文字列

	Token() = default;

	Token(TokenType type, int num, char* str, int len)
		:mType(type), mVal(num), mStr(str, len)
	{
	}


	/// <summary>
	/// 次のトークンが数値の場合, その数を返す。
	/// それ以外の場合はエラーを報告する
	/// </summary>
	/// <returns></returns>
	int expectNumber() const;

	/// <summary>
	/// 次のトークンが期待している記号か返す
	/// それ以外ならエラーを報告する
	/// </summary>
	/// <param name="op">期待する記号</param>
	bool expect(char op) const;

	bool isIdent() const {
		return mType == Token::TokenType::Ident;
	}

	bool at_eof() const {
		return mType == Token::TokenType::Eof;
	}
	
	bool isReturn() const {
		return mType == Token::TokenType::Return;
	}

	bool isOperator(char op) const {
		return mType == Token::TokenType::Reserved && mStr.size() == 1 && mStr[0] == op;
	}

	bool isOperator(const char* str) const {
		return mType == Token::TokenType::Reserved && mStr.size() == 2 && mStr.compare({ str, mStr.size()}) == 0;
	}
};

