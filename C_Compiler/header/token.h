#pragma once
#include <string>
using std::string_view;

struct Token
{
	enum class TokenType
	{
		Reserved, //�L��
		Num,      //����
		Ident,    //���ʎq
		Literal,  //�����񃊃e����
		Return,   //retun�錾
		Eof,      //���͂̏I���
	};
	TokenType mType; //�g�[�N���̎��
	int mVal;        //��ނ������̏ꍇ�A���̐��l
	string_view mStr;      //�g�[�N��������

	Token() = default;

	Token(TokenType type, int num, char* str, size_t len)
		:mType(type), mVal(num), mStr(str, len)
	{
	}


	/// <summary>
	/// ���̃g�[�N�������l�̏ꍇ, ���̐���Ԃ��B
	/// ����ȊO�̏ꍇ�̓G���[��񍐂���
	/// </summary>
	/// <returns></returns>
	int expectNumber() const;

	/// <summary>
	/// ���̃g�[�N�������҂��Ă���L�����Ԃ�
	/// ����ȊO�Ȃ�G���[��񍐂���
	/// </summary>
	/// <param name="op">���҂���L��</param>
	bool expect(char op) const;

	bool isIdent() const {
		return mType == Token::TokenType::Ident;
	}

	bool at_eof() const {
		return mType == Token::TokenType::Eof;
	}

	bool isReturn() const {
		return mType == Token::TokenType::Return;
	}

	bool isLiteral() const {
		return mType == Token::TokenType::Literal;
	}

	bool isReserved(char op) const {
		return mType == Token::TokenType::Reserved && mStr.size() == 1 && mStr[0] == op;
	}

	bool isReserved(const string_view sv) const {
		return mType == TokenType::Reserved && mStr.size() == sv.size() && mStr.compare(sv) == 0;
	}
};

