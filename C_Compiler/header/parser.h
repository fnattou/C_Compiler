#pragma once
#include <vector>
#include <cassert>
#include <unordered_map>
#include <set>
#include "token.h"
using std::vector;
using std::string_view;
using std::unordered_map;

//���͂��ꂽ�g�[�N�������͂��Ē��ۍ\���؂��쐬����N���X
class Parser {
public:
	enum class nodeType {
		Expr,
		Assign,			//���
		If_,
		For_,
		While_,
		Block,			//����
		CallFunc,		//�֐��Ăяo��
		DeclareFunc,	//�֐��錾
		Add,
		Sub,
		Mul,
		Div,
		Eq,
		Ne,
		Lt,
		Le,
		Gt,
		Ge,
		Num,
		LocalVal,		//���[�J���ϐ�
		Addr,			//�A�h���X���Z�q
		Deref,			//�A�h���X�O��
		Sizeof,			//sizeof���Z�q
		Return,
		None,
	};
	struct FuncInfo;
	struct ValTypeInfo;
	struct Node {
		nodeType type;
		Node* lhs;
		Node* rhs;
		Node* middle; //type��for�̎������g��
		int val; //type��Num�̂Ƃ������g��
		int offset; //type��LocalVal�̎������g��
		vector<Node*> innerBlockNodeTbl; // type��Block�̎������g��
		vector<Node*> argumentNodeTbl; // type��CallFunc�̎������g�����������
		FuncInfo* funcInfoPtr; //type��Func�̂Ƃ������g��
		const ValTypeInfo* valTypeInfoPtr; //�ϐ��̎��̌^���
	};

	struct FuncInfo {
		string_view name;
		unordered_map<string_view, int> lValOfsMap;
		unordered_map<string_view, const ValTypeInfo*> lValTypeMap;
		vector<Node*> argumentNodeTbl; //���������
		ValTypeInfo* returnValTypeInfoPtr; //�Ԃ�l�̌^���
		size_t totalBytes; //���[�J���ϐ��̃T�C�Y���v

		//�֐����̃��[�J���ϐ�����o�^����B�������������Ɋ܂܂��
		size_t pushBackToValMap(string_view name, size_t byteSize, const ValTypeInfo* ptr) {
			lValTypeMap.emplace(name, ptr);
			totalBytes += byteSize;
			lValOfsMap.emplace(name, totalBytes);
			return totalBytes;
		}
	};

	struct ValTypeInfo {
		enum class ValType {
			Int,
			Ptr,
		} type;
		ValTypeInfo* toPtr;
		int getByteSize() const {
			return (type == ValType::Int) ? 4 : 8;
		}

		//�|�C���^�������^�̃T�C�Y���擾
		int getToTypeSize() const {
			if (!toPtr) return 0;
			return toPtr->getByteSize();
		}
	};

	// ���[�J���ϐ��̃X�^�b�N����̃I�t�Z�b�g��ۑ����邽�߂̌^
	unordered_map<string_view, int> mlValOfsMap;
	unordered_map<string_view, FuncInfo> mFuncInfoTbl;
	//�ϐ��̌^���̕ۑ��ꏊ
	vector<ValTypeInfo> mTypeInfoTbl;

	FuncInfo* mCurrentFuncInfoPtr;
	
	vector<Node> mNodeTbl;
	vector<Node*> mRootNodeTbl;


	Node& getLastNode() {
		return mNodeTbl[mNodeTbl.size() - 1];
	}

	size_t getTotalBytesOfLVal() {
		return mlValOfsMap.size() * 8;
	}


	Parser()
		:mNodeTbl()
		, mCurrentPos()
		, mTokenTbl()
	{}

	void Parse(vector<Token>& tokenTbl);
private:
	//------------------------------------------
	//���ۍ\���؂̐������@
	//------------------------------------------
	//program	 = statement*
	//			| func statement* 
	//function   = ident "(" (ident ",")* ident? ")" "{" statement* "}"
	//statement	 = expr ";"
	//			| "if" "(" expr ")" statement ( "else" stmt )?
	//			| "while" "(" expr ")" statement
	//			| "for" "(" expr? ";" expr? ";" expr? ")" statement
	//			| "return" expr ";"
	//			| "{" statement* "}"


	//expr		 = assign
	//assign	 = equality ( "=" assign)?
	//equality   = relational ("==" relational | "!=" relational)*
	//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
	//add        = mul ("+" mul | "-" mul)*
	//mul        = unary ("*" unary | "/" unary)*
	//unary      = ("+" | "-")? primary
	//			| "sizeof" unary
	//		    | "*" unary
	//			| "&" unary
	//primary    = num 
	//			| ident ( "(" (ident "," )* ident? ")"�@)?
	//			| "(" expr ")"

	void Program();
	Node* Function();
	Node* Statement();
	Node* Expr();
	Node* Assign();
	Node* Equality();
	Node* Relational();
	Node* Add();
	Node* Mul();
	Node* Unary();
	Node* Primaly();

	//�g�[�N���𒲂ׂĊY������m�[�h�^�C�v��Ԃ�
	nodeType GetNodeType(const Token& token) const;

	/// <summary>
	/// �ϐ��錾��ǂݍ���ŁA�^���𓾂�
	/// </summary>
	/// <returns>�^��񃊃��N���X�g�̓��B�錾�łȂ��Ƃ��ɂ�nullptr</returns>
	ValTypeInfo* ReadValueType();

	//!@brief ���̏I���";"���ǂ���
	bool isEndOfState() {
		assert(mCurrentPos < mTokenTbl.size());
		return mTokenTbl[mCurrentPos].isReserved(';');
	}

	vector<Token> mTokenTbl;
	size_t mCurrentPos;
	const Token& getCurTk() {
		return mTokenTbl[mCurrentPos];
	}

	//���݂̃g�[�N�������͂Ɠ�������Ύ��ɐi��
	//�����łȂ���΃G���[���o�͂��Ē�~����
	void expectAndNext(char op) {
		getCurTk().expect(op);
		++mCurrentPos;
	}

	//���݂̃g�[�N�������͂Ɠ�������Ύ��ɂ�����true��Ԃ�
	//�����łȂ����false��Ԃ�
	bool nextIfIsReserved(string_view sv) {
		if (getCurTk().isReserved(sv)) {
			++mCurrentPos;
			return true;
		}
		return false;
	}
};
