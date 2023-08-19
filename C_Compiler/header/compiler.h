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
	void OutputFile(string filename);
	void Parse(string& src);
	string resultStr;
};
