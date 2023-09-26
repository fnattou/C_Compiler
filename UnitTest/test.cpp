#include "pch.h"
#include "compiler.h"
#include <Windows.h>
#include <iostream>
#include <string>
namespace {
	using std::string, std::string_view;

	/// <summary>
	/// アセンブリファイルを実行ファイルにコンパイルする
	/// </summary>
	/// <param name="filename">アセンブリファイル名</param>
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
	/// 実行ファイルを起動する。
	/// 起動に失敗した場合はエラーコードを出力する
	/// </summary>
	/// <param name="filename">実行ファイル名</param>
	/// <returns>終了コードを返す</returns>
	int execute(std::wstring_view filename) {
		std::wstring file(L"outfile/"); file += filename; file += L".exe";
		STARTUPINFO si{};
		PROCESS_INFORMATION pi{};

		si.cb = sizeof(si);

		// コマンドラインの実行(成功すると0以外が戻る)
		if (CreateProcess(file.data(), std::wstring(L"").data(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi)) {
			// アプリケーションの終了まで待つ
			WaitForSingleObject(pi.hProcess, INFINITE);

			// アプリケーションの終了コードの取得
			unsigned long exitCode;
			GetExitCodeProcess(pi.hProcess, &exitCode);

			// 終了コードが負の値になる場合もあるので、signedにキャストする
			long ec = static_cast<long>(exitCode);

			// ハンドルを閉じる
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return ec;
		}
		else {
			// 起動失敗
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
		const auto src = 
			"int main() {"
			"return 0;"
			"}";
		COMPILE_AND_TEST(src,  "JustZero", 0);
	}


	TEST(CompilerTest, SomeCalc) {
		const auto src = 
			"int main() {"
			"return 1+1;"
			"}";
		COMPILE_AND_TEST(src,  "SomeCalc", 2);
	}

	TEST(CompilerTest, SomeSpace) {
		const auto src = 
			"int main() {"
			"return 1 + 2;"
			"}";
		COMPILE_AND_TEST(src,  "SomeSpace", 3);
	}

	TEST(CompilerTest, MultipleSpace) {
		const auto src = 
			"int main() {"
			"return 1 + 2 - 1 + 4;"
			"}";
		COMPILE_AND_TEST(src,  "MultipleSpace", 6);
	}
}

namespace mulAndDiv {


	TEST(CompilerTest, Mul) {
		const auto src = 
			"int main() {"
			"return 2 * 2;"
			"}";
		COMPILE_AND_TEST(src,  "Mul", 4);
	}

	TEST(CompilerTest, MulAndPlus) {
		const auto src = 
			"int main() {"
			"return 2 * 2 + 1;"
			"}";
		COMPILE_AND_TEST(src,  "MulAndPlus", 5);
	}

	TEST(CompilerTest, SimpleDiv) {
		const auto src = 
			"int main() {"
			"return 2 / 2;"
			"}";
		COMPILE_AND_TEST(src,  "SimpleDiv", 1);
	}

	TEST(CompilerTest, DivAndMinus) {
		const auto src = 
			"int main() {"
			"return 2 / 2 - 1;"
			"}";
		COMPILE_AND_TEST(src,  "DivAndMinus", 0);
	}

	TEST(CompilerTest, Priority) {
		const auto src = 
			"int main() {"
			"return 2 / 2 - (1 + 1);"
			"}";
		COMPILE_AND_TEST(src,  "Priority", -1);
	}

	TEST(CompilerTest, SinglePlus) {
		const auto src = 
			"int main() {"
			"return 2 * + 2;"
			"}";
		COMPILE_AND_TEST(src,  "SinglePlus", 4);
	}

	TEST(CompilerTest, SingleMinus) {
		const auto src = 
			"int main() {"
			"return 2 * - 2;"
			"}";
		COMPILE_AND_TEST(src,  "SingleMinus", -4);
	}
}

namespace comparison {
	TEST(CompilerTest, Eq) {
		const auto src = 
			"int main() {"
			"return 1 == 1;"
			"}";
		COMPILE_AND_TEST(src,  "Eq", 1);
	}

	TEST(CompilerTest, Ne) {
		const auto src = 
			"int main() {"
			"return 1 != 1;"
			"}";
		COMPILE_AND_TEST(src,  "Ne", 0);
	}
	
	TEST(CompilerTest, Le) {
		const auto src = 
			"int main() {"
			"return 0 <= 1;"
			"}";
		COMPILE_AND_TEST(src,  "Le", 1);
	}
	
	TEST(CompilerTest, Lt) {
		const auto src = 
			"int main() {"
			"return 1 <= 1;"
			"}";
		COMPILE_AND_TEST(src,  "Lt", 1);
	}
	
	TEST(CompilerTest, Ge) {
		const auto src = 
			"int main() {"
			"return 1 >= 1;"
			"}";
		COMPILE_AND_TEST(src,  "Ge", 1);
	}
	
	TEST(CompilerTest, Gt) {
		const auto src = 
			"int main() {"
			"return 1 > 1;"
			"}";
		COMPILE_AND_TEST(src,  "Gt", 0);
	}
}


namespace localVal {
	TEST(CompilerTest, DeclareValueAndAdd) {
		const auto src = 
			"int main() {"
			"int a =  1; a = a + 1;"
			"return a;"
			"}";
		COMPILE_AND_TEST(src,  "DeclareValueAndAdd",2);
	}
}

namespace MultiSentence {
	TEST(CompilerTest, AddToVal) {
		const auto src = 
			"int main() {"
			"int a =  1; a = a + 1;"
			"return a;"
			"}";
		COMPILE_AND_TEST(src,  "AddToVal", 2);
	}

	TEST(CompilerTest, MulTwoVal) {
		const auto src = 
			"int main() {"
			"int a =  5; int b =  2;"
			"return a * b;"
			"}";
		COMPILE_AND_TEST(src,  "MulTwoVal", 10);
	}


	TEST(CompilerTest, CalcWithSomeVal) {
		const auto src = 
			"int main() {"
			"int a =  3; int b =  5 * 6 - 4;"
			"return a + b / 2;"
			"}";
		COMPILE_AND_TEST(src,  "AddToVal" ,16);
	}

	TEST(CompilerTest, LongNameLval) {
		COMPILE_AND_TEST(
			"int main() {int foo = 1;"
			"int bar = 2 + 3;"
			"return foo + bar; }", 
			"LongNameLval" ,6);
	}
}

namespace returnTest {
	TEST(CompilerTest, ReturnAValue) {
		const auto src =
			"int main() {"
			"return 10;"
			"}";
		COMPILE_AND_TEST(src, "ReturnAValue" ,10);
	}

	TEST(CompilerTest, ReturnSomeCalc) {
		const auto src =
			"int main() {"
			"int a =  1;"
			"int b =  2 * 3;"
			"return a + b;"
			"}";
		COMPILE_AND_TEST(src, "ReturnSomeCalc", 7);
	}
}


namespace branchTest {
	TEST(CompilerTest, IfTest) {
		const auto src =
			"int main() {"
			"if ( 0 < 1 )"
			"10;"
			"}";
		COMPILE_AND_TEST(src, "IfTest", 10);
	}

	TEST(CompilerTest, IfAndElse) {
		const auto src =
			"int main() {"
			"if ( 0 > 1 ) 2; "
			"else 10;"
			"}";
		COMPILE_AND_TEST(src, "IfAndElse", 10);
	}

	TEST(CompilerTest, whileTest) {
		const auto src =
			"int main() {"
			"int a =  1;"
			"while ( a < 10 ) a = a + 1; "
			"return a;"
			"}";
		COMPILE_AND_TEST(src, "WhileTest", 10);
	}


	TEST(CompilerTest, ForTest) {
		const auto src =
			"int main() {"
			"int a =  0;"
			"for (int i = 1; i <= 10; i = i + 1 )"
			"a =  a + i;"
			"}";
		COMPILE_AND_TEST(src, "ForTest", 55);
	}

}


namespace blockstate {
	TEST(CompilerTest,  BlockWithFor) {
		const auto src =
			"int main() {"
			"int a =  2;"
			"for (int i = 1; i <= 10; i = i + 1 )"
			"{"
			"	a = a + i;"
			"	a = a - i;"
			"}"
			"return a;"
			"}";
		COMPILE_AND_TEST(src, "BlockWithFor", 2);
	}
}

namespace funcTest {
	TEST(CompilerTest, FuncWithoutArgs) {
		const auto src =
			"int func() {"
			"return 1;}"
			"int main() {"
			"int a =  func();"
			"return a;"
			"}";
		COMPILE_AND_TEST(src, "FuncWithoutArgs", 1);
	}

	TEST(CompilerTest, FuncWithArgs) {
		const auto src =
			"int func(int b) {"
			"return b + 1;}"
			"int main() {"
			"int a =  func(2 + 2);"
			"return a;"
			"}";
		COMPILE_AND_TEST(src, "FuncWithArgs", 5);
	}
}


namespace addresTest {
	TEST(CompilerTest, AddressAndDeref) {
		const auto src =
			"int main() {"
			"int a =  3;"
			"int b =  &a;"
			"return *b;"
			"}";
		COMPILE_AND_TEST(src, "AddressAndDeref", 3);
	}
}
