#pragma once
#include <string>

struct Token
{
	enum class TokenType
	{
		Reserved, //�L��
		Num,      //����
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
	/// ���̃g�[�N�������l�̏ꍇ�A�g�[�N�����P�ǂݐi�߂Ă��̐��l��Ԃ��B
	/// ����ȊO�̏ꍇ�̓G���[��񍐂���
	/// </summary>
	/// <returns></returns>
	int expectNumber() const;

	/// <summary>
	/// ���̃g�[�N�������҂��Ă���L���̂Ƃ��ɂ́A�g�[�N������ǂݐi�߂�
	/// ����ȊO�Ȃ�G���[��񍐂���
	/// </summary>
	/// <param name="op">���҂���L��</param>
	bool expect(char op) const;

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

