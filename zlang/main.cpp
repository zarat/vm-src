////////////////////////////////////////////////////////////////////////////////////////////
//	WINGS : Interpreter for BIRD programming language
////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <time.h>

//Token types
#define INTEGER "INTEGER"
#define STRING "STRING"
#define PLUS "PLUS"
#define MINUS "MINUS"
#define MUL "MUL"
#define DIV "DIV"
#define MOD "MOD"
#define LPAREN "LPAREN"
#define RPAREN "RPAREN"
#define SEMI "SEMI"
#define COMMA "COMMA"
#define ASSIGN "ASSIGN"
#define VARIABLE "VARIABLE"
#define STATEMENT_LIST "STATEMENT_LIST"
#define EQUALITY "EQUALITY"
#define LBRACKET "LBRACKET"
#define RBRACKET "RBRACKET"
#define LESSTHAN "LESSTHAN"
#define MORETHAN "MORETHAN"
#define EMPTY "EMPTY"
#define IF "if"
#define ELSE "else"
#define WHILE "while"
#define ENDL "endl"
#define WRITE "write"
#define READ "read"
#define STRLEN "strlen"
#define STRCMP "strcmp"

//Containers to store reserved keywords, variables : value and types
std::set< std::string > RESERVED_KEYWORDS;
std::map<std::string, std::string> GLOBAL_SCOPE;
std::map<std::string, std::string> VARIABLE_TYPES;
std::map<std::string, int> VARIABLE_IDX;

int variable_idx = 0;
int label_idx = 0;

////////////////////////////////////////////////////////////////////////////////////////////
//	LEXER
////////////////////////////////////////////////////////////////////////////////////////////

class Token
{
private:
	//token type: eg: int, plus, EOF, etc
	std::string type;
	//token value: eg: 0, 1, '+', 'EOF'
	std::string value;

public:
	Token()
	{
		type = "";
		value = "";
	}

	Token(std::string _type, std::string _value)
	{
		type = _type;
		value = _value;
	}

	~Token(){};

	//String representation of class instance
	// eg:  Token(int,3)    Token(plus,'+')
	std::string str()
	{
		return ("Token("+type+","+value+")");
	}

	//returns value of this token
	std::string _value()
	{
		return value;
	}
	//returns type of this token
	std::string _type()
	{
		return type;
	}
};





class Lexer
{
private:
	//Code in bird
	std::string text;
	//current char at text[pos]
	char current_char;
	//current index of text to be tokenized
	int pos;

public:
	Lexer()
	{
		text = "";
		pos = 0;
		current_char = EOF;
	}
	Lexer(std::string _text)
	{
		text = _text;
		pos = 0;
		current_char = text[pos];
	}
	~Lexer(){};

	void error()
	{
		std::cout<<"Lexer Error:\n";
		std::cout<<"pos = "<<pos<<"   text[pos] = "<<text[pos]<<"\n";
		std::exit(0);
	}

	//Ignores whitespaces and put pos pointer to a non-whitespace character
	void skip_whitespaces();
	//Ignores comments
	void skip_comments();
	//Advances position of pos pointer in text, and check if EOF reached
	void advance_pos();
	//returns a multi-digit integer in form of a string starting at pos, and shift pos to next pointer
	std::string integer();
	//returns a string which is present under double quotes in bird
	std::string get_string();
	//get arguments for write()
	void write_arg();
	//Most important function in Lexical analyzer.
	//This method breaks the text into tokens, and returns a token starting at pos
	Token get_next_token();
	char peek();
	Token _id();
};

void Lexer::skip_whitespaces()
{
	//as the name suggests, skip all whitespaces till any new character is encountered
	while(text[pos] == ' ' or text[pos] == '\t' or text[pos] == '\n')
	{
		advance_pos();
	}
}

void Lexer::skip_comments()
{
	advance_pos();
	advance_pos();
	while((text[pos] != '/' or peek() != '/') and text[pos] != EOF)
	{
		advance_pos();
	}
	advance_pos();
	advance_pos();
}

void Lexer::advance_pos()
{
	pos++;
	//if pos past the end of text, i.e tokenization is completed
	//return EOF token
	if(pos >= text.length())
		current_char = EOF;
	else current_char = text[pos];
}

std::string Lexer::integer()
{
	std::string str;
	while(current_char>=48 and current_char<=57)
	{
		str.push_back(current_char);
		advance_pos();
	}
	// std::cout<<"Lexer: integer():  str = "<<str<<"   current_char = "<<current_char<<"\n";
	if(current_char == '.')
	{
		str.push_back(current_char);
		advance_pos();
		while(current_char>=48 and current_char<=57)
		{
			str.push_back(current_char);
			advance_pos();
		}
		// std::cout<<"Double value here  str = "<<str<<"\n";		
	}
	return str;
}

std::string Lexer::get_string()
{
	std::string str;
    advance_pos(); // remove first '"'                   
    while(current_char != '\"') {    
        // Escape characters in strings (t, n, r, b, f, '"')
        if(current_char == '\\' && peek() == '"') { str.push_back('"'); advance_pos(); }
        else if(current_char == '\\' && peek() == 't') { str.push_back('\t'); advance_pos(); } 
        else if(current_char == '\\' && peek() == 'n') { str.push_back('\n'); advance_pos(); } 
        else if(current_char == '\\' && peek() == 'r') { str.push_back('\r'); advance_pos(); } 
        else if(current_char == '\\' && peek() == 'b') { str.push_back('\b'); advance_pos(); }
        else if(current_char == '\\' && peek() == 'f') { str.push_back('\f'); advance_pos(); }  
        else if(current_char == '\\' && peek() == '\\') { str.push_back('\\'); advance_pos(); }  
        else { str.push_back(current_char); } 
        advance_pos();        
    } 
    advance_pos(); // remove last '"'
    return str;
}

//Only peeks into input buffer without actually consuming the next character
char Lexer::peek()
{
	//if peek_pos past the end of text, return EOF
	if(pos+1 >= text.length())
		return EOF;
	else return text[pos+1];
}

Token Lexer::_id()
{
	std::string result;
	Token token;
	while((current_char >= 48 and current_char <= 57) or (current_char >= 65 and current_char <=90) or current_char >= 97 and current_char <=122)
	{
		result.push_back(current_char);
		advance_pos();
	}
	auto it = RESERVED_KEYWORDS.find(result);
	if(it == RESERVED_KEYWORDS.end())
		token = Token(VARIABLE, result);
	else token = Token(result, result);
	return token;
}

