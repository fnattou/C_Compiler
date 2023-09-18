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
		return nodeType::Gt;
	}
	else if (token.isOperator('<')) {
		return nodeType::Lt;
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
	Node* node;
	const Token& t = getCurTk();
	const auto expectAndNext = [&](char c) { getCurTk().expect(c); ++mCurrentPos; };
	if (t.isReturn()) {
		++mCurrentPos;
		node = PushBackNode({ .type = nodeType::Return, .lhs = Expr() });
	}
	// if ( lhs ) middle  else ( rhs )
	else if (t.isReserved("if")) {
		++mCurrentPos;
		Node n{ .type = nodeType::If_ };
		expectAndNext('(');
		n.lhs = Expr();
		expectAndNext(')');
		n.middle = Statement();
		if (mCurrentPos < mTokenTbl.size() && getCurTk().isReserved("else")) {
			++mCurrentPos;
			n.rhs = Statement();
		}
		node = PushBackNode(n);
	}
	// for ( lhs ; middle ; rhs ) { statement }
	else if (t.isReserved("for")) {
		++mCurrentPos;
		Node n{ .type = nodeType::For_ };
		const auto readExprIf = [&](Node* n, char c) {
			if (!mTokenTbl[mCurrentPos + 1].isOperator(c)) {
				n = Expr();
			}
			expectAndNext(c);
		};

		expectAndNext('(');
		readExprIf(n.lhs, ';');
		readExprIf(n.middle, ';');
		readExprIf(n.rhs, ')');
		node = PushBackNode(n);
	}
	else if (t.isReserved("while")) {
		expectAndNext('(');
		node = PushBackNode({ .type = nodeType::While_, .lhs = Expr() });
		expectAndNext(')');
		node->rhs = Statement();
	}
	else {
		node = Expr();
		expectAndNext(';');
	}
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
		if (mLValMap.contains(t.mStr)) {
			return PushBackNode(Node{ .type = nodeType::LocalVal, .offset =  mLValMap.at(t.mStr)});
		}
		else {
			const int ofs = (mLValMap.size() + 1) * 8;
			mLValMap.emplace(t.mStr, ofs);
			return PushBackNode(Node{ .type = nodeType::LocalVal, .offset = ofs });
		}
	}

	// そうでなければ数値のはず
	return PushBackNode(Node{ .type = nodeType::Num, .val = t.expectNumber()});
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

