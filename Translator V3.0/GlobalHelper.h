#pragma once
using namespace std;

#define DIGIT "^[0-9]*[.,]?[0-9]+$"
#define IDENTIFICATOR "([A-Z]|[a-z])([A-Z]|[a-z]|[0-9])*"
#define STRING "'([A-Z]|[a-z]|[0-9]|\s)*'"
#define CHAR "'\S{1}'"

struct Node//���������, ����������� ���� AST-������
{
	string name;
	string data;

	Node* left;
	Node* right;
	Node* next;

	Node* condition;//��� ��������� �������� ���������

	Node()
	{
		name = "";
		data = "";
		left = NULL;
		right = NULL;
		condition = NULL;
	}
};

vector<string> InfixToPostfix(string infix, char type = 'a');
bool Match_Reg(string input, string pattern);
string to_lower(string s);

Node* buildATree(vector <string> postfix);
void showMixed(Node* root, int depth);
string get_token();