Token Lexer::get_next_token()
{
	//ignore whitespaces
	skip_whitespaces();

	//skip comments
	while(current_char == '/' and peek() == '/')
	{
		skip_comments();
	}
    
	std::string temp_str;
    
    if(current_char == ',')
	{
		// std::cout<<"plus here\n";
		temp_str.push_back(current_char);
		advance_pos();
		return Token(COMMA, temp_str);
	}

	//if current_char is an integer, create an integer token and increment pos
	if(current_char >= 48 and current_char <=57)
	{
		// std::cout<<"yup int here\n";
		temp_str = integer();//remember integer() returned multi-digit interger in form of a string
		return Token(INTEGER, temp_str);
	}

	//if token is + operator
	if(current_char == '+')
	{
		// std::cout<<"plus here\n";
		temp_str.push_back(current_char);
		advance_pos();
		return Token(PLUS, temp_str);
	}

	//if token is - operator
	if(current_char == '-')
	{
		// std::cout<<"minus here\n";
		temp_str.push_back(current_char);
		advance_pos();
		return Token(MINUS, temp_str);
	}

	//if token is * operator
	if(current_char == '*')
	{
		// std::cout<<"mul here\n";
		temp_str.push_back(current_char);
		advance_pos();
		return Token(MUL, temp_str);
	}

	//if token is / operator
	if(current_char == '/')
	{
		// std::cout<<"div here\n";
		temp_str.push_back(current_char);
		advance_pos();
		return Token(DIV, temp_str);
	}

	//if token is % operator
	if(current_char == '%')
	{
		// std::cout<<"mod here\n";
		temp_str.push_back(current_char);
		advance_pos();
		return Token(MOD, temp_str);
	}

	//if token is ( left parenthesis
	if(current_char == '(')
	{
		// std::cout<<"lparen here\n";
		temp_str.push_back(current_char);
		advance_pos();
		return Token(LPAREN, temp_str);
	}

	//if token is ) right paranthesis
	if(current_char == ')')
	{
		// std::cout<<"RPAREN here\n";
		temp_str.push_back(current_char);
		advance_pos();
		return Token(RPAREN, temp_str);
	}

	//EOF reached
	if(current_char == EOF)
	{
		temp_str.push_back(current_char);
		// std::cout<<"File ends here\n";
		return Token("EOF", temp_str);//some work needs to be done here ;p
	}

	//indentifier: some variable or a keyword
	if((current_char >= 65 and current_char <=90) or (current_char >= 97 and current_char <=122))
		return _id();

	if(current_char == '=' and peek() == '=')
	{
		advance_pos();
		advance_pos();
		return Token(EQUALITY, "==");
	}

	if(current_char == '<')
	{
		advance_pos();
		return Token(LESSTHAN, "<");
	}

	if(current_char == '>')
	{
		advance_pos();
		return Token(MORETHAN, ">");
	}

	//Assignment operator
	if(current_char == '=' and peek() != '=')
	{
		Token token(ASSIGN, "=");
		advance_pos();
		return token;
	}

	if(current_char == ';')
	{
		advance_pos();
		return Token(SEMI, ";");
	}

	if(current_char == '{')
	{
		advance_pos();
		return Token(LBRACKET, "{");
	}

	if(current_char == '}')
	{
		advance_pos();
		return Token(RBRACKET, "}");
	}

	if(current_char == '\"')
	{
		temp_str = get_string();
		return Token(STRING, temp_str);
	}

	error();
}





////////////////////////////////////////////////////////////////////////////////////////////
//	PARSER
////////////////////////////////////////////////////////////////////////////////////////////

class ASTNode
{
private:
	Token token;
public:
	std::vector< ASTNode* > child;
	ASTNode(){};
	ASTNode(Token _token)
	{
		token = _token;
	}
	~ASTNode(){};
	void make_child(ASTNode _node)
	{
		ASTNode *temp = new ASTNode(_node._token());
		temp->child = _node.child;
		child.push_back(temp);
	}
	Token _token()
	{
		return token;
	}
	//Method to print AST, AST means level of node, root is at level zero
	void show(int level)
	{
        std::cout << ";" << std::string(level*2, ' ') << "Token: level = " << level << "  type = "<<(token._type())<<"  value = "<<(token._value())<<std::endl;
		for(auto it = child.begin(); it != child.end(); it++)
			(*it)->show(level+1);
	}
};


class Parser
{
private:
	Lexer lexer;
	Token current_token;
public:
	Parser(){};
	Parser(Lexer _lexer)
	{
		lexer = _lexer;
		current_token = lexer.get_next_token();
	}
	~Parser(){};


	// compare the current token type with the passed token
    // type and if they match then "eat" the current token
    // and assign the next token to the self.current_token,
    // otherwise raise an exception.
	void eat(std::string token_type);
	void error()
	{
		std::cout<<"Parser Error:\n";
		std::cout<<"current_token = "<<(current_token.str())<<"\n";
		std::exit(0);
	}

	//Arithmatic expression parser/interpreter
	//factor : (PLUS|MINUS)factor | INTEGER | LPAREN expr RPAREN
	ASTNode factor();
	//term : PLUS factor | MINUS factor | INTEGER | LPAREN exor RPAREN | variable
	ASTNode term();
	//expr : term((PLUS | MINUS) term)*
	ASTNode expr();
	//assignment_statement : variable ASSIGN expr
	ASTNode assignment_statement();
	//statement : assignment_statement | empty
	ASTNode statement();
	//variable : ID
	ASTNode variable();
	//statement_list : statement | statement ENDL statement_list
	ASTNode statement_list();
	//empty statement
	// ASTNode empty();
	//write() arguments list
	// write : write LPAREN variable RPAREN
	ASTNode write();
	//read() takes only one argument, takes input from console and store it in that variable
	//read : read LPAREN expr() RPAREN
	ASTNode read();
    ASTNode strlen();
    ASTNode _strcmp();
	//conditional operator : left OPERATOR right
	ASTNode conditional();
	//if : IF LPAREN Conditional RPAREN ( ( LBACKET statement_list RBRACKET ) | statement ) (EMPTY | ELSE ( ( LBACKET statement_list RBRACKET ) | statement ) )
	ASTNode If();
	//while : WHILE LPAREN conditional RPAREN ( ( LBACKET statement_list RBRACKET ) | statement )
	ASTNode While();

