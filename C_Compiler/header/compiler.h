#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "parser.h"

using std::string, std::vector, std::string_view;
class Compiler
{
public:
	Compiler();
	~Compiler(){}
	void Compile(string_view src, string filename);

private:
	void OutputFile(string filename);

	/// <summary>
	/// 入力文字列をトークンに分割してTokenTblに保存する
	/// </summary>
	/// <param name="src">入力文字列</param>
	void Tokenize();

	void error_at(size_t pos, const char* fmt...) const;

	//パース後の抽象構文木を読み込んで、コードを作成する
	struct NodeTblInfo;
	void ReadNodeTree(Parser::Node& node, NodeTblInfo& info);

	//左辺値としてノードを読みこんで、コードを生成する
	void ReadLValueNode(Parser::Node& node);

	//関数としてノードを読んでコードを生成する
	void ReadFuncNode(Parser::Node& node);

	//i < mSrcStr.size()
	bool isValidIdx(size_t i) {
		return i < mSrcStr.size();
	}

	//関数に引数を渡すときに使用するレジスタ配列
	static constexpr const char* argRegisterTbl[] = {"rdi", "rsi" ,"rdx", "rcx", "r8", "r9"};
	string mSrcStr;
	std::ostringstream oss;
	std::vector<Token> mTokenTbl;
	Parser mParser;
	struct NodeTblInfo {
		std::vector<Parser::Node*>& nodeTbl;
		size_t currentNodeTblIdx;
		bool isEndOfTbl() {
			return currentNodeTblIdx == nodeTbl.size();
		}
		Parser::Node& getCurNode() {
			return *nodeTbl[currentNodeTblIdx];
		}
	};
};
