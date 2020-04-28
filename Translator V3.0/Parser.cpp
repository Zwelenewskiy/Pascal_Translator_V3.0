#include "Parser.h"

void Parser::Parse(string path)
{ 
	lexer = new Lexer(path);
	current_token = lexer->GetToken();

	bool correct = true;
	if (to_lower(current_token->value) == "type") {

		match(new Token("type"));

		while ((current_token->value != "function")
			&& (current_token->value != "procedure")
			&& (current_token->value != "var")
			&& (current_token->value != "begin")) {
			if (!parse_struct()) {
				correct = false;
				break;
			}
		}
	}
	else if (to_lower(current_token->value) == "function") {
		correct = parse_subprogramm(Function);
	}
	else if (to_lower(current_token->value) == "procedure") {
		correct = parse_subprogramm(Procedure);
	}
	else if (current_token->value == "var") {
		correct = parse_var();
	}
	else if (current_token->value != "begin") {
		correct = false;
	}
	
	while ((current_token != nullptr) && correct) {
		correct = stmt();
	}

	if(correct)
		cout << endl << "SYNTAX IS OK" << endl;
	else
		cout << endl << "SYNTAX ERROR" << endl;

	/*Token* token; 
	while (true) {
		token = lexer->GetToken();
		if (token == nullptr)
			break;

		cout << token->value << endl;
	}*/
}

bool Parser::parse_expr()
{
	while ((current_token->value != ";") && (current_token->value != "else")) {
		if (current_token->type == Identificator) {
			if (!match(Identificator))
				return false;

			if (!match(new Token(":="), false))
				return false;

			save_state();
			if (!match(Identificator, false)) {
				if (!match(Literal, false))
					return false;
			}
			else {
				load_state();
				if (!current_env->get(current_token) && !global_env->get(current_token)) {
					cout << endl << "TOKEN NOT DEFINED: " << current_token->value << endl;
					return false;
				}

				if (current_token->check_type == Function) {
					if (!parse_call(current_token))
						return false;
				}
				else {
					cout << endl << "EXPECTED FUNCTION: " << current_token->value << endl;
					return false;
				}
			}

			if (current_token->value == ";")
				return true;

			if (current_token->value == "else")
				return true;

			if (!match(LogicalOperator, false)) {
				if(!match(AriphmethicalOperator))
					return
						false;
			}

			if (!parse_expr())
				return false;
		}
		else if (current_token->type == LogicalOperator) {
			if (!match(LogicalOperator))
				return false;

			if (!match(Identificator))
				return false;

			if (!parse_expr())
				return false;
		}
		else if (current_token->type == AriphmethicalOperator) {
			if (!match(AriphmethicalOperator))
				return false;

			if (!match(Identificator))
				return false;

			if (!parse_expr())
				return false;
		}
		else if (current_token->type == Literal) {
			if (!match(Literal))
				return false;

			if (current_token->value == ";")
				return true;

			if (current_token->value == "else")
				return true;
			
			if (current_token->value == ")") {
				if (current_token->value == ";")
					return true;
				else {
					match(new Token(")"));
					continue;
				}
			}

			if (!match(LogicalOperator, false)) {
				if (!match(AriphmethicalOperator))
				return
					false;
			}

			if (!parse_expr())
				return false;
		}
		else if (current_token->value == "(") {
			if (!match(new Token("(")))
				return false;

			save_state();
			if (!match(Identificator, false)) {
				if (!match(Literal, false))
					return false;
			}
			else {
				load_state();
				if (!current_env->get(current_token) && !global_env->get(current_token)) {
					cout << endl << "TOKEN NOT DEFINED: " << current_token->value << endl;
					return false;
				}

				if (current_token->check_type == Function) {
					if (!parse_call(current_token))
						return false;
				}
				else {
					cout << endl << "EXPECTED FUNCTION: " << current_token->value << endl;
					return false;
				}
			}

			if (!match(LogicalOperator, false)) {
				if (!match(AriphmethicalOperator))
					return false;
			}

			if (!parse_expr())
				return false;
		}
		else if (current_token->value == ")") {
			if (!match(new Token(")")))
				return false;

			if (!match(LogicalOperator, false)) {
				if (!match(AriphmethicalOperator))
					return
					false;
			}

			if (!parse_expr())
				return false;
		}
		else
			return false;
	}

	return true;
}

