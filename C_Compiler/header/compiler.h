#pragma once
#include <string>
#include <vector>
#include "parser.h"

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

	string mSrcStr;
	string mResultStr;
	std::vector<Token> mTokenTbl;
	size_t mCurrentPos;
	Parser mParser;
};
