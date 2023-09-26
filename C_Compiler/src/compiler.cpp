#include "compiler.h"
#include <fstream>
#include <stdarg.h>
Compiler::Compiler()
	:mTokenTbl()
{
 
}

void Compiler::Compile(string_view src, string filename) {
	//トークナイズ、パースを行い、抽象構文木を作成する
	mSrcStr = src;
	Tokenize();
	mParser.Parse(mTokenTbl);

	//アセンブリの前半部分を出力
	oss << ".intel_syntax noprefix\n";
	oss << ".globl main\n";

	//先頭の式から順にコードを生成
	NodeTblInfo info{ mParser.mRootNodeTbl, 0 };
	while (!info.isEndOfTbl()) {
		ReadNodeTree(info.getCurNode(), info);
		++info.currentNodeTblIdx;
	}

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
	if (node.type != Type::LocalVal) {
		std::cerr << "代入の左辺値が変数ではありません";
	}

	oss << "  mov rax, rbp\n";
	oss << "  sub rax, " << node.offset <<"\n";	
	oss << "  push  rax\n";
}

void Compiler::ReadFuncNode(Parser::Node& node) {
	assert(node.type == Parser::nodeType::DeclareFunc);
	auto* infoPtr = node.funcInfoPtr;

	//関数名をラベルとして記述
	oss <<  infoPtr->name << ":\n";

	//プロローグ : 宣言された変数分の
	// 領域を確保する（実引数もここに入っているはず)
	oss << "  push rbp\n";
	oss << "  mov rbp, rsp\n";
	oss << "  sub rsp, " << infoPtr->lValMap.size() * 8 << "\n";

	//プロローグ２ : 引数が格納されているレジスタから実引数用の変数に移す
	for (size_t i = 0; i < infoPtr->argumentNodeTbl.size(); ++i) {
		auto reg = argRegisterTbl[i];
		auto ofs = infoPtr->argumentNodeTbl[i]->offset;
		oss << "  mov [rbp -  " << ofs << "], " << reg << "\n";	
	}

	//先頭の式から順にコードを生成
	NodeTblInfo info{ node.innerBlockNodeTbl, 0 };
	while (!info.isEndOfTbl()) {
		ReadNodeTree(info.getCurNode(), info);
		info.currentNodeTblIdx++;
		//式の評価結果としてスタックに一つの値が残っているはずなので
		//スタックが溢れないようにポップしておく
		oss << "  pop rax\n";
	}

	//エピローグ : 最後の式の結果がRAXに残っているのでそれを返り値とする
	oss << "  mov rsp, rbp\n";
	oss << "  pop rbp\n";
	oss << "  ret\n";


}

