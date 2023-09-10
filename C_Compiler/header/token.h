#pragma once
#include <string>

struct Token
{
	enum class TokenType
	{
		Reserved, //�L��
		Num,      //����
		Ident,    //���ʎq
		Eof,      //���͂̏I���
	};
	TokenType mType; //�g�[�N���̎��
	int mVal;        //��ނ������̏ꍇ�A���̐��l
	char* mStr;      //�g�[�N��������
	int mLen;         //�g�[�N��������̒���

	Token() = default;

	Token(TokenType type, int num, char* str, int len)
		:mType(type), mVal(num), mStr(str), mLen(len)
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

	bool isOperator(char op) const {
		return mType == Token::TokenType::Reserved && mLen == 1 && mStr[0] == op;
	}

	bool isOperator(const char* str) const {
		return mType == Token::TokenType::Reserved && mLen == 2 && !memcmp(mStr, str, mLen);
	}
};

