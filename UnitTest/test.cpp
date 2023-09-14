#include "pch.h"
#include "compiler.h"
#include <Windows.h>
#include <iostream>
#include <string>
namespace {
	using std::string, std::string_view;

	/// <summary>
	/// �A�Z���u���t�@�C�������s�t�@�C���ɃR���p�C������
	/// </summary>
	/// <param name="filename">�A�Z���u���t�@�C����</param>
	void assemble(string_view filename) {
		{
			string s; s.reserve(40);
			s.append("as -o outfile/");
			s.append(filename); s.append(".o ");
			s.append(filename); s.append(".s");
			system(s.data());
		}
		{
			string s; s.reserve(50);
			s.append("gcc -o outfile/");
			s.append(filename); s.append(" outfile/");
			s.append(filename); s.append(".o");
			system(s.data());
		}
	}

	/// <summary>
	/// ���s�t�@�C�����N������B
	/// �N���Ɏ��s�����ꍇ�̓G���[�R�[�h���o�͂���
	/// </summary>
	/// <param name="filename">���s�t�@�C����</param>
	/// <returns>�I���R�[�h��Ԃ�</returns>
	int execute(std::wstring_view filename) {
		std::wstring file(L"outfile/"); file += filename; file += L".exe";
		STARTUPINFO si{};
		PROCESS_INFORMATION pi{};

		si.cb = sizeof(si);

		// �R�}���h���C���̎��s(���������0�ȊO���߂�)
		if (CreateProcess(file.data(), std::wstring(L"").data(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi)) {
			// �A�v���P�[�V�����̏I���܂ő҂�
			WaitForSingleObject(pi.hProcess, INFINITE);

			// �A�v���P�[�V�����̏I���R�[�h�̎擾
			unsigned long exitCode;
			GetExitCodeProcess(pi.hProcess, &exitCode);

			// �I���R�[�h�����̒l�ɂȂ�ꍇ������̂ŁAsigned�ɃL���X�g����
			long ec = static_cast<long>(exitCode);

			// �n���h�������
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return ec;
		}
		else {
			// �N�����s
			std::wcout << L"Execution of " << filename.data() <<   L".exe failed.Error code : " <<  GetLastError() << std::endl;
			return -1;
		}

	}
}

#define COMPILE_AND_TEST(src, filename, result) \
Compiler c;\
c.Compile(src, filename);\
assemble(filename);\
EXPECT_EQ(execute(L##filename), result);\

namespace makingAsmFile {
	TEST(CompilerTest, ExecuteFunctionReturnExecutionResult) {
		assemble("test");
		EXPECT_EQ(execute(L"test"), 42);
	}
}

namespace addAndSub {
	TEST(CompilerTest, JustZero) {
		COMPILE_AND_TEST("0;", "JustZero", 0);
	}


	TEST(CompilerTest, SomeCalc) {
		COMPILE_AND_TEST("1+1;", "SomeCalc", 2);
	}

	TEST(CompilerTest, SomeSpace) {
		COMPILE_AND_TEST("1 + 2;", "SomeSpace", 3);
	}

	TEST(CompilerTest, MultipleSpace) {
		COMPILE_AND_TEST("1 + 2 - 1 + 4;", "MultipleSpace", 6);
	}
}

namespace mulAndDiv {


	TEST(CompilerTest, Mul) {
		COMPILE_AND_TEST("2 * 2;", "Mul", 4);
	}

	TEST(CompilerTest, MulAndPlus) {
		COMPILE_AND_TEST("2 * 2 + 1;", "MulAndPlus", 5);
	}

	TEST(CompilerTest, SimpleDiv) {
		COMPILE_AND_TEST("2 / 2;", "SimpleDiv", 1);
	}

	TEST(CompilerTest, DivAndMinus) {
		COMPILE_AND_TEST("2 / 2 - 1;", "DivAndMinus", 0);
	}

	TEST(CompilerTest, Priority) {
		COMPILE_AND_TEST("2 / 2 - (1 + 1);", "Priority", -1);
	}

	TEST(CompilerTest, SinglePlus) {
		COMPILE_AND_TEST("2 * + 2;", "SinglePlus", 4);
	}

	TEST(CompilerTest, SingleMinus) {
		COMPILE_AND_TEST("2 * - 2;", "SingleMinus", -4);
	}
}

namespace comparison {
	TEST(CompilerTest, Eq) {
		COMPILE_AND_TEST("1 == 1;", "Eq", 1);
	}

	TEST(CompilerTest, Ne) {
		COMPILE_AND_TEST("1 != 1;", "Ne", 0);
	}
	
	TEST(CompilerTest, Le) {
		COMPILE_AND_TEST("0 <= 1;", "Le", 1);
	}
	
	TEST(CompilerTest, Lt) {
		COMPILE_AND_TEST("1 <= 1;", "Lt", 1);
	}
	
	TEST(CompilerTest, Ge) {
		COMPILE_AND_TEST("1 >= 1;", "Ge", 1);
	}
	
	TEST(CompilerTest, Gt) {
		COMPILE_AND_TEST("1 > 1;", "Gt", 0);
	}
}


namespace localVal {
	TEST(CompilerTest, DeclareValue) {
		COMPILE_AND_TEST("a = 1;", "DeclareValue", 1);
	}
}

namespace MultiSentence {
	TEST(CompilerTest, AddToVal) {
		COMPILE_AND_TEST("a = 1; a + 1;", "AddToVal", 2);
	}

	TEST(CompilerTest, MulTwoVal) {
		COMPILE_AND_TEST("a = 5; b = 2;a * b;", "MulTwoVal", 10);
	}


	TEST(CompilerTest, CalcWithSomeVal) {
		COMPILE_AND_TEST("a = 3; b = 5 * 6 - 4; a + b / 2;", "AddToVal" ,16);
	}
}
