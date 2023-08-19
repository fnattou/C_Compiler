#pragma once
#include <string>
using std::string;
class Compiler
{
public:
	Compiler(){}
	~Compiler(){}
	void Compile(string src, string filename);
private:
	void OutputFile(string src, string filename);
};