bool Parser::parse_bool_expr()
{
	bracket_balance = 1;
	while (bracket_balance != 0) {
		Token* pred_token = current_token;

		if (bracket_balance == 0) {
			current_token = pred_token;
			return true;
		}

		if (current_token->type == Identificator) {
			if (!match(Identificator))
				return false;

			if (current_token->value == ")") {
				bracket_balance--;

				if (bracket_balance == 0)
					return true;
			}

			if (!match(LogicalOperator, false))
				return false;

			if (current_token->value == "(")
				bracket_balance++;

			if (!parse_bool_expr())
				return false;
		}
		else
			return false;
	}
}

bool Parser::parse_call(Token* subprogram_token)
{
	Token* tmp = current_token;
	if (!match(Identificator))
		return false;

	if (!match(new Token("(")))
		return false;

	vector<Variable> signature;
	if (!parse_call_param_list(signature))
		return false;
	
	if (global_env->check_signature(subprogram_token, signature)) {
		cout << endl << "PARAMETERS DON'T MATCH THE SIGNATURE: " << tmp->value << endl;
		return false;
	}

	if (!match(new Token(")")))
		return false;

	if (current_token->value == ";")
		return true;
	else
		return false;
}

bool Parser::parse_subprogramm(CheckTokenType type, bool global)
{
	if(global)
		current_env = new Env();

	if (type == Function) {
		if (!match(new Token("function")))
			return false;

		current_token->check_type = Function;
	}
	else if (type == Procedure) {
		if (!match(new Token("procedure")))
			return false;

		current_token->check_type = Procedure;
	}	

	Token* subprogramm_token = current_token;

	if (current_token->type != Identificator)
		return false;	

	if (!match(Identificator))
		return false;

	if (!match(new Token("(")))
		return false;
	
	vector<Variable> signature;
	if (!parse_param_list(signature))
		return false;

	subprogramm_token->signature = signature;

	if (global) {
		if (!global_env->get(subprogramm_token)) {
			global_env->put(subprogramm_token);
		}
		else {
			cout << endl << "TOKEN ALREADY EXIST: " << subprogramm_token->value << endl;
			return false;
		}
	}
	else {
		if (!current_env->get(subprogramm_token)) {
			current_env->put(subprogramm_token);
		}
		else {
			cout << endl << "TOKEN ALREADY EXIST: " << subprogramm_token->value << endl;
			return false;
		}
	}	

	if (!match(new Token(")")))
		return false;

	if (type == Function) {
		if (!match(new Token(":")))
			return false;

		if (!match(TypeData))
			return false;
	}	

	if (!match(new Token(";")))
		return false;

	if (current_token->value == "var")
		if (!parse_var())
			return false;

	if (current_token->value != "begin")
		return false;

	if (match(new Token("end"), false))
		return true;
	else {
		if (!stmt())
			return false;
	}
}

bool Parser::parse_param_list(vector<Variable>& signature)
{
	Token* tmp_token;

	while (true) {
		vector<Token*> tmp_vars;
		tmp_token = current_token;

		if (!match(Identificator, false)) {
			if (current_token->value != ")")
				return false; 
			else
				return true;
		}
		else {
			if (!current_env->get(tmp_token)) {
				current_env->put(tmp_token);

				tmp_vars.push_back(tmp_token);
			}
			else {
				cout << endl << "TOKEN NOT DEFINED: " << tmp_token->value << endl;
				return false;
			}
		}		

		if (!match(new Token(","), false)) {
			if (match(new Token(":"))) {
				Token* data_type = current_token;
				if (!match(TypeData))
					return false;
				else {
					for (int i = 0; i < tmp_vars.size(); i++) {
						if(data_type->value == "integer")
							signature.push_back(Variable(tmp_vars[i]->value, Integer));
						else if (data_type->value == "boolean")
							signature.push_back(Variable(tmp_vars[i]->value, Boolean));
						else if (data_type->value == "char")
							signature.push_back(Variable(tmp_vars[i]->value, Char));
						else if (data_type->value == "double")
							signature.push_back(Variable(tmp_vars[i]->value, Double));
						else if (data_type->value == "string")
							signature.push_back(Variable(tmp_vars[i]->value, String));
					}
				}
			}
		}
		else
			continue;

		if (!match(new Token(";"), false)) {
			if (current_token->value != ")")
				return false;
			else
				return true;
		}			
	}
}

