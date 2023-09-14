#pragma once
#include "parser.h"

Parser::Node* Parser::PushBackNode(Node n) {
	mNodeTbl.push_back(n);
	return &mNodeTbl.at(mNodeTbl.size() - 1);
}

Parser::nodeType Parser::GetNodeType(Token& token) {
	if (token.isOperator('+')) {
		return nodeType::Add;
	}
	else if (token.isOperator('-')) {
		return nodeType::Sub;
	}
	else if (token.isOperator('*')) {
		return nodeType::Mul;
	}
	else if (token.isOperator('/')) {
		return nodeType::Div;
	}
	else if (token.isOperator('>')) {
		return nodeType::Lt;
	}
	else if (token.isOperator('<')) {
		return nodeType::Gt;
	}
	else if (token.isOperator("<=")) {
		return nodeType::Le;
	}
	else if (token.isOperator(">=")) {
		return nodeType::Ge;
	}
	else if (token.isOperator("==")) {
		return nodeType::Eq;
	}
	else if (token.isOperator("!=")) {
		return nodeType::Ne;
	}
	return nodeType::None;
}

void Parser::Program() {
	while (mCurrentPos < mTokenTbl.size()) {
		Node* node = Statement();
		mRootNodeTbl.push_back(node);
	}
}

Parser::Node* Parser::Statement() {
	Node* node = Expr();
	mTokenTbl.at(mCurrentPos++).expect(';');
	return node;
}

Parser::Node* Parser::Expr() {
	return Assign();
}

Parser::Node* Parser::Assign() {
	Node* node = Equality();
	if (mTokenTbl[mCurrentPos].isOperator('=')) {
		++mCurrentPos;
		node = PushBackNode(Node{ nodeType::Assign,  node, Assign() });
	}
	return node;
}

Parser::Node* Parser::Equality() {
	Node* node = Relational();
	while (!isEndOfState()) {
		switch (auto type = GetNodeType(mTokenTbl[mCurrentPos])) {
		case nodeType::Eq:
		case nodeType::Ne:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Relational() });
			continue;
		default:
			return node;
		}
	}
	return node;
}

Parser::Node* Parser::Relational() {
	Node* node = Add();
	while (!isEndOfState()) {
		switch (auto type = GetNodeType(mTokenTbl[mCurrentPos])) {
		case nodeType::Lt:
		case nodeType::Le:
		case nodeType::Gt:
		case nodeType::Ge:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Add() });
			continue;
		default:
			return node;
		}
	}
	return node;
}

Parser::Node* Parser::Add() {
	Node* node = Mul();
	while (!isEndOfState()) {
		switch (auto type = GetNodeType(mTokenTbl[mCurrentPos])) {
		case nodeType::Add:
		case nodeType::Sub:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Mul() });
			continue;
		default:
			return node;
		}
	}
	return node;
}
 
Parser::Node* Parser::Mul() {
	Node* node = Unary();
	while (!isEndOfState()) {
		switch (auto type = GetNodeType(mTokenTbl[mCurrentPos])) {
		case nodeType::Mul:
		case nodeType::Div:
			++mCurrentPos;
			node = PushBackNode(Node{ type, node, Unary() });
			continue;
		default:
			return node;
		}
	}
	return node;
}

Parser::Node* Parser::Unary() {
	switch (GetNodeType(mTokenTbl[mCurrentPos])) {
	case nodeType::Add:
		++mCurrentPos;
		return Primaly();
	case nodeType::Sub:   //単項-のときには0－Numに変換する
		++mCurrentPos;
		{
		Node* node = PushBackNode(Node{ nodeType::Num, nullptr, nullptr, 0 });
		return PushBackNode(Node{ nodeType::Sub, node, Primaly() });
		}
	default:
		return Primaly();
	}
}

Parser::Node* Parser::Primaly() {
	//次のトークンが"("なら、"(" expr ")"のはず	
	Token& t = mTokenTbl[mCurrentPos++];
	if (t.isOperator('(')) {
		Node* node = Expr();
		mTokenTbl[mCurrentPos++].expect(')');
		return node;
	}
	else if (t.isIdent()) {
		const int ofs = (t.mStr[0] - 'a' + 1) * 8;
		return PushBackNode(Node{ .type = nodeType::LovalVal, .offset = ofs });
	}

	// そうでなければ数値のはず
	return PushBackNode(Node{ nodeType::Num, nullptr, nullptr, t.expectNumber()});
}


void Parser::Parse(vector<Token>& tokenTbl) {
	mTokenTbl = tokenTbl;
	//Fix me : ポインタでlhs, rhsを保存しているため、サイズ拡張時にmoveが発生して壊れる。
	//回避策として多めにとっておく
	mNodeTbl.reserve(tokenTbl.size() * 10);
	if (mTokenTbl.size() > 0) {
		Program();
	}
}

