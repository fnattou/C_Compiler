#pragma once

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
	char mStr;      //�g�[�N��������

	Token() = default;

	Token(TokenType type, int num, char str)
		:mType(type), mVal(num), mStr(str)
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
		return mType == Token::TokenType::Reserved && mStr == op;
	}
};

