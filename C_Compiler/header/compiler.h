#pragma once
#include <string>
#include <iostream>
#include <sstream>
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

	/// <summary>
	/// 入力文字列をトークンに分割してTokenTblに保存する
	/// </summary>
	/// <param name="src">入力文字列</param>
	void Tokenize();

	void error_at(int pos, const char* fmt...) const;

	//パース後の抽象構文木を読み込んで、コードを作成する
	void ReadNodeTree(Parser::Node& node);

	//左辺値としてノードを読みこむ
	void ReadLValueNode(Parser::Node& node);

	string mSrcStr;
	std::ostringstream oss;
	std::vector<Token> mTokenTbl;
	size_t mCurrentPos;
	Parser mParser;
};
