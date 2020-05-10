#pragma once
#include "Lexer.h"

enum TreeType {
	Ariphmethik,
	Bool
};

stack<string> operation_stack;
stack <Node*> tmp_stack;
map<string, int> prec;//����������� ��������
vector<Token*> postfixList;

Node* builded_tree;

vector<Token*> infix_to_postfix(Token* token, TreeType type)
{
	//���������� ��������� ��������
	if (type == Bool)// ��� ��������� ���������
	{
		prec["="] = 5;
		prec[">"] = 5;
		prec["<"] = 5;
		prec["<="] = 5;
		prec["=>"] = 5;
		prec["<>"] = 5;
		prec["!"] = 4;
		prec["&&"] = 3;
		prec["||"] = 2;
	}
	else //��� ��������������
	{
		prec["*"] = 3;
		prec["/"] = 3;
		prec["+"] = 2;
		prec["-"] = 2;
	}

	prec["("] = 1;

	//Token* token = lexer->GetToken();
	//while (token->value != ";")
	//{
	//����������� ���� ������
	if (Match_Reg(token->value, DIGIT) || Match_Reg(token->value, IDENTIFICATOR))
	{
		postfixList.push_back(token);
	}
	else if (token->value == "(")
	{
		operation_stack.push(token->value);
	}
	else if (token->value == ")")
	{
		string top_token = operation_stack.top();
		operation_stack.pop();
		while (top_token != "(")
		{
			postfixList.push_back(new Token(top_token));
			top_token = operation_stack.top();
			operation_stack.pop();
		}
	}
	else
	{
		while ((!operation_stack.empty()) && (prec[operation_stack.top()] >= prec[token->value]))
		{
			postfixList.push_back(new Token(operation_stack.top()));
			operation_stack.pop();
		}

		operation_stack.push(token->value);
	}
	//}

	while (!operation_stack.empty())
	{
		postfixList.push_back(new Token(operation_stack.top()));
		operation_stack.pop();
	}

	return postfixList;
}

void build_tree(Token* token, bool end)//������� ��� ���������� ������
{	
	Node* tmp = new Node();
	tmp->data = token;

	if (Match_Reg(token->value, DIGIT) || Match_Reg(token->value, IDENTIFICATOR))
	{
		if (!tmp_stack.empty())
			tmp->next = tmp_stack.top();
		tmp_stack.push(tmp);
	}
	else
	{
		tmp->right = tmp_stack.top();

		if (token->value != "not") {
			tmp->left = tmp_stack.top()->next;
			tmp->next = tmp_stack.top()->next->next;
		}

		tmp_stack.push(tmp);
	}

	if(end)
		builded_tree = tmp_stack.top();
}

void create_ast_tree(Token* token, TreeType type, bool end = false) {
	infix_to_postfix(token, type);
	build_tree(token, end);
}

void clear() {
	while (!tmp_stack.empty())
		tmp_stack.top();

	while (!operation_stack.empty())
		operation_stack.top();

	prec.clear();
	postfixList.clear();
}