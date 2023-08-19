#include "compiler.h"
#include <fstream>
void Compiler::Compile(string src, string filename) {
	resultStr.reserve(200);
	resultStr += (".intel_syntax noprefix\n");
	resultStr += (".globl main\n");
	resultStr += ("main:\n");
	Parse(src);
	resultStr += ("  ret\n");
	OutputFile(filename);
}


void Compiler::OutputFile(string filename) {
	std::ofstream wf;
	filename += ".s";
	wf.open(filename, std::ios::out);
	wf << resultStr.c_str() << std::endl;
	wf.close();
}

void Compiler::Parse(string& src) {
	size_t pos = 0;
	resultStr += "  mov rax, "; resultStr += src[pos++]; resultStr += "\n";
	while (pos < src.size()) {
		if (src[pos++] == '+') {
			resultStr += "  add rax, ";
			while ('0' <= src[pos] && src[pos] <= '9') {
				resultStr += src[pos++];
			}
			resultStr += "\n";
		}
		else if (src[pos++] == '-') {
			resultStr += "  sub rax, ";
			while ('0' <= src[pos] && src[pos] <= '9') {
				resultStr += src[pos++];
			}
			resultStr += "\n";
		}
	}
}
