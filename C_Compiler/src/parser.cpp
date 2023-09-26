#pragma once
#include "parser.h"
#include <iostream>

Parser::Node* Parser::PushBackNode(Node n) {
	mNodeTbl.push_back(n);
	return &mNodeTbl.at(mNodeTbl.size() - 1);
}

Parser::nodeType Parser::GetNodeType(const Token& token) const {
	if (token.isReturn()) {
		return nodeType::Return;
	}

	static const std::unordered_map<string_view, nodeType>  map = {
		{"+", nodeType::Add}, {"-", nodeType::Sub}, {"*", nodeType::Mul},
		{"/", nodeType::Div}, {">", nodeType::Gt}, {"<", nodeType::Lt},
		{"<=", nodeType::Le}, {">=", nodeType::Ge}, {"==", nodeType::Eq},
		{"!=", nodeType::Ne}, {"if", nodeType::If_}, {"for", nodeType::For_},
		{"while", nodeType::While_}, {"{", nodeType::Block}, {"&", nodeType::Addr}
	};
	for (const auto& pair : map) {
		if (token.isReserved(pair.first)) {
			return pair.second;
		}
	}
	return nodeType::None;
}

void Parser::Program() {
	while (mCurrentPos < mTokenTbl.size()) {
			mRootNodeTbl.push_back(Function());
	}
}

Parser::Node* Parser::Function() {
	Node n{ .type = nodeType::DeclareFunc }; 
	mFuncInfoTbl.emplace(getCurTk().mStr,  FuncInfo{ .name = getCurTk().mStr });
	mCurrentFuncInfoPtr = &mFuncInfoTbl[getCurTk().mStr];
	n.funcInfoPtr = mCurrentFuncInfoPtr;
	//�֐���
	if (!getCurTk().isIdent()) {
		std::cerr << "�֐��錾����n�܂��Ă��܂���" << std::endl;
		assert(0);
	}
	++mCurrentPos;

	//��������
	getCurTk().expect('('); ++mCurrentPos;
	while (!getCurTk().isReserved(')')) {
		const int ofs = (mCurrentFuncInfoPtr->lValMap.size() + 1) * 8;
		mCurrentFuncInfoPtr->lValMap.emplace(getCurTk().mStr, ofs);
		auto p = PushBackNode(Node{ .type = nodeType::LocalVal, .offset = ofs });
		mCurrentFuncInfoPtr->argumentNodeTbl.push_back(p);
		++mCurrentPos;
	}
	++mCurrentPos;

	//�֐��̖{��
	getCurTk().expect('{'); ++mCurrentPos;
	while (!getCurTk().isReserved('}')) {
		Node* node = Statement();
		n.innerBlockNodeTbl.push_back(node);
	}
	++mCurrentPos;
	return PushBackNode(n);
}

Parser::Node* Parser::Statement() {
	Node* node;
	const auto expectAndNext = [&](char c) { getCurTk().expect(c); ++mCurrentPos; };
	switch ( auto type = GetNodeType(getCurTk()) ) {
	case nodeType::Return:
		++mCurrentPos;
		node = PushBackNode({ .type = type, .lhs = Expr() });
		expectAndNext(';');
		break;

	// if ( lhs ) middle  else ( rhs )
	case nodeType::If_: {
		++mCurrentPos;
		Node n{ .type = type };
		expectAndNext('(');
		n.lhs = Expr();
		expectAndNext(')');
		n.middle = Statement();
		if (mCurrentPos < mTokenTbl.size() && getCurTk().isReserved("else")) {
			++mCurrentPos;
			n.rhs = Statement();
		}
		node = PushBackNode(n);
		break;
	}

	// for ( lhs ; middle ; rhs ) { statement }
	case nodeType::For_: {
		++mCurrentPos;
		Node n{ .type = type };
		const auto readExprIf = [&](Node*& n, char c) {
			if (!mTokenTbl[mCurrentPos + 1].isReserved(c)) {
				n = Expr();
			}
			expectAndNext(c);
			};
		expectAndNext('(');
		readExprIf(n.lhs, ';');
		readExprIf(n.middle, ';');
		readExprIf(n.rhs, ')');
		node = PushBackNode(n);
		break;
	}
	case nodeType::While_:
		++mCurrentPos;
		expectAndNext('(');
		node = PushBackNode({ .type = type, .lhs = Expr() });
		expectAndNext(')');
		node->rhs = Statement();
		break;
	case nodeType::Block: {
		++mCurrentPos;
		Node n{ .type = type };
		while (!getCurTk().isReserved('}')) {
			n.innerBlockNodeTbl.push_back(Statement());
		}
		expectAndNext('}');
		node = PushBackNode(n);
		break;
	}
	default:
		node = Expr();
		expectAndNext(';');
		break;
	}
	return node;
}