	ASTNode parse()
	{
		ASTNode node = statement_list();
		return node;
	}	
};

void Parser::eat(std::string token_type)
{
	// std::cout<<"Eat called\n";
	if(current_token._type() == token_type)
		current_token = lexer.get_next_token();
	else
	{
		std::cout<<"Error when eat() called\n";
		std::cout<<"current_token.type = "<<current_token._type()<<"     token_type = "<<token_type<<"\n";
		error();
	}
}


ASTNode Parser::statement()
{
	ASTNode node;
	if(current_token._type() == VARIABLE)
	{
		node = assignment_statement();
		eat(SEMI);
	}
	else if(current_token._type() == WRITE)
	{
		node = write();
		eat(SEMI);
	}
	else if(current_token._type() == READ)
	{
		node = read();
		eat(SEMI);
	}
    else if(current_token._type() == STRLEN)
	{
		node = strlen();
		eat(SEMI);
	}
    else if(current_token._type() == STRCMP)
	{
		node = _strcmp();
		eat(SEMI);
	}
	else if(current_token._type() == IF)
		node = If();
	else if(current_token._type() == WHILE)
		node = While();
	else error(); // some unknown kind of statement
	return node;
}

ASTNode Parser::assignment_statement()
{
	ASTNode left = variable();
	eat(ASSIGN);
	ASTNode right = expr();
	ASTNode node(Token(ASSIGN, "="));
	node.make_child(left);
	node.make_child(right);
	return node;
}

ASTNode Parser::variable()
{
	ASTNode node(current_token);
	eat(VARIABLE);
	return node;
}

ASTNode Parser::statement_list()
{
	ASTNode node(Token("STATEMENT_LIST", "STATEMENT_LIST"));
	while(current_token._type() != "EOF" and current_token._type() != RBRACKET)
	{
		node.make_child(statement());
	}
	return node;
}

ASTNode Parser::factor()
{
	if(current_token._type() == INTEGER)
	{
		ASTNode node(current_token);
		eat(INTEGER);
		return node;
	}
	else if(current_token._type() == LPAREN)
	{
		eat(LPAREN);
		ASTNode node = expr();
		eat(RPAREN);
		return node;
	}
	else if(current_token._type() == PLUS)
	{
		ASTNode node(current_token);
		eat(PLUS);
		node.make_child(factor());
		return node;
	}
	else if(current_token._type() == MINUS)
	{
		ASTNode node(current_token);
		eat(MINUS);
		node.make_child(factor());
		return node;
	}
	else if(current_token._type() == VARIABLE)
	{
		ASTNode node(variable());
		return node;
	}
	else if(current_token._type() == STRING)
	{
		ASTNode node(current_token);
		eat(STRING);
		return node;
	}
	else if(current_token._type() == ENDL)
	{
		ASTNode node(current_token);
		eat(ENDL);
		return node;
	}
    else if(current_token._type() == STRLEN)
	{
		ASTNode node = strlen();
        return node;
	}
    else if(current_token._type() == STRCMP)
	{
		ASTNode node = _strcmp();
		return node;
	}
	else error();
}

ASTNode Parser::term()
{
	ASTNode node = factor();
	while(true)
	{		
		ASTNode temp;
		if(current_token._type() == MUL)
		{
			eat(MUL);
			temp = ASTNode(Token(MUL, "*"));
		}
		else if(current_token._type() == DIV)
		{
			eat(DIV);
			temp = ASTNode(Token(DIV, "/"));
		}
		else if(current_token._type() == MOD)
		{
			eat(MOD);
			temp = ASTNode(Token(MOD, "%"));
		}
		else break;
		//child[1] holds value to be multiplied/divided from child[0]
		temp.make_child(node);
		temp.make_child(factor());
		node = temp;
	}
	return node;
}

ASTNode Parser::expr()
{
	ASTNode node = term();
	while(true)
	{
		ASTNode temp;
		if(current_token._type() == PLUS)
		{
			eat(PLUS);
			temp = ASTNode(Token(PLUS, "+"));
		}
		else if(current_token._type() == MINUS)
		{
			eat(MINUS);
			temp = ASTNode(Token(MINUS, "-"));
		}
		else break;
		//child[1] holds value to be added/subtracted from child[0]
		temp.make_child(node);
		temp.make_child(term());
		node = temp;
	}
	return node;
}

ASTNode Parser::write()
{
	eat(WRITE);
	eat(LPAREN);
	ASTNode node(Token(WRITE, WRITE));
	node.make_child(expr()); //child[0] holds expr() to be printed
	eat(RPAREN);
	return node;
}

ASTNode Parser::read()
{
	eat(READ);
	eat(LPAREN);
	ASTNode node(Token(READ, READ));
	node.make_child(variable()); //child[0] holds variable whose value is to be taken as input
	eat(RPAREN);
	return node;
}

ASTNode Parser::strlen()
{
	eat(STRLEN);
	eat(LPAREN);
	ASTNode node(Token(STRLEN, STRLEN));
	node.make_child(variable()); //child[0] holds variable whose value is to be taken as input
	eat(COMMA);
    node.make_child(variable());
    eat(RPAREN);

	return node;
}

ASTNode Parser::_strcmp()
{
	eat(STRCMP);
	eat(LPAREN);
	ASTNode node(Token(STRCMP, STRCMP));
	node.make_child(variable()); 
	eat(COMMA);
    node.make_child(variable());
    eat(COMMA);
    node.make_child(variable());
    eat(RPAREN);

	return node;
}

ASTNode Parser::If()
{
	//child[0] holds conditional statement, and child[1] holds statement_list to be executed if conditional returns true
	//child[2] stores ELSE part
	eat(IF);
	eat(LPAREN);
	ASTNode node(Token(IF, IF));
	node.make_child(conditional());
	eat(RPAREN);
	if(current_token._type() == LBRACKET)
	{
		eat(LBRACKET);
		node.make_child(statement_list());
		eat(RBRACKET);
	}
	else node.make_child(statement());
	//if ELSE part is present, then make it child, otherwise make empty statement as ELSE part
	if(current_token._type() == ELSE)
	{
		eat(ELSE);
		if(current_token._type() == LBRACKET)
		{
			eat(LBRACKET);
			node.make_child(statement_list());
			eat(RBRACKET);
		}
		else node.make_child(statement());
	}
	else node.make_child(Token(EMPTY, EMPTY));
	return node;
}

