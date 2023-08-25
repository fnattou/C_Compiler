#include "compiler.h"
#include <fstream>
#include <stdarg.h>

Compiler::Compiler()
	:mResultStr()
	,mTokenTbl()
	,mCurrentPos(0)
{
 
}

void Compiler::Compile(string src, string filename) {
	mSrcStr = src;
	mResultStr.reserve(200);
	mResultStr += (".intel_syntax noprefix\n");
	mResultStr += (".globl main\n");
	mResultStr += ("main:\n");
	Tokenize();
	Parse();
	mResultStr += ("  ret\n");
	OutputFile(filename);
}

void Compiler::OutputFile(string filename) {
	std::ofstream wf;
	filename += ".s";
	wf.open(filename, std::ios::out);
	wf << mResultStr.c_str() << std::endl;
	wf.close();
}

void Compiler::Parse() {
	size_t cnt = 0;
	appendAssemblyLine("mov", "rax", mTokenTbl[cnt++].expectNumber());
	while (cnt < mTokenTbl.size())
	{
		const auto& t = mTokenTbl[cnt++];
		if (t.at_eof()) {
			break;
		}
		if (t.isOperator('+')) {
			appendAssemblyLine("add", "rax", mTokenTbl[cnt++].expectNumber());
		}
		else if (t.isOperator('-')) {
			appendAssemblyLine("sub", "rax", mTokenTbl[cnt++].expectNumber());
		}
	}
}

void Compiler::Tokenize() {
	mTokenTbl.reserve(100);
	for (int i = 0; i < mSrcStr.size(); ++i) {
		const auto c = mSrcStr[i];
		if (isspace(c)) {
			continue;
		}

		if (c == '+' || c == '-') {
			mTokenTbl.emplace_back(Token::TokenType::Reserved, 0, c);
			continue;
		}

		if (isdigit(c)) {
			char* endptr;
			int j = strtol(&mSrcStr.at(i), &endptr, 10);
			while (mSrcStr.data() + i + 1 < endptr) {
				++i;
			}

			mTokenTbl.emplace_back(Token::TokenType::Num, j, c );
			continue;
		}
		error_at(i, "トークナイズできません");
	}
}

void Compiler::appendAssemblyLine(string_view operand, string_view reg, int num) {
	mResultStr += "  ";
	mResultStr += operand;
	mResultStr += " ";
	mResultStr += reg;
	mResultStr += ", ";
	mResultStr += std::to_string(num);
	mResultStr += "\n";
}

void Compiler::error_at(int pos, const char* fmt...) const {

	fprintf(stderr, "%s\n", mSrcStr.c_str());
	fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
	fprintf(stderr, "^ "); //エラーの出た目印を入力

	//エラーメッセージ出力
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}
