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
	/// ���͕�������g�[�N���ɕ�������TokenTbl�ɕۑ�����
	/// </summary>
	/// <param name="src">���͕�����</param>
	void Tokenize();

	void error_at(int pos, const char* fmt...) const;

	void ReadNodeTree(Parser::Node& node);

	string mSrcStr;
	std::ostringstream oss;
	std::vector<Token> mTokenTbl;
	size_t mCurrentPos;
	Parser mParser;
};
