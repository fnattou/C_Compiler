#pragma once
#include <string>

struct Token
{
	enum class TokenType
	{
		Reserved, //記号
		Num,      //整数
		Eof,      //入力の終わり
	};
	TokenType mType; //トークンの種類
	int mVal;        //種類が整数の場合、その数値
	char* mStr;      //トークン文字列
	int mLen;         //トークン文字列の長さ

	Token() = default;

	Token(TokenType type, int num, char* str, int len)
		:mType(type), mVal(num), mStr(str), mLen(len)
	{
	}


	/// <summary>
	/// 次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
	/// それ以外の場合はエラーを報告する
	/// </summary>
	/// <returns></returns>
	int expectNumber() const;

	/// <summary>
	/// 次のトークンが期待している記号のときには、トークンを一つ読み進める
	/// それ以外ならエラーを報告する
	/// </summary>
	/// <param name="op">期待する記号</param>
	bool expect(char op) const;

	bool at_eof() const {
		return mType == Token::TokenType::Eof;
	}

	bool isOperator(char op) const {
		return mType == Token::TokenType::Reserved && mLen == 1 && mStr[0] == op;
	}

	bool isOperator(const char* str) const {
		return mType == Token::TokenType::Reserved && mLen == 2 && !memcmp(mStr, str, mLen);
	}
};