bool Parser::parse_call_param_list(vector<Variable>& signature){
	;
	while (true) {
		Token* tmp = current_token;
		if (!match(Identificator, false)) {
			tmp = current_token;
			if (!match(Literal, false))
			{
				if (current_token->value != ")")
					return false;
			}			
			else {
				signature.push_back(Variable(tmp->value, define_data_type(tmp)));
			}
		} 
		else {
			if (global_env->get(tmp, false)) {
				signature.push_back(Variable(tmp->value, tmp->data_type)); 
			}
			else {
				cout << endl << "TOKEN NOT DEFINED: " << tmp->value << endl;
				return false;
			}
		}

		if (current_token->value == ")")
			return true;

		if (!match(new Token(","), false)) {
			return false;
		}
		else
			continue;
	}
}

bool Parser::parse_var(bool global, bool in_struct)
{
	Env* env;
	Token* tmp_token;

	if (global)
		env = global_env;
	else
		env = new Env();
	
	if (!in_struct) {
		if (!match(new Token("var")))
			return false;
	}	

	if (current_token->type != Identificator) 
		return false;

	//while (current_token->value != "begin")

	vector<Token*> tmp_vars;
	while (true)
	{
		if (!in_struct) {
			if (current_token->value == "begin")
				break;
		}
		else {
			if ((current_token->value == "end")
				|| (current_token->value == "procedure")
|| (current_token->value == "function"))
break;
		}

		if (current_token->type == Identificator) {
			current_token->check_type = Var;

			if (global) {
				if (!global_env->get(current_token)) {
					for (Token* t : tmp_vars) {
						if (t->value == current_token->value) {
							cout << endl << "TOKEN ALREADY EXIST: " << current_token->value << endl;
							return false;
						}
					}

					tmp_vars.push_back(current_token);
				}
				else {
					cout << endl << "TOKEN ALREADY EXIST: " << current_token->value << endl;
					return false;
				}
			}
			else {
				if (!env->get(current_token) && !global_env->get(current_token)) {
					for (Token* t : tmp_vars) {
						if (t->value == current_token->value) {
							cout << endl << "TOKEN ALREADY EXIST: " << current_token->value << endl;
							return false;
						}
					}

					tmp_vars.push_back(current_token);
				}
				else {
					cout << endl << "TOKEN ALREADY EXIST: " << current_token->value << endl;
					return false;
				}
			}

			if (!match(Identificator)) {
				return false;
			}

			if (!match(new Token(","), false)) {
				if (match(new Token(":"))) {
					Token* data_type = current_token;

					if (match(TypeData)) {
						if (!match(new Token(";"))) {
							return false;
						}
						else {
							for (int i = 0; i < tmp_vars.size(); i++) {

								if (data_type->value == "integer")
									tmp_vars[i]->data_type = Integer;
								else if (data_type->value == "boolean")
									tmp_vars[i]->data_type = Boolean;
								else if (data_type->value == "char")
									tmp_vars[i]->data_type = Char;
								else if (data_type->value == "double")
									tmp_vars[i]->data_type = Double;
								else if (data_type->value == "string")
									tmp_vars[i]->data_type = String;

								env->put(tmp_vars[i]);
							}

							tmp_vars.clear();
							continue;
						}
					}
				}
				else
					return false;
			}
		}
		else
			return false;
	}

	if (global)
		current_env = global_env;
	else
		current_env = env;

	return true;
}

bool Parser::parse_struct()
{
	CheckTokenType type;

	Token* tmp = current_token;
	if (!match(Identificator)) {
		return false;
	}
	else {
		if (global_env->get(tmp, false)) {
			cout << endl << "TOKEN ALREADY  EXIST: " << tmp->value << endl;
			return false;
		}
	}

	if (!match(new Token("=")))
		return false;

	if (!match(new Token("record"), false)) {
		if (!match(new Token("class"))) {
			cout << endl << "EXPECTED CLASS OR RECORD: " << endl;
			return false;
		}
		else
			type = Class;
	}
	else
		type = Record;

	if (current_token->type == Identificator) {
		if (!parse_var(false, true))
			return false;
	}

	while (current_token->value != "end") {
		if (current_token->value == "function") {
			if (!parse_subprogramm(Function, false))
				return false;
		}
		else if (current_token->value == "procedure") {
			if (!parse_subprogramm(Procedure, false))
				return false;
		}
		else {
			cout << endl << "EXPECTED FUNCTION, PROCEDURE OR VARIABLE DECLARATION: " << endl;
			return false;
		}
	}

	match(new Token("end"));
	if (!match(new Token(";"))) {
		return false;
	}
	else {
		tmp->check_type = type;
		tmp->members = current_env;

		global_env->put(tmp);

		return true;
	}
}

