#include "compiler.h"
#include <fstream>
#include <stdarg.h>
Compiler::Compiler()
	:mTokenTbl()
	,mCurrentPos(0)
{
 
}

void Compiler::Compile(string src, string filename) {
	//�g�[�N�i�C�Y�A�p�[�X���s���A���ۍ\���؂��쐬����
	mSrcStr = src;
	Tokenize();
	mParser.Parse(mTokenTbl);

	//�A�Z���u���̑O���������o��
	oss << ".intel_syntax noprefix\n";
	oss << ".globl main\n";
	oss << "main:" << std::endl;

	//�v�����[�O : �錾���ꂽ�ϐ����̗̈���m�ۂ���
	oss << "  push rbp\n";
	oss << "  mov rbp, rsp\n";
	oss << "  sub rsp, " << mParser.getTotalBytesOfLVal() << "\n";

	//�擪�̎����珇�ɃR�[�h�𐶐�
	for (auto* rootNode : mParser.mRootNodeTbl) {
		ReadNodeTree(*rootNode);

		//���̕]�����ʂƂ��ăX�^�b�N�Ɉ�̒l���c���Ă���͂��Ȃ̂�
		//�X�^�b�N�����Ȃ��悤�Ƀ|�b�v���Ă���
		oss << "  pop rax\n";
	}

	//�G�s���[�O : �Ō�̎��̌��ʂ�RAX�Ɏc���Ă���̂ł����Ԃ�l�Ƃ���
	oss << "  mov rsp, rbp\n";
	oss << "  pop rbp\n";
	oss << "  ret\n";

	//���ʂ��t�@�C���ɕۑ�
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
		std::cerr << "����̍��Ӓl���ϐ��ł͂���܂���";
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
		if (isspace(c) || c == '\r' || c == '\n') {
			continue;
		}

		if (c == '+' || c == '-' || c == '*' || c == '/'
			|| c == '(' || c == ')') {
			mTokenTbl.emplace_back(Token::TokenType::Reserved, 0, ref, 1);
			continue;
		}

		//�񕶎��̉\��������ꍇ
		if (c == '=' || c == '<' || c == '>' || c == '!' || c == ';') {
			int len = 1;
			if (isValidIdx(i + 1) && mSrcStr[i + 1] == '=') {
				++i;
				len = 2;
			}
			mTokenTbl.emplace_back(Token::TokenType::Reserved, 0, ref, len);
			continue;
		}

		//return�̏ꍇ
		//�����[�vmemcmp���Ă�ƒx�����Ȃ̂ŁA�ŏ��̕��������ŗ\�ߔ��肷��
		if (c == 'r') {
			if (isValidIdx(i + 5) && memcmp(ref, "return", 5) == 0) {
				//returnx�̂悤�ȏꍇ������
				if (isValidIdx(i + 6) && !isalnum(mSrcStr[i + 6])) {
					mTokenTbl.emplace_back(Token::TokenType::Return, 0, ref, 6);
					i += 5;
					continue;
				}
			}
		}

		//�ϐ��錾�̏ꍇ
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