ASTNode Parser::conditional()
{
	//child[0] holds Left Hand Side of conditional
	//child[1] holds Right Hand Side of conditional
	ASTNode node;
	ASTNode left = expr();
	if(current_token._type() == EQUALITY)
	{
		node = Token(EQUALITY, EQUALITY);
		eat(EQUALITY);
	}
	else if(current_token._type() == LESSTHAN)
	{
		node = Token(LESSTHAN, LESSTHAN);
		eat(LESSTHAN);
	}
	else if(current_token._type() == MORETHAN)
	{
		node = Token(MORETHAN, MORETHAN);
		eat(MORETHAN);
	}
	else error();
	node.make_child(left);
	node.make_child(expr());
	return node;
}

ASTNode Parser::While()
{
	//child[0] holds conditional statement, and child[1] holds statement_list to be executed while conditional returns true
	eat(WHILE);
	eat(LPAREN);
	ASTNode node(Token(WHILE, WHILE));
	node.make_child(conditional());
	eat(RPAREN);
	if(current_token._type() == LBRACKET)
	{
		eat(LBRACKET);
		node.make_child(statement_list());
		eat(RBRACKET);
	}
	else node.make_child(statement());
	return node;
}





////////////////////////////////////////////////////////////////////////////////////////////
//	INTERPRETER
////////////////////////////////////////////////////////////////////////////////////////////

class Interpreter
{
private:
	Parser parser;
	
public:
	Interpreter(Parser _parser)
	{
		parser = _parser;
	}
	~Interpreter(){};
	