Parser::Node* Parser::Expr() {
	return Assign();
}

Parser::Node* Parser::Assign() {
	Node* node = Equality();
	if (getCurTk().isReserved('=')) {
		++mCurrentPos;
		node = PushBackNode(Node{ nodeType::Assign,  node, Assign() });
	}
	return node;
}

Parser::Node* Parser::Equality() {
	Node* node = Relational();
	while (!isEndOfState()) {
		switch (auto type = GetNodeType(getCurTk())) {
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
		switch (auto type = GetNodeType(getCurTk())) {
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
		switch (auto type = GetNodeType(getCurTk())) {
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
		switch (auto type = GetNodeType(getCurTk())) {
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
	switch (GetNodeType(getCurTk())) {
	case nodeType::Add:
		++mCurrentPos;
		return Primaly();
	case nodeType::Sub:   //�P��-�̂Ƃ��ɂ�0�|Num�ɕϊ�����
		++mCurrentPos;
		{
		Node* node = PushBackNode(Node{ nodeType::Num, nullptr, nullptr, 0 });
		return PushBackNode(Node{ nodeType::Sub, node, Primaly() });
		}
	case nodeType::Addr:
		++mCurrentPos;
		return PushBackNode(Node{ .type = nodeType::Addr, .rhs = Unary() });
	case nodeType::Mul: //getnodeType��'*'��Mul�Ƃ��ĕԂ�, �����ł�deref
		++mCurrentPos;
		return PushBackNode(Node{ .type = nodeType::Deref, .rhs = Unary() });
	default:
		return Primaly();
	}
}

Parser::Node* Parser::Primaly() {
	Token& t = mTokenTbl[mCurrentPos++];
	//���̃g�[�N����"("�Ȃ�A"(" expr ")"�̂͂�	
	if (t.isReserved('(')) {
		Node* node = Expr();
		mTokenTbl[mCurrentPos++].expect(')');
		return node;
	}
	//�ϐ����������͊֐���
	if (t.isIdent()) {

		//"(" �������Ȃ�֐��Ăяo��
		if (mTokenTbl[mCurrentPos].isReserved("(")) {
			Node n{ .type = nodeType::CallFunc , .funcInfoPtr = &mFuncInfoTbl[t.mStr]};
			++mCurrentPos;
			while (!getCurTk().isReserved(")")) {
				n.argumentNodeTbl.push_back(Expr());
				if (getCurTk().isReserved(",")) ++mCurrentPos;
			}
			++mCurrentPos;
			return PushBackNode(n);
		}

		//�ϐ�
		if (mCurrentFuncInfoPtr->lValMap.contains(t.mStr)) {
			return PushBackNode(Node{
				.type = nodeType::LocalVal,
				.offset = mCurrentFuncInfoPtr->lValMap.at(t.mStr)
				});
		}
		const int ofs = (mCurrentFuncInfoPtr->lValMap.size() + 1) * 8;
		mCurrentFuncInfoPtr->lValMap.emplace(t.mStr, ofs);
		return PushBackNode(Node{ .type = nodeType::LocalVal, .offset = ofs });
	}

	// �����łȂ���ΐ��l�̂͂�
	return PushBackNode(Node{ .type = nodeType::Num, .val = t.expectNumber()});
}


void Parser::Parse(vector<Token>& tokenTbl) {
	mTokenTbl = tokenTbl;
	//Fix me : �|�C���^��lhs, rhs��ۑ����Ă��邽�߁A�T�C�Y�g������move���������ĉ���B
	//�����Ƃ��đ��߂ɂƂ��Ă���
	mNodeTbl.reserve(tokenTbl.size() * 10);
	mRootNodeTbl.reserve(tokenTbl.size());
		if (mTokenTbl.size() > 0) {
		Program();
	}
}

