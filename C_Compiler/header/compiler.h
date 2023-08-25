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
	/// ���͕�������g�[�N���ɕ�������TokenTbl�ɕۑ�����
	/// </summary>
	/// <param name="src">���͕�����</param>
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