	Token visit(ASTNode node)
	{
		std::string result;

		if(node._token()._type() == STATEMENT_LIST)
		{
			for(auto it = node.child.begin(); it != node.child.end(); it++)
			{
				Token token = visit(**it);
				if(token._value() != "0")
					error("Failed to interpret statement:  type = "+token._type()+"  value = "+token._value());
			}
			return (Token(INTEGER, "0"));
		}

        if(node._token()._type() == EMPTY)
		{
			return Token(INTEGER, "0");
		}
        
        // variable assignment
        // @todo variable at rhs
		if(node._token()._type() == ASSIGN)
		{
            
            printf("; ASSIGN START\n");            

            // check if it already exist
            auto it = GLOBAL_SCOPE.find(node.child[0]->_token()._value());

            // if it does not already exist
            // create new
            if(it == GLOBAL_SCOPE.end()) { 

                GLOBAL_SCOPE[node.child[0]->_token()._value()] = node.child[1]->_token()._value();
                VARIABLE_TYPES[node.child[0]->_token()._value()] = node.child[1]->_token()._type();            
                VARIABLE_IDX[node.child[0]->_token()._value()] = variable_idx;
                
                // if rhs is string
                if(node.child[1]->_token()._type() == STRING) {
                    printf("; create string variable at memory location '%d'\n", variable_idx);
                    printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\n", node.child[1]->_token()._value().c_str());
                    printf("push %d ; address\nputs\n\n", variable_idx);
                }
                
                // if rhs is integer
                else if(node.child[1]->_token()._type() == INTEGER) {
                    printf("; create INTEGER variable at memory location '%d'\n", variable_idx);
                    printf("int 2\nint 10\npush %s\npush 1\npush %d ; address\nputs\n", node.child[1]->_token()._value().c_str(), variable_idx);
                }
                
                // if rhs is variable
                else if(node.child[1]->_token()._type() == VARIABLE) {  
                
                    // check type
                    if(VARIABLE_TYPES[node.child[1]->_token()._value()] == INTEGER) {
                        
                        printf("; create INTEGER variable at memory location '%d'\n", variable_idx);
                        printf("int 2\nint 10\npush %s\npush 1\npush %d ; address\nputs\n", GLOBAL_SCOPE[node.child[1]->_token()._value()].c_str(), variable_idx);
                        
                    }
                        
                    else if(VARIABLE_TYPES[node.child[1]->_token()._value()] == STRING) {
                        
                        printf("; create string variable at memory location '%d'\n", variable_idx);
                        printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\n", GLOBAL_SCOPE[node.child[1]->_token()._value()].c_str());
                        printf("push %d ; address\nputs\n\n", variable_idx);
                    
                    } 
                
                }
                    
                variable_idx++;
            
            } else {
            
                 if(node.child[1]->_token()._type() == STRING) {
                    printf("; assign existing string variable at memory location '%d'\n", VARIABLE_IDX[node.child[0]->_token()._value()]); 
                    printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\n", node.child[1]->_token()._value().c_str());
                    printf("push %d ; address\nputs\n\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                }
                
                else  if(node.child[1]->_token()._type() == INTEGER) {
                    printf("; assign existing integer variable at memory location '%d'\n", VARIABLE_IDX[node.child[0]->_token()._value()]); 
                    printf("int 2\nint 10\n push %s\npush 1 ; length\npush %d\nputs\n", node.child[1]->_token()._value().c_str(), VARIABLE_IDX[node.child[0]->_token()._value()]);
                }
                
                // if rhs is variable
                else if(node.child[1]->_token()._type() == VARIABLE) {

                    // check type
                    if(VARIABLE_TYPES[node.child[1]->_token()._value()] == INTEGER) {
                        
                        printf("; create INTEGER variable at memory location '%d'\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                        printf("int 2\nint 10\npush %s\npush 1\npush %d ; address\nputs\n", node.child[1]->_token()._value().c_str(), VARIABLE_IDX[node.child[0]->_token()._value()]);
                        
                    }
                        
                    else if(VARIABLE_TYPES[node.child[1]->_token()._value()] == STRING) {
                        
                        printf("; create string variable at memory location '%d'\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                        printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\n", node.child[1]->_token()._value().c_str());
                        printf("push %d ; address\nputs\n\n", variable_idx);
                    
                    }
                
                }
                
            }

            printf("; ASSIGN END\n\n");
            
            return (Token(INTEGER, "0"));
            
		}

		if(node._token()._type() == VARIABLE)
		{

            auto it = GLOBAL_SCOPE.find(node._token()._value());
			if(it == GLOBAL_SCOPE.end())
				error("Garbage value in "+node._token()._value());
			
            auto i = VARIABLE_TYPES.find(node._token()._value());
			if(i == GLOBAL_SCOPE.end())
				error("Error while determining datatype of "+node._token()._value());         
            
            if(i->second == INTEGER) {
            
                printf("; variable '%s' (INTEGER) at memory location %d\n", node._token()._value().c_str(), VARIABLE_IDX[node._token()._value()]);
                printf("int 2\nint 10\npush %d\ngets\n\n", VARIABLE_IDX[node._token()._value()]);
            
            }
            else if(i->second == STRING) { 
            
                printf("; variable '%s' (STRING) at memory location %d\nint 1\nint 9\npush %d\ngets\n\n", node._token()._value().c_str(), VARIABLE_IDX[node._token()._value()], VARIABLE_IDX[node._token()._value()]);
            
            }
                   
            if(i->second == INTEGER)
				return Token(INTEGER, it->second);  
			else if(i->second == STRING)
				return Token(STRING, it->second);
   
		}

		if(node._token()._type() == INTEGER)
		{

            // push it on the stack
            printf("; integer literal\n");           
            printf("int 2\nint 10\n");
            printf("push %s\n\n", node._token()._value().c_str());
           
            return node._token();
            
		}

		if(node._token()._type() == STRING)
		{

            // push the string on the stack
            printf("; string literal\n");            
            printf("int 1\nint 9\n");
            printf("push \"%s\"\n\n", node._token()._value().c_str());
            
            return node._token();
            
		}

		//Unary plus or binary plus
		if(node._token()._type() == PLUS)
		{

			printf("; ADDITION START\n");
            
            Token temp1, temp2;
			temp1 = visit(*(node.child[0]));
			temp2 = visit(*(node.child[1]));

            printf("pop bx\npop ax\nadd ax bx\npush ax\n\n");
                
			return Token(INTEGER, "0"); //return Token(INTEGER, result);
            
		}
		
		//Unary minus or binary minus
		if(node._token()._type() == MINUS)
		{
			
            printf("; SUBTRACION START\n");
            
            Token temp1, temp2;
			temp1 = visit(*(node.child[0]));
			temp2 = visit(*(node.child[1]));
            
            printf("pop bx\npop ax\nsub ax bx\npush ax\n");
                
            printf("; SUBTRACION END\n");
                
			return Token(INTEGER, "0");//return Token(INTEGER, result);

		}

		if(node._token()._type() == MUL)
		{
			           
            printf("; MULTIPLICATION START\n");
            
            Token temp1, temp2;
			temp1 = visit(*(node.child[0]));
			temp2 = visit(*(node.child[1]));
			/*
            std::string str1 = temp1._value();
			std::string str2 = temp2._value();
			//If both operands are integer only then do integer multiplication and return without decimals
			if(str1.find('.') == std::string::npos and str2.find('.') == std::string::npos)
				result = std::to_string(std::stoi(temp1._value()) * std::stoi(temp2._value()));
			//if any of th operands is double type, then return with decimal digits
			else result = std::to_string(std::stod(temp1._value()) * std::stod(temp2._value()));
			return Token(INTEGER, result);
            */
            
            printf("pop bx\npop ax\nmul ax bx\npush ax\n");
            
            printf("; MULTIPLICATION END\n");
            
            return Token(INTEGER, "0");
            
		}
		
		if(node._token()._type() == DIV)
		{
        
            printf("; division\n");
             
			Token temp1, temp2;
			temp1 = visit(*(node.child[0]));
			temp2 = visit(*(node.child[1]));
            
            /*
			std::string str1 = temp1._value();
			std::string str2 = temp2._value();
			//If both operands are integer only then do integer division and return quotient of division
			if(str1.find('.') == std::string::npos and str2.find('.') == std::string::npos)
				result = std::to_string(std::stoi(temp1._value()) / std::stoi(temp2._value()));
			//if any of th operands is double type, do double division
			else result = std::to_string(std::stod(temp1._value()) / std::stod(temp2._value()));
			return Token(INTEGER, result);
            */
            
            printf("pop bx\npop ax\ndiv ax bx\npush ax\n\n");
            
            return Token(INTEGER, "0");
            
		}
		
        // @todo
		if(node._token()._type() == MOD)
		{
			Token temp1, temp2;
			temp1 = visit(*(node.child[0]));
			temp2 = visit(*(node.child[1]));
			result = std::to_string(std::stoi(temp1._value()) % std::stoi(temp2._value()));
			return Token(INTEGER, result);
		}

        // can be equality, less than or more than
		if(node._token()._type() == IF)
		{

            std::string funcName = "conditional_statement_" + std::to_string(label_idx++);
            printf("%s:\n\n", funcName.c_str());
            
            // condition get pushed onto the stack
            Token token = visit(*node.child[0]);

            printf("jnz %s_else\n\n" , funcName.c_str());
            
            // visit the condition if true
            visit(*node.child[1]);
            
            // jmp to end
            printf("jmp %s_end\n\n", funcName.c_str());
     
            printf("%s_else:\n\n", funcName.c_str());
            
            // visit the else node
            visit(*node.child[2]);
            
            printf("%s_end:\n\n", funcName.c_str());
            
			return Token(INTEGER, "0"); //return temp;
            
		}
        
        // can be equality, less than or more than
		if(node._token()._type() == WHILE)
		{

            std::string funcName = "conditional_statement_" + std::to_string(label_idx++);
            
            printf("%s:\n\n", funcName.c_str());

            //printf("%s_loop:\n\n", funcName.c_str());
            
            visit(*node.child[1]);
            
            printf("; body executed..\n\n");
            
            visit(*node.child[0]);
            
            printf("jz %s\n\n", funcName.c_str());
            //printf("jz %s_loop\n\n", funcName.c_str());

			return Token(INTEGER, "0");
            
		}

        // good 
		if(node._token()._type() == EQUALITY)
		{
        
			// No need to visit right now
            Token token0; // = visit(*node.child[0]);
			Token token1; // = visit(*node.child[1]);
            
            int mode = 0; // rw bytes

            /*                     
            string literal at -1            
            */
            if(node.child[0]->_token()._type() == STRING) {
                // push it on the stack and store at -1
                printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\npush -1 ; address\nputs\n\n", node.child[0]->_token()._value().c_str());                
                // get first character and store again but as int at -1
                printf("push -1\npush 0\nldm\nint 2 ; rw_int\npush 1 ; length\npush -1 ; address\nputs\n\n");                
            }
            /*
            integer literal at -1
            */
            else if(node.child[0]->_token()._type() == INTEGER) {
            
                  // push it on the stack and store at -1
                  printf("int 2\nint 10\npush %s\npush 1\npush -1\nputs\n\n", node.child[0]->_token()._value().c_str());
                  mode = 1;
                  
            }
            
            /*                     
            variable at -1
            */ 
            else if(node.child[0]->_token()._type() == VARIABLE) {    

                if(VARIABLE_TYPES[node.child[0]->_token()._value()] == INTEGER) {
                    
                    // get it on the stack and store at -1
                    printf("int 2\nint 10\npush %d\ngets\npush 1\npush -1\nputs\n\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                    
                    // no need to convert
                    //printf("push -1\ngets\npush 1 ; length\npush -1 ; address\nputs\n\n");
                    
                    mode = 1;
                    
                }
                    
                else if(VARIABLE_TYPES[node.child[0]->_token()._value()] == STRING) {
                    
                    // get it on the stack and store at -1
                    printf("int 1\nint 9\nsi ax\npush %d ; address\ngets\nsi bx\nsub bx ax\npush bx\npush -1 ; address\nputs\n\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                    
                    // get first character and store again but as int at -1
                    printf("push -1\npush 0\nldm\nint 2 ; rw_int\nint 10\npush 1 ; length\npush -1 ; address\nputs\n\n");
                
                }
                
            }
            
            /*                     
            string literal at -2           
            */
            if(node.child[1]->_token()._type() == STRING) {
                
                printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\npush -2\nputs\n\n", node.child[1]->_token()._value().c_str());
                printf("push -2\npush 0\nldm\npush 1 ; length\npush -2 ; address\nint 2 ; rw_int\nputs\n\n");
                
            }
            
            /*
            integer literal at -2
            good
            */
            else if(node.child[1]->_token()._type() == INTEGER) {
                printf("int 2\nint 10\npush %s\npush 1\npush -2\nputs\n\n", node.child[1]->_token()._value().c_str());
                mode = 1;
            }
            
            /*                     
            variable at -2            
            */
            else if(node.child[1]->_token()._type() == VARIABLE) {    
                
                // good
                if(VARIABLE_TYPES[node.child[1]->_token()._value()] == INTEGER) {
                    printf("int 2\nint 10\npush %d\ngets\npush 1\npush -2\nputs\n\n", VARIABLE_IDX[node.child[1]->_token()._value()]);
                    printf("push -2\ngets\npush 1 ; length\npush -2 ; address\nputs\n\n");
                    mode = 1;
                }
                    
                else if(VARIABLE_TYPES[node.child[1]->_token()._value()] == STRING) {
                    printf("int 1\nint 9\nsi ax\npush %d ; address\ngets\nsi bx\nsub bx ax\npush bx\npush -2 ; store at -2\nputs\n\n", VARIABLE_IDX[node.child[1]->_token()._value()]);
                    printf("push -2\npush 0\nldm\npush 1 ; length\npush -2 ; address\nint 2 ; rw_int\nputs\n\n");
                }
                
            }
            
            // compare in memory
            //printf("push -1\npush -2\ncmp\n\n");
            
		    // compare on the stack
            printf("int 2\nint 10\npush -1\ngets\npush -2\ngets\neq\n");
            
            return Token(INTEGER, "0");
            
		}

        // good
		if(node._token()._type() == LESSTHAN)
		{
        
            // No need to visit right now
            Token token0; // = visit(*node.child[0]);
			Token token1; // = visit(*node.child[1]);
            
            int mode = 0; // rw bytes

            /*                     
            string literal at -1            
            */
            if(node.child[0]->_token()._type() == STRING) {
                // push it on the stack and store at -1
                printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\npush -1 ; address\nputs\n\n", node.child[0]->_token()._value().c_str());                
                // get first character and store again but as int at -1
                printf("push -1\npush 0\nldm\nint 2 ; rw_int\npush 1 ; length\npush -1 ; address\nputs\n\n");                
            }
            /*
            integer literal at -1
            */
            else if(node.child[0]->_token()._type() == INTEGER) {
            
                  // push it on the stack and store at -1
                  printf("int 2\nint 10\npush %s\npush 1\npush -1\nputs\n\n", node.child[0]->_token()._value().c_str());
                  mode = 1;
                  
            }
            
            /*                     
            variable at -1
            */ 
            else if(node.child[0]->_token()._type() == VARIABLE) {    

                if(VARIABLE_TYPES[node.child[0]->_token()._value()] == INTEGER) {
                    
                    // get it on the stack and store at -1
                    printf("int 2\nint 10\npush %d\ngets\npush 1\npush -1\nputs\n\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                    
                    // no need to convert
                    //printf("push -1\ngets\npush 1 ; length\npush -1 ; address\nputs\n\n");
                    
                    mode = 1;
                    
                }
                    
                else if(VARIABLE_TYPES[node.child[0]->_token()._value()] == STRING) {
                    
                    // get it on the stack and store at -1
                    printf("int 1\nint 9\nsi ax\npush %d ; address\ngets\nsi bx\nsub bx ax\npush bx\npush -1 ; address\nputs\n\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                    
                    // get first character and store again but as int at -1
                    printf("push -1\npush 0\nldm\nint 2 ; rw_int\npush 1 ; length\npush -1 ; address\nputs\n\n");
                
                }
                
            }
            
            /*                     
            string literal at -2           
            */
            if(node.child[1]->_token()._type() == STRING) {
                
                printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\npush -2\nputs\n\n", node.child[1]->_token()._value().c_str());
                printf("push -2\npush 0\nldm\npush 1 ; length\npush -2 ; address\nint 2 ; rw_int\nputs\n\n");
                
            }
            
            /*
            integer literal at -2
            good
            */
            else if(node.child[1]->_token()._type() == INTEGER) {
                printf("int 2\nint 10\npush %s\npush 1\npush -2\nputs\n\n", node.child[1]->_token()._value().c_str());
                mode = 1;
            }
            
            /*                     
            variable at -2            
            */
            else if(node.child[1]->_token()._type() == VARIABLE) {    
                
                // good
                if(VARIABLE_TYPES[node.child[1]->_token()._value()] == INTEGER) {
                    printf("int 2\nint 10\npush %d\ngets\npush 1\npush -2\nputs\n\n", VARIABLE_IDX[node.child[1]->_token()._value()]);
                    printf("push -2\ngets\npush 1 ; length\npush -2 ; address\nputs\n\n");
                    mode = 1;
                }
                    
                else if(VARIABLE_TYPES[node.child[1]->_token()._value()] == STRING) {
                    printf("int 1\nint 9\nsi ax\npush %d ; address\ngets\nsi bx\nsub bx ax\npush bx\npush -2 ; store at -2\nputs\n\n", VARIABLE_IDX[node.child[1]->_token()._value()]);
                    printf("push -2\npush 0\nldm\npush 1 ; length\npush -2 ; address\nint 2 ; rw_int\nputs\n\n");
                }
                
            }
            
            // compare in memory
            //printf("push -1\npush -2\ncmp\n\n");
            
		    // compare on the stack
            printf("int 2\nint 10\npush -2\ngets\npush -1\ngets\nlt\n");
            
            return Token(INTEGER, "0");
            
		}

        // good
		if(node._token()._type() == MORETHAN)
		{
        
            // No need to visit right now
            Token token0; // = visit(*node.child[0]);
			Token token1; // = visit(*node.child[1]);
            
            int mode = 0; // rw bytes

            /*                     
            string literal at -1            
            */
            if(node.child[0]->_token()._type() == STRING) {
                // push it on the stack and store at -1
                printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\npush -1 ; address\nputs\n\n", node.child[0]->_token()._value().c_str());                
                // get first character and store again but as int at -1
                printf("push -1\npush 0\nldm\nint 2 ; rw_int\npush 1 ; length\npush -1 ; address\nputs\n\n");                
            }
            /*
            integer literal at -1
            */
            else if(node.child[0]->_token()._type() == INTEGER) {
            
                  // push it on the stack and store at -1
                  printf("int 2\nint 10\npush %s\npush 1\npush -1\nputs\n\n", node.child[0]->_token()._value().c_str());
                  mode = 1;
                  
            }
            
            /*                     
            variable at -1
            */ 
            else if(node.child[0]->_token()._type() == VARIABLE) {    

                if(VARIABLE_TYPES[node.child[0]->_token()._value()] == INTEGER) {
                    
                    // get it on the stack and store at -1
                    printf("int 2\nint 10\npush %d\ngets\npush 1\npush -1\nputs\n\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                    
                    // no need to convert
                    //printf("push -1\ngets\npush 1 ; length\npush -1 ; address\nputs\n\n");
                    
                    mode = 1;
                    
                }
                    
                else if(VARIABLE_TYPES[node.child[0]->_token()._value()] == STRING) {
                    
                    // get it on the stack and store at -1
                    printf("int 1\nint 9\nsi ax\npush %d ; address\ngets\nsi bx\nsub bx ax\npush bx\npush -1 ; address\nputs\n\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                    
                    // get first character and store again but as int at -1
                    printf("push -1\npush 0\nldm\nint 2 ; rw_int\npush 1 ; length\npush -1 ; address\nputs\n\n");
                
                }
                
            }
            
            /*                     
            string literal at -2           
            */
            if(node.child[1]->_token()._type() == STRING) {
                
                printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\npush -2\nputs\n\n", node.child[1]->_token()._value().c_str());
                printf("push -2\npush 0\nldm\npush 1 ; length\npush -2 ; address\nint 2 ; rw_int\nputs\n\n");
                
            }
            
            /*
            integer literal at -2
            good
            */
            else if(node.child[1]->_token()._type() == INTEGER) {
                printf("int 2\nint 10\npush %s\npush 1\npush -2\nputs\n\n", node.child[1]->_token()._value().c_str());
                mode = 1;
            }
            
            /*                     
            variable at -2            
            */
            else if(node.child[1]->_token()._type() == VARIABLE) {    
                
                // good
                if(VARIABLE_TYPES[node.child[1]->_token()._value()] == INTEGER) {
                    printf("int 2\nint 10\npush %d\ngets\npush 1\npush -2\nputs\n\n", VARIABLE_IDX[node.child[1]->_token()._value()]);
                    printf("push -2\ngets\npush 1 ; length\npush -2 ; address\nputs\n\n");
                    mode = 1;
                }
                    
                else if(VARIABLE_TYPES[node.child[1]->_token()._value()] == STRING) {
                    printf("int 1\nint 9\nsi ax\npush %d ; address\ngets\nsi bx\nsub bx ax\npush bx\npush -2 ; store at -2\nputs\n\n", VARIABLE_IDX[node.child[1]->_token()._value()]);
                    printf("push -2\npush 0\nldm\npush 1 ; length\npush -2 ; address\nint 2 ; rw_int\nputs\n\n");
                }
                
            }
            
            // compare in memory
            //printf("push -1\npush -2\ncmp\n\n");
            
		    // compare on the stack
            printf("int 2\nint 10\npush -2\ngets\npush -1\ngets\ngt\n");
            
            return Token(INTEGER, "0");
            
		}
        
        // good
        if(node._token()._type() == WRITE)
		{
        
			printf("; WRITE START\n");
         
            if(node.child[0]->_token()._type() == VARIABLE) {             

                if(VARIABLE_TYPES[node.child[0]->_token()._value()] == STRING) {

                     printf("push %d\nwrite\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                     
                }    
                    
                else if(VARIABLE_TYPES[node.child[0]->_token()._value()] == INTEGER) {
                
                    printf("int 2\nint 10\npush %d\ngets\npush 100\nprint\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
                    
                } 
                               
            }
            
            else if(node.child[0]->_token()._type() == STRING) 
                printf("int 1\nint 9\nsi ax\npush \"%s\"\nsi bx\nsub bx ax\npush bx\npush -1\nputs\npush -1\nwrite\n", node.child[0]->_token()._value().c_str());

            else if(node.child[0]->_token()._type() == INTEGER)
                printf("push %s\npush 'd'\nprint\n", node.child[0]->_token()._value().c_str());
                
            printf("; WRITE END\n\n"); 
               
			return Token(INTEGER, "0");
            
		}
        
        if(node._token()._type() == STRLEN)
		{

            printf("; STRLEN START\n");
            
            // @todo store copy at address -1
            std::string funcName = "strlen_statement_" + std::to_string(label_idx++);
            
            // add null terminator!
            printf("int 1\nint 9\nsi ax\npush 0\npush %d\ngets\nsi bx\nsub bx ax\npush bx\npush %d\nputs\n\n", VARIABLE_IDX[node.child[0]->_token()._value()], VARIABLE_IDX[node.child[0]->_token()._value()]);
            
            printf("; get the length of string \"%s\" and store it in variable \"%s\"\n", node.child[0]->_token()._value().c_str(), node.child[1]->_token()._value().c_str());
            printf("int 1 ; rw_char\n");
            printf("int 9 ; arith_char\n");
            printf("mov r3 %d ; get address of result\n", VARIABLE_IDX[node.child[1]->_token()._value()]);
            printf("mov r1 %d; get address of variable\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
            printf("mov r2 0 ; position of current character, start at position 0\n\n");
            
            printf("%s:\n\n", funcName.c_str());
            
            printf("ldr r1 ; address of variable \"%s\"\n", node.child[0]->_token()._value().c_str());
            printf("ldr r2 ; position of character\n");
            printf("ldm\n");
            printf("push 0 ; 0 marks the end of the string\n");
            printf("eq\n");
            printf("jz %s_end\n", funcName.c_str());
            printf("inc r2\n");
            printf("jmp %s\n\n", funcName.c_str());
            
            printf("%s_end:\n\n", funcName.c_str());
		    printf("ldr r2 ; the length\n");
            printf("push 1\n");
            printf("push r3 ; address to store result\n", VARIABLE_IDX[node.child[1]->_token()._value()]); // addr
            printf("int 2\nint 10\nputs\n"); // addr
            //printf("    int 4\n");
            
            printf("; STRLEN END\n\n");
            
			return Token(INTEGER, "0");
            
		}
        
        if(node._token()._type() == STRCMP)
		{

            // compare 0 and 1
            // store result in 3
            printf("; STRCMP START\n");
            
            std::string funcName = "strcmp_statement_" + std::to_string(label_idx++);
            printf("%s:\n", funcName.c_str());
            
            //printf("int 1\nint 9\n");
            printf("push %d ; address of string 1\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
            printf("push %d ; address of string 2\n", VARIABLE_IDX[node.child[1]->_token()._value()]);
            
            // cmp will set zeroflag
            printf("cmp\n");
               
            printf("jnz %s_false\n", funcName.c_str());
            
            // if we came here, it was true
            printf("int 2\nint 10\n");
            printf("push 1\npush 1\npush %d ; address of string 3\nputs\n\n", VARIABLE_IDX[node.child[2]->_token()._value()]);
            printf("jmp %s_end\n", funcName.c_str());
            
            // we only come here if it was false
            printf("%s_false:\n", funcName.c_str());
            // store 0 in variable 3
            printf("int 2\nint 10\n");
            printf("push 0\npush 1\npush %d ; address of string 3\n", VARIABLE_IDX[node.child[2]->_token()._value()]);
            printf("puts\n\n");

            printf("%s_end:\n\n", funcName.c_str());
            
            // store result in ax
            //printf("; leave result in ax\nint 2\nint 10\npush %d\ngets\npop ax\n", VARIABLE_IDX[node.child[2]->_token()._value()]);
            
            printf("; STRCMP END\n\n");
            
			return Token(INTEGER, "0");
            
		}

		if(node._token()._type() == READ)
		{

            //printf("<READ '%s' at memory location '%d'>\n", node.child[0]->_token()._value().c_str(), VARIABLE_IDX[node.child[0]->_token()._value()]);
            
            //printf("; read input into memory\npush %d ; address\nstr r1\nread\n\n; remove trailing null terminator\nsi ax\nldr r1\ngets\nsi bx\nsub bx ax\ndec bx\nldr bx\nldr r1\nputs\n\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
            printf("; read input into memory\n");
            printf("push %d ; address\nstr r1\nread\n\n; remove trailing null terminator\nsi ax\nldr r1\ngets\nsi bx\nsub bx ax\nldr bx\nldr r1\nputs\n\n", VARIABLE_IDX[node.child[0]->_token()._value()]);
            
			return Token(INTEGER, "0");
            
		}

		if(node._token()._type() == ENDL)
		{
			return Token(STRING, "\n");
		}
        
	}

	Token interpret()
	{
		ASTNode tree = parser.parse();
		return visit(tree);
	}

	void error(std::string str)
	{
		std::cout<<"Interpreter error: "<<str<<"\n";
		exit(0);
	}
};



////////////////////////////////////////////////////////////////////////////////////////////
//	main
////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char const *argv[])
{
	// set reserved keywords
	RESERVED_KEYWORDS.insert("write");
	RESERVED_KEYWORDS.insert("read");
	RESERVED_KEYWORDS.insert("if");
	RESERVED_KEYWORDS.insert("else");
	RESERVED_KEYWORDS.insert("while");
	RESERVED_KEYWORDS.insert("endl");
    RESERVED_KEYWORDS.insert("strlen");
    RESERVED_KEYWORDS.insert("strcmp");
    
	// argv[1] stores name of file containing bird code
	// if argv[2] == "details" then show internal details of interpretation
	if(argc < 2)
	{
		std::cout<<"Please Enter name of file as commandline argument\n";
		exit(0);
	}

	std::string text, temp;
	std::ifstream file;
	file.open(argv[1]);

	while(std::getline(file, temp))
	{
		text += temp;
	}
	file.close();
	Lexer lexer(text);
	Parser parser(lexer);
	Interpreter interpreter(parser);

	printf("; source code generator v1.0 early alpha\n\n");
    Token result = interpreter.interpret();
    
    if(0) {
    
        ASTNode tree = parser.parse();
        tree.show(0);
        
        std::cout << "\n;# # # # #\n";
        
        for(auto it = GLOBAL_SCOPE.begin(); it!= GLOBAL_SCOPE.end(); it++) {
        
            std::cout << ";\t" << VARIABLE_IDX[it->first] << ": " << it->first << " (" << VARIABLE_TYPES[it->first] << ") " << it->second << "\n";
    
            std::cout << "\n";
            
        }
    
    } 

	return 0;
    
}