#ifndef AST_HH_INCLUDED
#define AST_HH_INCLUDED

#include <iostream>
#include <vector>
#include <utility>
#include "type.hh"

using namespace std;
class exp_astnode;
class identifier_astnode;

enum typeExp
{
	EmptyNode = 0,
	SeqNode = 1,
	AssNode,
	ReturnNode,
	IfNode,
	WhileNode,
	ForNode, // of statement kind
	OpBinaryNode,
	OpUnaryNode,
	AssignNode,
	FunCallNode,
	IntConstNode,
	FloatConstNode,
	StringConstNode,
	PointerNode,
	IdentifierNode,
	ArrayRefNode,
	DerefNode,
	MemberNode,
	ArrowNode
};
class funcall_astnode; //forward declaration
class abstract_astnode
{
public:
	virtual void print(int ntabs) = 0;
	enum typeExp astnode_type;

protected:
	std::string id;
};

class statement_astnode : public abstract_astnode
{
public:
	string typecast;
	virtual void print(int ntabs) = 0;
	// statement_astnode();
protected:
};

class exp_astnode : public abstract_astnode
{
public:
	bool lvalue;   
	string typecast;	// a string like "to_int etc
	datatype data_type; // the type of the expression
	bool is_calculable; // in case of constant folding
	int int_val;		// the folded integer
	virtual void print(int ntabs) = 0;
	string idname();
	int offset;
	int second_offset;
	// exp_astnode();
	int tag;
	int number;
	string word;
};

class ref_astnode : public exp_astnode
{
public:
	virtual void print(int ntabs) = 0;
	// ref_astnode();
};

/////////////////////////////////////////

class empty_astnode : public statement_astnode
{
public:
	empty_astnode();
	void print(int ntabs);
};

class seq_astnode : public statement_astnode
{
public:
	seq_astnode();
	void pushback(statement_astnode *newchild);
	void print(int ntabs);

protected:
	std::vector<statement_astnode *> children_nodes;
};

class assignS_astnode : public statement_astnode
{
public:
	assignS_astnode(exp_astnode *left, exp_astnode *right, string tc);
	void print(int ntabs);

protected:
	exp_astnode *left;
	exp_astnode *right;
};

class return_astnode : public statement_astnode
{
public:
	return_astnode(exp_astnode *child);
	void print(int ntabs);

protected:
	exp_astnode *child;
};

class if_astnode : public statement_astnode
{
public:
	if_astnode(exp_astnode *left, statement_astnode *middle, statement_astnode *right);
	void print(int ntabs);

protected:
	exp_astnode *left;
	statement_astnode *middle;
	statement_astnode *right;
};

class while_astnode : public statement_astnode
{
public:
	while_astnode(exp_astnode *left, statement_astnode *right);
	void print(int ntabs);

protected:
	exp_astnode *left;
	statement_astnode *right;
};

class for_astnode : public statement_astnode
{
public:
	for_astnode(exp_astnode *left, exp_astnode *middle1, exp_astnode *middle2, statement_astnode *right);
	void print(int ntabs);

protected:
	exp_astnode *left;
	exp_astnode *middle1;
	exp_astnode *middle2;
	statement_astnode *right;
};

class proccall_astnode : public statement_astnode
{
public:
	int lc_token;
	proccall_astnode(funcall_astnode *fc);
	void print(int ntabs);
	std::vector<exp_astnode *> children;
	proccall_astnode(funcall_astnode* c, int lccount);

protected:
	identifier_astnode *procname;
};

///////////////////////////////////////////////////////////////

class op_binary_astnode : public exp_astnode
{
public:
	op_binary_astnode(std::string val, exp_astnode *left, exp_astnode *right);
	void print(int ntabs);

protected:
	exp_astnode *left;
	exp_astnode *right;
};

class op_unary_astnode : public exp_astnode
{
public:
	op_unary_astnode(std::string val);
	op_unary_astnode(std::string val, exp_astnode *child);
	// op_unary_astnode(abstract_astnode* child1, abstract_astnode* child2);
	std::string getoperator();
	// void setchild(abstract_astnode* child);
	void print(int ntabs);

protected:
	exp_astnode *child;
};

class assignE_astnode : public exp_astnode
{
public:
	assignE_astnode(exp_astnode *left, exp_astnode *right); // earlier was ref_astnode*, now no concept of lvalue in grammar
	void print(int ntabs);
	exp_astnode *left; // change to protected later
	exp_astnode *right;
};

class funcall_astnode : public exp_astnode
{
public:
	funcall_astnode();
	funcall_astnode(identifier_astnode *child);
	void setname(std::string funcname);
	void pushback(exp_astnode *child);
	void print(int ntabs);
	std::vector<exp_astnode *> children;
	friend class proccall_astnode;
	int lc_token;
protected:
	identifier_astnode *funcname;
};

class intconst_astnode : public exp_astnode
{
public:
	intconst_astnode(int value);
	void print(int ntabs);
	// protected:
};

class floatconst_astnode : public exp_astnode
{
public:
	floatconst_astnode(float value);
	void print(int ntabs);
	// protected:
	float value;
};

class stringconst_astnode : public exp_astnode
{
public:
	stringconst_astnode(std::string value);
	void print(int ntabs);
	// protected:
	std::string value;
};

////////////////////////////////////////////////

class identifier_astnode : public ref_astnode
{
public:
	identifier_astnode(std::string val);
	void print(int ntabs);

};

// class pointer_astnode : public exp_astnode
// {
// public:
// 	pointer_astnode(ref_astnode *child);
// 	void print(int ntabs);

// protected:
// 	ref_astnode *child;
// };

class arrayref_astnode : public ref_astnode
{
public:
	arrayref_astnode(exp_astnode *left, exp_astnode *right);
	void print(int ntabs);

protected:
	exp_astnode *left;
	exp_astnode *right;
};

class deref_astnode : public ref_astnode
{
public:
	deref_astnode(ref_astnode *child);
	void print(int ntabs);

protected:
	ref_astnode *child;
};

class member_astnode : public ref_astnode
{
public:
	member_astnode(exp_astnode *left, identifier_astnode *right); // ref to exp
	void print(int ntabs);

protected:
	exp_astnode *left;
	identifier_astnode *right;
};

class arrow_astnode : public ref_astnode
{
public:
	arrow_astnode(exp_astnode *left, identifier_astnode *right); // ref to exp
	void print(int ntabs);

protected:
	exp_astnode *left;
	identifier_astnode *right;
};
#endif
void printblanks(int ntabs);
void printAst(const char *astname, const char *fmt...);
char *stringTocharstar(string str);
