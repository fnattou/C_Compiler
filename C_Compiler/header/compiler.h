#pragma once
#include <string>
#include <vector>
using std::string, std::vector, std::string_view;
class Compiler
{
public:
	Compiler();
	~Compiler(){}
	void Compile(string src, string filename);

private:
	void OutputFile(string filename);

	void Parse();

	/// <summary>
	/// ���͕�������g�[�N���ɕ�������TokenTbl�ɕۑ�����
	/// </summary>
	/// <param name="src">���͕�����</param>
	void Tokenize();

	void error_at(int pos, const char* fmt...) const;

	void appendAssemblyLine(
		string_view operand,
		string_view reg,
		int num
		);

	string srcStr;
	string mResultStr;
	class Token;
	std::vector<Token> mTokenTbl;
	size_t mCurrentPos;

	class Token
	{
	public:
		enum class TokenType
		{
			Reserved, //�L��
			Num,      //����
			Eof,      //���͂̏I���
		};
		TokenType mType; //�g�[�N���̎��
		int mVal;        //��ނ������̏ꍇ�A���̐��l
		char mStr;      //�g�[�N��������

		Token(TokenType type, int num, char str)
			:mType(type), mVal(num), mStr(str)
		{
		};

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
};