void Parser::save_state()
{
	tmp_current_file_pos = lexer->current_file_pos;
	tmp_current_token = current_token;
}

void Parser::load_state()
{
	lexer->current_file_pos = tmp_current_file_pos;
	current_token = tmp_current_token;
}

DataTypes Parser::define_data_type(Token * token)
{
	if (Match_Reg(token->value, CHAR))
		return Char;

	if (Match_Reg(token->value, STRING))
		return String;

	if ((to_lower(token->value) == "true") || (to_lower(token->value) == "false"))
		return Boolean;

	if (Match_Reg(token->value, INTEGER))
		return Integer;

	if (Match_Reg(token->value, DIGIT))
		return Double;

	return DataTypes();
}

bool Parser::stmt()
{
	if (current_token == nullptr) {
		return true;
	}

	if (to_lower(current_token->value) == "function") {
		if (!parse_subprogramm(Function))
			return false;
	}
	else if(to_lower(current_token->value) == "procedure") {
		if (!parse_subprogramm(Procedure))
			return false;
	}
	else if(current_token->value == "var") {
		if (!parse_var(true))
			return false;
	}
	else if (current_token->value == "begin") {
		operator_brackets_balance++;

		if (!match(new Token("begin")))
			return false;

		if (!stmt())
			return false;	

		if (!match(new Token("end"))) {
			return false;
		}
		else {
			operator_brackets_balance--;
		}

		if (match(new Token(";"), false)) {
			return true;
		}
		else if (match(new Token("."))) {
			if (operator_brackets_balance != 0)
				return false;

			if (current_token != nullptr) {
				cout << endl << "EXPECTED END OF FILE" << endl;
				return false;
			}
			else
				return true;
		}
		else
			return false;
	}
	else if (current_token->value == "end") {
		if (operator_brackets_balance == 0)
			return false;

		return true;
	}
	else if (current_token->value == "var") {
		if (!parse_var())
			return false;
	}
	else if (current_token->value == "if") {
		if (!match(new Token("if")))
			return false;

		if (!match(new Token("("))) 
			return false;

		if (!parse_bool_expr())
			return false;

		if (!match(new Token(")")))
			return false;

		if (!match(new Token("then")))
			return false;

		if (!stmt())
			return false;
	}
	else  if (current_token->type == Identificator) {
		save_state();

		if (!current_env->get(current_token, false) && !global_env->get(current_token, false)) {
			cout << endl << "TOKEN NOT DEFINED: " << current_token->value << endl;
			return false;
		}

		if (current_token->check_type == Var) {
			if (!parse_expr())
				return false;
		}
		else if ((current_token->check_type == Function) ||
				(current_token->check_type == Procedure)) {
			if (!parse_call(current_token))
				load_state();
		}	

		if(match(new Token("else"), false)){
			if (!stmt())
				return false;
		}
		else if (!match(new Token(";"))) 			
			return false;
		else if (!stmt())
			return false;
	}
	else if (current_token->value == "while") {
		if (!match(new Token("while")))
			return false;

		if (!match(new Token("(")))
			return false;

		if (!parse_bool_expr())
			return false;

		if (!match(new Token(")")))
			return false;

		if (!match(new Token("do")))
			return false;

		if (!stmt())
			return false;
	}
	else {
		return false;
	}

	return true;
}

bool Parser::match(Token* token, bool show_error)
{
	if (token->value == current_token->value) {		
			current_token = lexer->GetToken();
			return true;
	}
	else if(show_error){
		cout << endl << "EXPECTED TOKEN: " << token->value << endl;
		return false;
	}
}

bool Parser::match(TokenType token_type, bool show_error)
{
	if (token_type == current_token->type) {
		current_token = lexer->GetToken();

		if (current_token == nullptr) {
			cout << endl << "EXPECTED TOKEN TYPE: " << token_type << " but found the end of the file" << endl;
			return false;
		}
		return true;
	}
	else if (show_error) {
		cout << endl << "EXPECTED TOKEN TYPE: " << token_type << endl;
		return false;
	}
}

Parser::Parser(){ operator_brackets_balance = bracket_balance = 0; }

Parser::~Parser(){}
