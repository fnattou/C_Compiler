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

TEST(CompilerTest, ExecuteFunctionReturnExecutionResult) {
	assemble("test");
	EXPECT_EQ(execute(L"test"), 42);
}

TEST(CompilerTest, Compile_JustZero) {
	COMPILE_AND_TEST("0", "JustZero", 0);
}