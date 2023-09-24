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
	/// ���͕�������g�[�N���ɕ�������TokenTbl�ɕۑ�����
	/// </summary>
	/// <param name="src">���͕�����</param>
	void Tokenize();

	void error_at(size_t pos, const char* fmt...) const;

	//�p�[�X��̒��ۍ\���؂�ǂݍ���ŁA�R�[�h���쐬����
	struct NodeTblInfo;
	void ReadNodeTree(Parser::Node& node, NodeTblInfo& info);

	//���Ӓl�Ƃ��ăm�[�h��ǂ݂���ŁA�R�[�h�𐶐�����
	void ReadLValueNode(Parser::Node& node);

	//�֐��Ƃ��ăm�[�h��ǂ�ŃR�[�h�𐶐�����
	void ReadFuncNode(Parser::Node& node);

	//i < mSrcStr.size()
	bool isValidIdx(size_t i) {
		return i < mSrcStr.size();
	}

	//�֐��Ɉ�����n���Ƃ��Ɏg�p���郌�W�X�^�z��
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
