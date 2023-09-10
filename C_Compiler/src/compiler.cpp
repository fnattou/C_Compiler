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

	//�t�̃m�[�h�̏ꍇ
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
	case Type::Eq:
	case Type::Ne:
	case Type::Le:
	case Type::Lt:
	case Type::Gt:
	case Type::Ge:
		if (node.type == Type::Ge || node.type == Type::Gt) {
			oss << "  cmp rdi, rax\n";
		}
		else {
			oss << "  cmp rax, rdi\n";
		}
		switch (node.type) {
			case Type::Eq:
				oss << "  sete al\n";
				break;
			case Type::Ne:
				oss << "  setne al\n";
				break;
			case Type::Le:
			case Type::Ge:
				oss << "  setle al\n";
				break;
			case Type::Lt:
			case Type::Gt:
				oss << "  setl al\n";
				break;
		}
		oss << "  movzb rax, al\n";
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

		//�񕶎��̉\��������ꍇ
		if (c == '=' || c == '<' || c == '>' || c == '!') {
			++i;
			int len = 1;
			if (i < mSrcStr.size() && mSrcStr[i] == '=') {
				len = 2;
			}
			mTokenTbl.emplace_back(Token::TokenType::Reserved, 0, ref, len);
			continue;
		}

		//�ϐ��錾�̏ꍇ
		if ('a' <= c && c <= 'z') {
			mTokenTbl.emplace_back(Token::TokenType::Ident, 0, ref, 1);
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
		error_at(i, "�g�[�N�i�C�Y�ł��܂���");
	}
}


void Compiler::error_at(int pos, const char* fmt...) const {

	fprintf(stderr, "%s\n", mSrcStr.c_str());
	fprintf(stderr, "%*s", pos, " "); //pos�̋󔒂��o��
	fprintf(stderr, "^ "); //�G���[�̏o���ڈ�����

	//�G���[���b�Z�[�W�o��
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}