void Compiler::ReadNodeTree(Parser::Node& node, NodeTblInfo& info) {
	using Node = Parser::Node; using Type = Parser::nodeType;

	//代入、制御文、return文など
	static int labelNum = 0;
	switch (node.type) 
	{
	case Type::Num:
		oss << "  push " << node.val << "\n";
		return;
	case Type::Assign:
		ReadLValueNode(*node.lhs);
		ReadNodeTree(*node.rhs, info);
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
		ReadNodeTree(*node.lhs, info);
		oss << "  pop rax\n";
		oss << "  mov rsp, rbp\n";
		oss << "  pop rbp\n";
		oss << "  ret\n";
		return;
	case Type::If_: {
		int ifLabelNum = labelNum;
		ReadNodeTree(*node.lhs, info);
		oss << "  pop rax\n";
		oss << "  cmp rax, 0\n";
		oss << "  je .LElse" << ifLabelNum << "\n";
		ReadNodeTree(*node.middle, info);
		oss << "  jmp .LEnd" << ifLabelNum << "\n";
		oss << ".LElse" << ifLabelNum << ":\n";
		if (node.rhs) {
			ReadNodeTree(*node.rhs, info);
		}
		oss << ".LEnd" << ifLabelNum << ":\n";
		++labelNum;
		return;
	}
	case Type::For_: {
		int forLabelNum = labelNum;
		if (node.lhs) {
			ReadNodeTree(*node.lhs, info);
		}
		oss << ".LBegin" << forLabelNum << ":\n";
		if (node.middle) {
			ReadNodeTree(*node.middle, info);
		}
		else {
			oss << "  push 1\n";
		}
		oss << "  pop rax\n";
		oss << "  cmp rax, 0\n";
		oss << "  je .LEnd" << forLabelNum << "\n";
		ReadNodeTree(*info.nodeTbl[++info.currentNodeTblIdx], info);
		if (node.rhs) {
			ReadNodeTree(*node.rhs, info);
			//余計に一回pushしているのでここでpopしておく
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
		ReadNodeTree(*node.lhs, info);
		oss << "  pop rax\n";
		oss << "  cmp rax, 0\n";
		oss << "  je .LEnd" << whileLabelNum << "\n";
		ReadNodeTree(*node.rhs, info);
		oss << "  jmp .LBegin" << whileLabelNum << "\n";
		oss << ".LEnd" << whileLabelNum << ":\n";
		++labelNum;
		return;
	}
	case Type::Block: {
		for (auto* innerNode : node.innerBlockNodeTbl) {
			ReadNodeTree(*innerNode, info);
			oss << "  pop rax\n";
		}
		return;
	}
	case Type::CallFunc: {
		assert( node.argumentNodeTbl.size() <= 6);
		for (const auto n : node.argumentNodeTbl) {
			ReadNodeTree(*n, info);
		}
		for (size_t i = node.argumentNodeTbl.size(); i >= 1 ; --i) {
			oss << "  pop " << argRegisterTbl[i - 1] << "\n";
		}
		oss << "  call " << node.funcInfoPtr->name << "\n";
		oss << "  push rax\n";
		return;
	}

	case Type::DeclareFunc: 
		ReadFuncNode(node);
		return;	
	case Type::Addr:
		ReadLValueNode(*node.rhs);
		return;
	case Type::Deref:
		ReadNodeTree(*node.rhs, info);
		oss << "  pop rax\n";
		oss << "  mov rax, [rax]\n";
		oss << "  push rax\n";
		return;
	default:
		break;
	}


	//葉のノードの場合
	if (!node.lhs && !node.rhs) {
		return;
	}

	ReadNodeTree(*node.lhs, info);
	ReadNodeTree(*node.rhs, info);

	oss << "  pop rdi\n";
	oss << "  pop rax\n";

	//演算子の場合
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

		//演算子、括弧、文末などの特殊文字
		if (c == '+' || c == '-' || c == '*' || c == '/'
			|| c == '(' || c == ')'|| c == '{' || c == '}'
			|| c == ';' || c == '&') {
			mTokenTbl.emplace_back(TokenType::Reserved, 0, ref, 1);
			continue;
		}
		//演算子で二文字の可能性がある場合
		if (c == '=' || c == '<' || c == '>' || c == '!') {
			int len = 1;
			if (isValidIdx(i + 1) && mSrcStr[i + 1] == '=') {
				++i;
				len = 2;
			}
			mTokenTbl.emplace_back(TokenType::Reserved, 0, ref, len);
			continue;
		}

		//予約語か判断する関数
		const auto checkWord = [&](string_view sv) { 
			if (isValidIdx(i + sv.size() - 1) && memcmp(ref, sv.data(), sv.size()) == 0) {
				//予約語の次が文字である場合は変数宣言になる。例　returnHoge, if3, forcast
				if (isValidIdx(i + sv.size()) && !isalnum(mSrcStr[i + sv.size()])) {
					return true;
				}
			}
			return false;
		};
		//予約語の場合。変数宣言より先に判断する
		bool isContinue = false;
		for (string_view str : {"return", "if", "while", "for", "else" }) {
			if (checkWord(str)) {
				const auto type = (str == "return") ? TokenType::Return : TokenType::Reserved;
				mTokenTbl.emplace_back(type, 0,  ref, str.size());
				//次のループの先頭でまた++iが行われるので、ここではsize - 1を足す
				i += str.size() - 1;
				isContinue = true; break;
			}
		}
		if (isContinue) continue;

		//変数宣言の場合
		if (isalpha(c)) {
			int len = 1;
			while (isValidIdx(i + 1) && isalpha(mSrcStr[i + 1])) {
				++i; ++len;
			}
			mTokenTbl.emplace_back(TokenType::Ident, 0, ref, len);
			continue;
		}

		//数値の場合
		if (isdigit(c)) {
			char* endptr;
			int j = strtol(ref, &endptr, 10);
			while (mSrcStr.data() + i + 1 < endptr) {
				++i;
			}

			mTokenTbl.emplace_back(TokenType::Num, j, ref, 1);
			continue;
		}

		error_at(i, "トークナイズできません");
	}
}


void Compiler::error_at(size_t pos, const char* fmt...) const {

	fprintf(stderr, "%s\n", mSrcStr.c_str());
	fprintf(stderr, "%*s", static_cast<int>(pos), " "); //pos個の空白を出力
	fprintf(stderr, "^ "); //エラーの出た目印を入力

	//エラーメッセージ出力
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}
