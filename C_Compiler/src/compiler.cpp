#include "compiler.h"
#include <fstream>
void Compiler::Compile(string src, string filename)
{
	string res; res.reserve(200);
	res += (".intel_syntax noprefix\n");
	res += (".globl main\n");
	res += ("main:\n");
	res += ("  mov rax, "); res += (src); res += ("\n");
	res += ("  ret\n");
	OutputFile(res, filename);
}


void Compiler::OutputFile(string src, string filename)
{
	std::ofstream wf;
	filename += ".s";
	wf.open(filename, std::ios::out);
	wf << src.c_str() << std::endl;
	wf.close();
}
