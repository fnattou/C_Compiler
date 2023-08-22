#pragma once
#include <string>
#include <vector>
using std::string, std::vector, std::string_view;
class Compiler
{
public:
	Compiler();
	~Compiler(){}
	void Compile(string src, string filename);

private:
	void OutputFile(string filename);

	void Parse();

	/// <summary>
	/// 入力文字列をトークンに分割してTokenTblに保存する
	/// </summary>
	/// <param name="src">入力文字列</param>
	void Tokenize();

	void error_at(int pos, const char* fmt...) const;

	void appendAssemblyLine(
		string_view operand,
		string_view reg,
		int num
		);

	string srcStr;
	string mResultStr;
	class Token;
	std::vector<Token> mTokenTbl;
	size_t mCurrentPos;

	class Token
	{
	public:
		enum class TokenType
		{
			Reserved, //記号
			Num,      //整数
			Eof,      //入力の終わり
		};
		TokenType mType; //トークンの種類
		int mVal;        //種類が整数の場合、その数値
		char mStr;      //トークン文字列

		Token(TokenType type, int num, char str)
			:mType(type), mVal(num), mStr(str)
		{
		};

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
			return mType == Token::TokenType::Reserved && mStr == op;
		}
	};
};
