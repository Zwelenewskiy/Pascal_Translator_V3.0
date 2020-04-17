#pragma once
#include <iostream>
#include "Lexer.h"
#include "Env.h";

using namespace std;

class Parser
{
private:
	int bracket_balance;
	streampos tmp_current_file_pos;

	Token* current_token;
	Token* tmp_current_token;
	Lexer* lexer;

	bool parse_expr();
	bool parse_bool_expr();

	bool parse_function();
	bool parse_procedure();

	bool parse_call();
	bool parse_param_list();
	bool parse_call_param_list();
	bool parse_var(bool global = false);

	void save_state();
	void load_state();

	//var - ����� ������ ���������� ���������� 								
	bool match(Token* token, bool show_error = true);
	bool match(TokenType token_type, bool show_error = true);
	bool stmt();

public:
	void Parse(string path);

	Parser();
	~Parser();
};

