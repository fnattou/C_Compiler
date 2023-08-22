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
	mResultStr.reserve(200);
	mResultStr += (".intel_syntax noprefix\n");
	mResultStr += (".globl main\n");
	mResultStr += ("main:\n");
	Tokenize(src);
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

void Compiler::Tokenize(string& src) {
	mTokenTbl.reserve(100);
	for (int i = 0; i < src.size(); ++i) {
		const auto c = src[i];
		if (isspace(c)) {
			continue;
		}

		if (c == '+' || c == '-') {
			mTokenTbl.emplace_back(
				Token::TokenType::Reserved,
				0,
				c
			);
			continue;
		}

		if (isdigit(c)) {
			char* endptr;
			int j = strtol(&src.at(i), &endptr, 10);
			while (src.data() + i + 1 < endptr) {
				++i;
			}

			mTokenTbl.emplace_back(
				Token::TokenType::Num,
				j,
				c
			);
			continue;
		}
		error("トークナイズできません");
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
