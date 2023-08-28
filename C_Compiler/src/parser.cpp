#pragma once
#include "parser.h"
Parser::Node* Parser::Expr() {
	Node* node = Mul();
	while (mCurrentPos < mTokenTbl.size()) {
		Token& currentToken = mTokenTbl[mCurrentPos];
		if (currentToken.isOperator('+')) {
			mNodeTbl.push_back(Node{ nodeType::Add, node, Mul() });
			node = &mNodeTbl.at(mNodeTbl.size() - 1);
		}
		else if (currentToken.isOperator('-')) {
			mNodeTbl.push_back(Node{ nodeType::Sub, node, Mul() });
			node = &mNodeTbl.at(mNodeTbl.size() - 1);
		}
		else {
			return node;
		}
		++mCurrentPos;
	}
}
 
Parser::Node* Parser::Mul() {
	Node* node = Primaly();
	while (mCurrentPos < mTokenTbl.size()) {
		Token& currentToken = mTokenTbl[mCurrentPos];
		if (currentToken.isOperator('*')) {
			mNodeTbl.push_back(Node{ nodeType::Mul, node, Primaly() });
			node = &mNodeTbl.at(mNodeTbl.size() - 1);
		}
		else if (currentToken.isOperator('/')) {
			mNodeTbl.push_back(Node{ nodeType::Div, node, Primaly() });
			node = &mNodeTbl.at(mNodeTbl.size() - 1);
		}
		else {
			return node;
		}
		++mCurrentPos;
	}
}

Parser::Node* Parser::Primaly() {
	//次のトークンが"("なら、"(" expr ")"のはず	
	Token& t = mTokenTbl[mCurrentPos];
	if (t.isOperator('(')) {
		Node* node = Expr();
		mTokenTbl[++mCurrentPos].expect(')');
		return node;
	}

	// そうでなければ数値のはず
	mNodeTbl.push_back(Node{ nodeType::Num, nullptr, nullptr, t.expectNumber()});
	return &mNodeTbl.at(mNodeTbl.size() - 1);
}

void Parser::Parse(vector<Token>& tokenTbl) {
	mTokenTbl = tokenTbl;
	mNodeTbl.reserve(tokenTbl.size());
	if (mTokenTbl.size() > 0) {
		Expr();
	}
}

