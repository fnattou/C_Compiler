#include "compiler.h"
#include <fstream>
#include <stdarg.h>
namespace {
	void error(const char* fmt...) {
		va_list ap;
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
		exit(1);
	}
}

Compiler::Compiler()
	:mResultStr()
	,mTokenTbl()
	,mCurrentPos(0)
{
 
}

void Compiler::Compile(string src, string filename) {
	srcStr = src;
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
	for (int i = 0; i < srcStr.size(); ++i) {
		const auto c = srcStr[i];
		if (isspace(c)) {
			continue;
		}

		if (c == '+' || c == '-') {
			mTokenTbl.emplace_back(Token::TokenType::Reserved, 0, c);
			continue;
		}

		if (isdigit(c)) {
			char* endptr;
			int j = strtol(&srcStr.at(i), &endptr, 10);
			while (srcStr.data() + i + 1 < endptr) {
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

	fprintf(stderr, "%s\n", srcStr.c_str());
	fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
	fprintf(stderr, "^ "); //エラーの出た目印を入力

	//エラーメッセージ出力
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

int Compiler::Token::expectNumber() const {
	if (mType != Token::TokenType::Num) {
		error("数ではありません");
	}
	return mVal;
}

bool Compiler::Token::expect(char op) const{
	if (mType != Token::TokenType::Reserved || mStr != op) {
		error("'%c'ではありません");
	}
	return true;
}
