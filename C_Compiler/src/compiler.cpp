#include "compiler.h"
#include <fstream>
#include <stdarg.h>
Compiler::Compiler()
	:mTokenTbl()
	,mCurrentPos(0)
{
 
}

void Compiler::Compile(string src, string filename) {
	//トークナイズ、パースを行い、抽象構文木を作成する
	mSrcStr = src;
	Tokenize();
	mParser.Parse(mTokenTbl);

	//アセンブリの前半部分を出力
	oss << ".intel_syntax noprefix\n";
	oss << ".globl main\n";
	oss << "main:" << std::endl;

	//プロローグ : 宣言された変数分の領域を確保する
	oss << "  push rbp\n";
	oss << "  mov rbp, rsp\n";
	oss << "  sub rsp, " << mParser.getTotalBytesOfLVal() << "\n";

	//先頭の式から順にコードを生成
	for (auto* rootNode : mParser.mRootNodeTbl) {
		ReadNodeTree(*rootNode);

		//式の評価結果としてスタックに一つの値が残っているはずなので
		//スタックが溢れないようにポップしておく
		oss << "  pop rax\n";
	}

	//エピローグ : 最後の式の結果がRAXに残っているのでそれを返り値とする
	oss << "  mov rsp, rbp\n";
	oss << "  pop rbp\n";
	oss << "  ret\n";

	//結果をファイルに保存
	OutputFile(filename);
}

void Compiler::OutputFile(string filename) {
	std::ofstream wf;
	filename += ".s";
	wf.open(filename, std::ios::out);
	wf << oss.str() << std::endl;
	wf.close();
}

void Compiler::ReadLValueNode(Parser::Node& node) {
	using Node = Parser::Node; using Type = Parser::nodeType;
	if (node.type != Type::LovalVal) {
		std::cerr << "代入の左辺値が変数ではありません";
	}

	oss << "  mov rax, rbp\n";
	oss << "  sub rax, " << node.offset <<"\n";	
	oss << "  push  rax\n";
}

void Compiler::ReadNodeTree(Parser::Node& node) {
	using Node = Parser::Node; using Type = Parser::nodeType;
	switch (node.type) 
	{
	case Type::Num:
		oss << "  push " << node.val << "\n";
		return;
	case Type::Assign:
		ReadLValueNode(*node.lhs);
		ReadNodeTree(*node.rhs);
		oss << "  pop rdi\n";
		oss << "  pop rax\n";
		oss << "  mov [rax], rdi\n";
		oss << "  push rdi\n";
		return;
	case Type::LovalVal:

		ReadLValueNode(node);
		oss << "  pop rax\n";
		oss << "  mov rax, [rax]\n";
		oss << "  push rax\n";
		return;
	case Type::Return:
		ReadNodeTree(*node.lhs);
		oss << "  pop rax\n";
		oss << "  mov rsp, rbp\n";
		oss << "  pop rbp\n";
		oss << "  ret\n";
		return;
	default:
		break;
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
		if (isspace(c) || c == '\r' || c == '\n') {
			continue;
		}

		if (c == '+' || c == '-' || c == '*' || c == '/'
			|| c == '(' || c == ')') {
			mTokenTbl.emplace_back(Token::TokenType::Reserved, 0, ref, 1);
			continue;
		}

		//二文字の可能性がある場合
		if (c == '=' || c == '<' || c == '>' || c == '!' || c == ';') {
			int len = 1;
			if (isValidIdx(i + 1) && mSrcStr[i + 1] == '=') {
				++i;
				len = 2;
			}
			mTokenTbl.emplace_back(Token::TokenType::Reserved, 0, ref, len);
			continue;
		}

		//returnの場合
		//毎ループmemcmpしてると遅そうなので、最初の文字だけで予め判定する
		if (c == 'r') {
			if (isValidIdx(i + 5) && memcmp(ref, "return", 5) == 0) {
				//returnxのような場合を除く
				if (isValidIdx(i + 6) && !isalnum(mSrcStr[i + 6])) {
					mTokenTbl.emplace_back(Token::TokenType::Return, 0, ref, 6);
					i += 5;
					continue;
				}
			}
		}

		//変数宣言の場合
		if (isalpha(c)) {
			int len = 1;
			while (isValidIdx(i + 1) && isalpha(mSrcStr[i + 1])) {
				++i; ++len;
			}
			mTokenTbl.emplace_back(Token::TokenType::Ident, 0, ref, len);
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
