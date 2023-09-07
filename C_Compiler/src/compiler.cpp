#include "compiler.h"
#include <fstream>
#include <stdarg.h>
Compiler::Compiler()
	:mTokenTbl()
	,mCurrentPos(0)
{
 
}

void Compiler::Compile(string src, string filename) {
	mSrcStr = src;
	oss << ".intel_syntax noprefix\n";
	oss << ".globl main\n";
	oss << "main:" << std::endl;
	Tokenize();
	mParser.Parse(mTokenTbl);
	ReadNodeTree(mParser.getLastNode());
	oss << "  pop rax\n";
	oss << "  ret\n";
	OutputFile(filename);
}

void Compiler::OutputFile(string filename) {
	std::ofstream wf;
	filename += ".s";
	wf.open(filename, std::ios::out);
	wf << oss.str() << std::endl;
	wf.close();
}

void Compiler::ReadNodeTree(Parser::Node& node) {
	using Node = Parser::Node; using Type = Parser::nodeType;
	if (node.type == Type::Num) {
		oss << "  push " << node.val << "\n";
	}

	//葉のノードの場合
	if (!node.lhs && !node.rhs) {
		return;
	}

	ReadNodeTree(*node.lhs);
	ReadNodeTree(*node.rhs);

	oss << "  pop rdi\n";
	oss << "  pop rax\n";

	switch (node.type) {
	case Type::Add:
		oss << "  add rax, rdi\n";
		break;
	case Type::Sub:	
		oss << "  sub rax, rdi\n";
		break;
	case Type::Mul:
		oss << "  imul rax, rdi\n";
		break;
	case Type::Div:
		oss << "  cqo\n";
		oss << "  idiv rdi\n";
		break;
	}

	oss << "  push rax" << std::endl;
}

void Compiler::Tokenize() {
	mTokenTbl.reserve(100);
	for (int i = 0; i < mSrcStr.size(); ++i) {
		char* ref = &mSrcStr[i];
		const auto c = mSrcStr[i];
		if (isspace(c)) {
			continue;
		}

		if (c == '+' || c == '-' || c == '*' || c == '/'
			|| c == '(' || c == ')') {
			mTokenTbl.emplace_back(Token::TokenType::Reserved, 0, ref, 1);
			continue;
		}

		//二文字の可能性がある場合
		if (c == '=' || c == '<' || c == '>' || c == '!') {
			++i;
			int len = 1;
			if (i < mSrcStr.size() && mSrcStr[i] == '=') {
				len = 2;
			}
			mTokenTbl.emplace_back(Token::TokenType::Reserved, 0, ref, len);
			continue;
		}

		if (isdigit(c)) {
			char* endptr;
			int j = strtol(ref, &endptr, 10);
			while (mSrcStr.data() + i + 1 < endptr) {
				++i;
			}

			mTokenTbl.emplace_back(Token::TokenType::Num, j, ref, 1);
			continue;
		}
		error_at(i, "トークナイズできません");
	}
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
