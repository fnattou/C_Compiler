#include "compiler.h"
#include <fstream>
#include <stdarg.h>
Compiler::Compiler()
	:mTokenTbl()
	,mCurrentRootNodeIdx(0)
{
 
}

void Compiler::Compile(string_view src, string filename) {
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
	while (mCurrentRootNodeIdx <  mParser.mRootNodeTbl.size()) {
		ReadNodeTree(*mParser.mRootNodeTbl.at(mCurrentRootNodeIdx));

		//���̕]�����ʂƂ��ăX�^�b�N�Ɉ�̒l���c���Ă���͂��Ȃ̂�
		//�X�^�b�N�����Ȃ��悤�Ƀ|�b�v���Ă���
		oss << "  pop rax\n";
		++mCurrentRootNodeIdx;
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
	if (node.type != Type::LocalVal) {
		std::cerr << "����̍��Ӓl���ϐ��ł͂���܂���";
	}

	oss << "  mov rax, rbp\n";
	oss << "  sub rax, " << node.offset <<"\n";	
	oss << "  push  rax\n";
}

void Compiler::ReadNodeTree(Parser::Node& node) {
	using Node = Parser::Node; using Type = Parser::nodeType;

	//����A���䕶�Areturn���Ȃ�
	static int labelNum = 0;
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
	case Type::LocalVal:

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
	case Type::If_: {
		int ifLabelNum = labelNum;
		ReadNodeTree(*node.lhs);
		oss << "  pop rax\n";
		oss << "  cmp rax, 0\n";
		oss << "  je .LElse" << ifLabelNum << "\n";
		ReadNodeTree(*node.middle);
		oss << "  jmp .LEnd" << ifLabelNum << "\n";
		oss << ".LElse" << ifLabelNum << ":\n";
		if (node.rhs) {
			ReadNodeTree(*node.rhs);
		}
		oss << ".LEnd" << ifLabelNum << ":\n";
		++labelNum;
		return;
	}
	case Type::For_: {
		int forLabelNum = labelNum;
		if (node.lhs) {
			ReadNodeTree(*node.lhs);
		}
		oss << ".LBegin" << forLabelNum << ":\n";
		if (node.middle) {
			ReadNodeTree(*node.middle);
		}
		else {
			oss << "  push 1\n";
		}
		oss << "  pop rax\n";
		oss << "  cmp rax, 0\n";
		oss << "  je .LEnd" << forLabelNum << "\n";
		ReadNodeTree(*mParser.mRootNodeTbl.at(++mCurrentRootNodeIdx));
		if (node.rhs) {
			ReadNodeTree(*node.rhs);
			//�]�v�Ɉ��push���Ă���̂ł�����pop���Ă���
			oss << "  pop rax\n";
		}
		oss << "  jmp .LBegin" << forLabelNum << "\n";
		oss << ".LEnd" << forLabelNum << ":\n";
		++labelNum;
		return;
	}
	case Type::While_: {
		int whileLabelNum = labelNum;
		oss << ".LBegin" << whileLabelNum << ":\n";
		ReadNodeTree(*node.lhs);
		oss << "  pop rax\n";
		oss << "  cmp rax, 0\n";
		oss << "  je .LEnd" << whileLabelNum << "\n";
		ReadNodeTree(*node.rhs);
		oss << "  jmp .LBegin" << whileLabelNum << "\n";
		oss << ".LEnd" << whileLabelNum << ":\n";
		++labelNum;
		return;
	}
	case Type::Block: {
		for (auto* innerNode : node.innerBlockNodeTbl) {
			ReadNodeTree(*innerNode);
			oss << "  pop rax\n";
		}
		return;
	}
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

	//�񍀉��Z�q�̏ꍇ
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
	using TokenType = Token::TokenType;
	for (size_t i = 0; i < mSrcStr.size(); ++i) {
		char* ref = &mSrcStr[i];
		const auto c = mSrcStr[i];
		if (isspace(c) || c == '\r' || c == '\n' || c == '\t') {
			continue;
		}

		//���Z�q�܂��͕����̏ꍇ
		if (c == '+' || c == '-' || c == '*' || c == '/'
			|| c == '(' || c == ')'|| c == ';' || c == '{' || c == '}') {
			mTokenTbl.emplace_back(TokenType::Reserved, 0, ref, 1);
			continue;
		}
		//���Z�q�œ񕶎��̉\��������ꍇ
		if (c == '=' || c == '<' || c == '>' || c == '!') {
			int len = 1;
			if (isValidIdx(i + 1) && mSrcStr[i + 1] == '=') {
				++i;
				len = 2;
			}
			mTokenTbl.emplace_back(TokenType::Reserved, 0, ref, len);
			continue;
		}

		//�\��ꂩ���f����֐�
		const auto checkWord = [&](string_view sv) { 
			if (isValidIdx(i + sv.size() - 1) && memcmp(ref, sv.data(), sv.size()) == 0) {
				//�\���̎��������ł���ꍇ�͕ϐ��錾�ɂȂ�B��@returnHoge, if3, forcast
				if (isValidIdx(i + sv.size()) && !isalnum(mSrcStr[i + sv.size()])) {
					return true;
				}
			}
			return false;
		};
		//�\���̏ꍇ�B�ϐ��錾����ɔ��f����
		bool isContinue = false;
		for (string_view str : {"return", "if", "while", "for", "else" }) {
			if (checkWord(str)) {
				const auto type = (str == "return") ? TokenType::Return : TokenType::Reserved;
				mTokenTbl.emplace_back(type, 0,  ref, str.size());
				//���̃��[�v�̐擪�ł܂�++i���s����̂ŁA�����ł�size - 1�𑫂�
				i += str.size() - 1;
				isContinue = true; break;
			}
		}
		if (isContinue) continue;

		//�ϐ��錾�̏ꍇ
		if (isalpha(c)) {
			int len = 1;
			while (isValidIdx(i + 1) && isalpha(mSrcStr[i + 1])) {
				++i; ++len;
			}
			mTokenTbl.emplace_back(TokenType::Ident, 0, ref, len);
			continue;
		}

		//���l�̏ꍇ
		if (isdigit(c)) {
			char* endptr;
			int j = strtol(ref, &endptr, 10);
			while (mSrcStr.data() + i + 1 < endptr) {
				++i;
			}

			mTokenTbl.emplace_back(TokenType::Num, j, ref, 1);
			continue;
		}

		error_at(i, "�g�[�N�i�C�Y�ł��܂���");
	}
}


void Compiler::error_at(size_t pos, const char* fmt...) const {

	fprintf(stderr, "%s\n", mSrcStr.c_str());
	fprintf(stderr, "%*s", static_cast<int>(pos), " "); //pos�̋󔒂��o��
	fprintf(stderr, "^ "); //�G���[�̏o���ڈ�����

	//�G���[���b�Z�[�W�o��
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}
