#include "ast.hh"
#include <cstdarg>
#include <iostream>
#include <string>
#include <set>
#include<algorithm>

vector<string> stack {"%edi", "%esi", "%edx", "%ecx", "%ebx", "%eax"}; //  edi, esi, edx, ecx, ebx, eax
int  tokens = 2;
bool inMember = false;

void Display(exp_astnode* a, string mem) {
	if(a->astnode_type == ArrayRefNode || a->astnode_type == MemberNode){
		cout << "movl (" <<  mem << "), " << mem << endl; 	
	}
}

class CodeGenerator {
	public:

	void Print(string opcode) {
        cout << opcode << endl;
    }

    void Print(string opcode, string arg) {
        cout << opcode << " " << arg << endl;
    }

    void Print(string opcode, string arg1, string arg2) {
        cout << opcode << " " << arg1 << ", " << arg2 << endl;
    }

    void Compare(string op, string mem1, string mem2) {
        string setCode;
		if (op == "NE_OP_INT"){
            setCode = "jne";

			Print("cmpl", mem2, mem1);
			Print(setCode, ".L" + to_string(tokens++));
			Print("movl", "$0", mem1);
			Print("jmp", ".L" + to_string(tokens++));
			Print(".L" + to_string(tokens-2) + ":");
			Print("movl", "$1", mem1);
			Print(".L" + to_string(tokens-1) + ":");
			return;
		}
        else if (op == "LT_OP_INT")
            setCode = "setl %al";
        else if (op == "GT_OP_INT")
            setCode = "setg %al";
        else if (op == "LE_OP_INT")
            setCode = "setle %al";
        else if (op == "GE_OP_INT")
            setCode = "setge %al";
        else if (op == "EQ_OP_INT")
            setCode = "sete %al";

        Print("cmpl", mem2, mem1);
        Print(setCode);
        Print("movzbl %al,", mem1);
    }

    void And_OP(string mem1, string mem2) {
		Compare("NE_OP_INT", mem1, "$0");
		Compare("NE_OP_INT", mem2, "$0");
		Print("imul", mem2, mem1);
    }

    void Or_OP(string mem1, string mem2) {
		Compare("NE_OP_INT", mem1, "$0");
		Compare("NE_OP_INT", mem2, "$0");
		Print("addl", mem2, mem1);
		Compare("NE_OP_INT", mem1, "$0");
    }

    void Arithmetic(string op, string mem1, string mem2, exp_astnode* l) {
        string instruction;
		if(isPointer(l->data_type) || isArray(l->data_type)){
			if(l->data_type.type != 3){
				Print("imul", "$4", mem2);
			}
			else{
				Print("imul", "$"+to_string(l->data_type.struct_size()), mem2);
			}
		}
		;
        if (op == "PLUS_INT"){
            instruction = "addl";
		}
        else if (op == "MINUS_INT"){
            instruction = "subl";
		}
        else if (op == "MULT_INT"){
            instruction = "imul";
		}

        Print(instruction, mem2, mem1);
    }

	void generateDiv(string mem1, string mem2) {
		int a = tokens;
		tokens += 2;
		cout << "cmpl $0, " << mem1 << endl;
		cout << "jl  .L" << a << endl;
		cout << "    				  " << endl;
		cout << "jmp  .L" << a+1 << endl;
		cout << ".L" << a << " :" << endl;
		cout << "negl " << mem1 << endl;
		cout << "negl -4(%ebp)"<<endl;
		cout << ".L" << a+1 << " :" << endl;

		a = tokens;
		tokens += 2;
		cout << "cmpl $0, " << mem2 << endl;
		cout << "jl  .L" << a << endl;
		cout << "    				  " << endl;
		cout << "jmp  .L" << a+1 << endl;
		cout << ".L" << a << " :" << endl;
		cout << "negl " << mem2 << endl;
		cout << "negl -4(%ebp)"<<endl;
		cout << ".L" << a+1 << " :" << endl;

		cout << "movl $0, %edi"<< endl;
		a = tokens;
		tokens += 2;
		cout << "jmp  .L" << a << endl;
		cout << ".L" << a+1 << " :" << endl;
		cout << "subl " <<  mem2 << ", " << mem1 << endl;
		cout << "addl $1, %edi"<< endl;
		cout << ".L" << a << " :" << endl;
		cout << "cmpl  " <<  mem2 << ", " << mem1 << endl;
		cout << "jge  .L" << a+1 << endl;
		cout << "movl  %edi, " << mem1 << endl;
		cout << "imul " << "-4(%ebp)" << ", " << mem1 << endl;
		cout << "movl $1, -4(%ebp)" << endl;
	}
};




void Code(exp_astnode* l, exp_astnode* r, string op)
{
	if(l->tag < r->tag && l->tag < (int)stack.size())
	{
		CodeGenerator codeGenerator;
		
		string temp = stack.back();
		stack.back() = stack[stack.size()-2];
		stack[stack.size()-2] = temp;

		r->print(0);
		Display(r, stack.back());
		
		string Register = stack.back();
		stack.pop_back();

		l->print(0);
		Display(l, stack.back());
		string mem1 = stack.back();
        string mem2 = Register;

        if (op == "AND_OP") {
            codeGenerator.And_OP(mem1, mem2);
        } else if (op == "OR_OP") {
            codeGenerator.Or_OP(mem1, mem2);
        } else if (op == "LE_OP_INT" || op == "LT_OP_INT" || op == "GE_OP_INT" || op == "GT_OP_INT" || op == "NE_OP_INT" || op == "EQ_OP_INT"){
            codeGenerator.Compare(op, mem1, mem2);
        } else if (op == "DIV_INT") {
            codeGenerator.generateDiv(mem1, mem2);
        } else {
            codeGenerator.Arithmetic(op, mem1, mem2, l);
        }

		stack.push_back(Register);

		temp = stack.back();
		stack.back() = stack[stack.size()-2];
		stack[stack.size()-2] = temp;
	}

	else if(l->tag >= r->tag && r->tag < (int)stack.size() )
	{
		CodeGenerator codeGenerator;
		
		l->print(0);
		Display(l, stack.back());

		string Register = stack.back();
		stack.pop_back();

		r->print(0);
		Display(r, stack.back());
		
		string mem1 = Register;
        string mem2 = stack.back();

        if (op == "AND_OP") {
            codeGenerator.And_OP(mem1, mem2);
        } else if (op == "OR_OP") {
            codeGenerator.Or_OP(mem1, mem2);
        } else if (op == "LE_OP_INT" || op == "LT_OP_INT" || op == "GE_OP_INT" || op == "GT_OP_INT" || op == "NE_OP_INT" || op == "EQ_OP_INT"){
            codeGenerator.Compare(op, mem1, mem2);
        } else if (op == "DIV_INT") {
            codeGenerator.generateDiv(mem1, mem2);
        } else {
            codeGenerator.Arithmetic(op, mem1, mem2, l);
        }

		stack.push_back(Register);
	}
	
}
/////////////////////////////

empty_astnode::empty_astnode() : statement_astnode()
{
	astnode_type = EmptyNode;
}

void empty_astnode::print(int ntabs)
{
	cout << "		" << endl;
}

//////////////////////////

seq_astnode::seq_astnode() : statement_astnode()
{

	astnode_type = SeqNode;
}

void seq_astnode::pushback(statement_astnode *child)
{
	children_nodes.push_back(child);
}

void seq_astnode::print(int ntabs)
{
	printblanks(ntabs);
	printAst("", "l", "seq", &children_nodes);
}

///////////////////////////////////

assignS_astnode::assignS_astnode(exp_astnode *l, exp_astnode *r, string tc) : statement_astnode()
{
	typecast = tc;
	left = l;
	right = r;
	id = "Ass";
	astnode_type = AssNode;
}

void assignS_astnode::print(int ntabs)
{
											
										std::swap(stack.back(), stack[stack.size()-2]);										
	right->print(0);
	Display(right, stack.back());
	string Register = stack.back();
	stack.pop_back();

	
	switch (left->astnode_type) {
	case ArrowNode: {
		fprintf(stdout, "movl %d(%%ebp), %s\n", left->offset, stack.back().c_str());
		fprintf(stdout, "movl %s, %d(%s)\n", Register.c_str(), left->second_offset, stack.back().c_str());
		break;
	}
	case OpUnaryNode: {
		if (left->word == "DEREF") {
			fprintf(stdout, "movl %d(%%ebp), %s\n", left->offset, stack.back().c_str());
			fprintf(stdout, "movl %s, (%s)\n", Register.c_str(), stack.back().c_str());
		}
		break;
	}
	case MemberNode: {
		left->print(0);
		fprintf(stdout, "movl %s, (%s)\n", Register.c_str(), stack.back().c_str());
		break;
	}
	case ArrayRefNode: {
		left->print(0);
		fprintf(stdout, "movl %s, (%s)\n", Register.c_str(), stack.back().c_str());
		break;
	}
	default: {
		fprintf(stdout, "movl %s, %d(%%ebp)\n", Register.c_str(), left->offset);
		break;
	}
}

	stack.push_back(Register);
	std::swap(stack.back(), stack[stack.size()-2]);
}

///////////////////////////////////

assignE_astnode::assignE_astnode(exp_astnode *l, exp_astnode *r) : exp_astnode()
{
	left = l;
	right = r;
	astnode_type = AssignNode;
}

void assignE_astnode::print(int ntabs)
{
										
	std::swap(stack.back(), stack[stack.size()-2]);										
	right->print(0);
	Display(right, stack.back());
	string Register = stack.back();
	stack.pop_back();

	
	switch (left->astnode_type) {
	case ArrowNode: {
		fprintf(stdout, "movl %d(%%ebp), %s\n", left->offset, stack.back().c_str());
		fprintf(stdout, "movl %s, %d(%s)\n", Register.c_str(), left->second_offset, stack.back().c_str());
		break;
	}
	case OpUnaryNode: {
		if (left->word == "DEREF") {
			fprintf(stdout, "movl %d(%%ebp), %s\n", left->offset, stack.back().c_str());
			fprintf(stdout, "movl %s, (%s)\n", Register.c_str(), stack.back().c_str());
		}
		break;
	}
	case MemberNode: {
		left->print(0);
		fprintf(stdout, "movl %s, (%s)\n", Register.c_str(), stack.back().c_str());
		break;
	}
	case ArrayRefNode: {
		left->print(0);
		fprintf(stdout, "movl %s, (%s)\n", Register.c_str(), stack.back().c_str());
		break;
	}
	default: {
		fprintf(stdout, "movl %s, %d(%%ebp)\n", Register.c_str(), left->offset);
		break;
	}
}

	stack.push_back(Register);
	std::swap(stack.back(), stack[stack.size()-2]);
}

///////////////////////////////////

return_astnode::return_astnode(exp_astnode *c) : statement_astnode()
{
	child = c;
	id = "Return";
	astnode_type = ReturnNode;
}

void return_astnode::print(int ntabs)
{
	child->print(0);
	Display(child, stack.back());
	cout << "movl " << stack.back() << ", %eax" << endl;
	cout << "    "<< "leave\n    ret\n" << endl;
}

////////////////////////////////////

if_astnode::if_astnode(exp_astnode *l, statement_astnode *m, statement_astnode *r) : statement_astnode()
{
	left = l;
	middle = m;
	right = r;
	id = "If";
	astnode_type = IfNode;
}

void if_astnode::print(int ntabs)
{
	int a =  tokens;
	tokens += 2;
	left->print(0);
	cout << "cmpl $0, " << stack.back() << endl << "je  .L" << a << endl;
	middle->print(0);
	cout << "jmp  .L" << a+1 << endl << ".L" << a << " :" << endl;
	right->print(0);
	cout << ".L" << a+1 << " :" << endl;
}
////////////////////////////////////

while_astnode::while_astnode(exp_astnode *l, statement_astnode *r) : statement_astnode()
{
	left = l;
	right = r;
	id = "While";
	astnode_type = WhileNode;
}

void while_astnode::print(int ntabs)
{
	int a =  tokens;
	tokens += 2;
	cout << "jmp  .L" << a << endl << ".L" << a+1 << " :" << endl;
	right->print(0);
	cout << ".L" << a << " :" << endl;
	left->print(0);
	cout << "cmpl $1, " << stack.back() << endl << "je  .L" << a+1 << endl;
}
/////////////////////////////////

for_astnode::for_astnode(exp_astnode *l, exp_astnode *m1, exp_astnode *m2, statement_astnode *r) : statement_astnode()
{
	left = l;
	middle1 = m1;
	middle2 = m2;
	right = r;
	id = "For";
	astnode_type = ForNode;
}

void for_astnode::print(int ntabs)
{
	int a =  tokens;
	 tokens += 2;
	left->print(0);
	cout << "jmp  .L" << a << endl << ".L" << a+1 << " :" << endl;
	right->print(0);
	middle2->print(0);
	cout << ".L" << a << " :" << endl;
	middle1->print(0);
	cout << "cmpl $1, " << stack.back() << endl << "je  .L" << a+1 << endl;
}

//////////////////////////////////
string exp_astnode::idname()
{
	return id;
};

op_binary_astnode::op_binary_astnode(string val, exp_astnode *l, exp_astnode *r) : exp_astnode()
{
	id = val;
	word = val;
	left = l;
	right = r;
	astnode_type = OpBinaryNode;
	if(l->tag == r->tag)
	{
		tag = l->tag + 1;
	}
	else
	{
		tag = max(l->tag, r->tag);
	}
}

void op_binary_astnode::print(int ntabs)
{
	Code(left, right, id);
}

///////////////////////////////////

op_unary_astnode::op_unary_astnode(string val) : exp_astnode()
{
	id = val;
	astnode_type = OpUnaryNode;
}

void op_unary_astnode::print(int ntabs)

{                                   
 bool a = (word =="NOT"),b=(word=="UMINUS"),c=(word== "PP"),d=(word== "ADDRESS");

switch (d) {
case 1:
	fprintf(stdout, "leal %d(%%ebp), %s\n", child->offset, stack.back().c_str());
	break;
case 0:

switch (c) {
case 1:
	child->print(0);
	Display(child, stack.back());
	fprintf(stdout, "leal 1(%s), %s\n", stack.back().c_str(), stack[stack.size()-2].c_str());
	fprintf(stdout, "movl %s, %d(%%ebp)\n", stack[stack.size()-2].c_str(), child->offset);
	
	break;
case 0:
switch (b) {
case 1:
	child->print(0);
	Display(child, stack.back());
	fprintf(stdout, "negl %s\n", stack.back().c_str());
	break;
case 0:
switch (a) {
case 1:
	child->print(0);
	Display(child, stack.back());
	CodeGenerator codeGenerator;
	codeGenerator.Compare("EQ_OP_INT", stack.back(), "$0");
	break;
case 0:

	
	child->print(0);
	fprintf(stdout, "movl (%s), %s\n", stack.back().c_str(), stack.back().c_str());
	break;}}}}
   								

}

op_unary_astnode::op_unary_astnode(string val, exp_astnode *l) : exp_astnode()
{
	id = val;
	word = val;
	child = l;
	astnode_type = OpUnaryNode;
	offset = l->offset;
	tag = l->tag;
}

string op_unary_astnode::getoperator()
{
	return id;
}
///////////////////////////////////

funcall_astnode::funcall_astnode() : exp_astnode()
{
	astnode_type = FunCallNode;
}

funcall_astnode::funcall_astnode(identifier_astnode *child)
{
	funcname = child;
	astnode_type = FunCallNode;
}

void funcall_astnode::setname(string name)
{
	funcname = new identifier_astnode(name);
}

void funcall_astnode::pushback(exp_astnode *subtree)
{
	children.push_back(subtree);
}

void funcall_astnode::print(int ntabs)
{
	vector<string> registers;
	registers = {"%edi", "%esi", "%edx", "%ecx", "%ebx", "%eax"};	
    for (auto regIter = registers.rbegin(); regIter != registers.rend(); ++regIter) {
		const auto& reg = *regIter;
		if (reg != stack.back()) {
			cout << "pushl " << reg << endl;
		}
	}


	for (int i = 0; i < (int)children.size(); i++)
	{
		if(children[i]->data_type.type == 3){
			for(int j =0; j<children[i]->data_type.size()/4; j++){
				int offset = children[i]->offset;
				offset += children[i]->data_type.size();
				offset -= 4;
				offset -= 4*j;
				cout << "pushl " << offset<< "(%ebp)" << endl;
			}
		}
		else{
			children[i]->print(0);
			if(children[i]->data_type.array.size() == 0){
				Display(children[i], stack.back());
			}

			cout << "pushl " << stack.back() << endl; 
		}
	}

	cout << "call	" << funcname->word << endl;

	for (int i = 0; i < (int)children.size(); i++){
		cout << "addl	$"<< children[i]->data_type.size() << ", %esp" << endl;
	}

	cout << "movl %eax, " << stack.back() << endl;  

	registers = {"%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi"};
	for (auto regIter = registers.rbegin(); regIter != registers.rend(); ++regIter) {
		const auto& reg = *regIter;
		if (reg != stack.back()) {
			cout << "popl " << reg << endl;
		}
	}
}

proccall_astnode::proccall_astnode(funcall_astnode *fc)
{
	procname = fc->funcname;
	children = fc->children;
	lc_token = fc->lc_token;	
}

void proccall_astnode::print(int ntabs)
{
	if(procname->word == "printf")
	{
		for (int i = 0; i < (int)children.size(); i++)
		{
			children[(int)children.size()-i-1]->print(0);
			if(children[(int)children.size()-i-1]->data_type.array.size() == 0){
				Display(children[(int)children.size()-i-1], stack.back());
			}

			cout << "pushl " << stack.back() << endl; 
		}
		cout << "pushl	$.LC" << lc_token << "\ncall	printf\naddl	$"<<4*((int)children.size() + 1)<< ", %esp" << endl;
	}
	else 
	{
		for (int i = 0; i < (int)children.size(); i++)
		{
			if(children[i]->data_type.type == 3){
				for(int j =0; j<children[i]->data_type.size()/4; j++){
					int offset = children[i]->offset;
					offset += children[i]->data_type.size();
					offset -= 4;
					offset -= 4*j;
					cout << "pushl " << offset<< "(%ebp)" << endl;
				}
			}
			else{
				children[i]->print(0);
				if(children[i]->data_type.array.size() == 0){
					Display(children[i], stack.back());
				}

				cout << "pushl " << stack.back() << endl; 
			}
		}

		cout << "call	" << procname->word << endl;
	}
}
/////////////////////////////////////

intconst_astnode::intconst_astnode(int val) : exp_astnode()
{
	number = val;
	astnode_type = IntConstNode;
}

void intconst_astnode::print(int ntabs)
{
	cout << "movl $" << number << "," << stack.back() << endl; 
}
/////////////////////////////////////
floatconst_astnode::floatconst_astnode(float val) : exp_astnode()
{
	value = val;
	astnode_type = FloatConstNode;
}

void floatconst_astnode::print(int ntabs)
{
	printAst("", "f", "floatconst", value);
}
///////////////////////////////////
stringconst_astnode::stringconst_astnode(string val) : exp_astnode()
{
	word = val;
	value = val;
	astnode_type = StringConstNode;
}

void stringconst_astnode::print(int ntabs)
{
	printAst("", "s", "stringconst", stringTocharstar(value));
}

/////////////////////////////////

identifier_astnode::identifier_astnode(string val) : ref_astnode()
{
	word = val;
	astnode_type = IdentifierNode;
	id = val;
}

void identifier_astnode::print(int ntabs)
{                                           
	int a =0,b=3,e = data_type.type,f = offset;
	bool c = data_type.array.size(),d = inMember ;
	
	switch (c > a) {
		case 1:
			if (offset > a) {
				fprintf(stdout, "movl %d(%%ebp), %s\n", offset, stack.back().c_str());
			} else {
				fprintf(stdout, "leal %d(%%ebp), %s\n", offset, stack.back().c_str());
			}
			break;
		case 0:
			if (d && e == b) {
				fprintf(stdout, "leal %d(%%ebp), %s\n", offset, stack.back().c_str());
				inMember = false;
			} else {
				fprintf(stdout, "movl %d(%%ebp), %s\n", offset, stack.back().c_str());
			}
			break;
		default:
			break;
	}
								

}

////////////////////////////////

arrayref_astnode::arrayref_astnode(exp_astnode *l, exp_astnode *r) : ref_astnode() // again, changed from ref to exp
{
	left = l;
	right = r;
	id = "ArrayRef";
	astnode_type = ArrayRefNode;
	tag = l->tag;
}

void arrayref_astnode::print(int ntabs)
{
	left->print(0);
	string R = stack.back();
	stack.pop_back();
	right->print(0);

	int offsize = left->data_type.size();
	
	for (int i = 0; i < (int)(left->data_type.array.size()) ; i++ )
	{
		offsize /= left->data_type.array[i];
	}

	for (int i = 1; i < (int)(left->data_type.array.size()) ; i++ )
	{
		offsize *= left->data_type.array[i];
	}
	
	cout << "imul $" <<  offsize << ", " << stack.back() << endl; 
	
	cout << "addl " <<  stack.back() << ", " << R << endl;	

	stack.push_back(R);
}

///////////////////////////////

deref_astnode::deref_astnode(ref_astnode *c) : ref_astnode()
{
	child = c;
	id = "Deref";
	astnode_type = DerefNode;
}

void deref_astnode::print(int ntabs)
{
	printAst("", "a", "deref", child);
}

/////////////////////////////////

member_astnode::member_astnode(exp_astnode *l, identifier_astnode *r) //  from ref to exp(1st arg)
{
	left = l;
	right = r;
	astnode_type = MemberNode;
	tag = l->tag;
}

void member_astnode::print(int ntabs)
{
	inMember = true;
	left->print(0);
	fprintf(stdout, "addl $%d, %s\n", second_offset, stack.back().c_str());
}

/////////////////////////////////

arrow_astnode::arrow_astnode(exp_astnode *l, identifier_astnode *r)
{
	left = l;
	right = r;
	astnode_type = ArrowNode;
	tag = l->tag;
}

void arrow_astnode::print(int ntabs)
{
	cout <<  "movl " <<  offset << "(%ebp)," << stack.back() << endl;
	cout <<  "movl " <<  second_offset << "(" << stack.back() << "), " << stack.back() << endl;
}
void printblanks(int blanks)
{
	for (int i = 0; i < blanks; i++)
		cout << " ";
}

/////////////////////////////////

void printAst(const char *astname, const char *fmt...) // fmt is a format string that tells about the type of the arguments.
{
	typedef vector<abstract_astnode *> *pv;
	va_list args;
	va_start(args, fmt);
	while (*fmt != '\0')
	{
		if (*fmt == 'a')
		{
			char *field = va_arg(args, char *);
			abstract_astnode *a = va_arg(args, abstract_astnode *);

			a->print(0);
		}
		else if (*fmt == 's')
		{
			char *field = va_arg(args, char *);
			char *str = va_arg(args, char *);
		}
		else if (*fmt == 'i')
		{
			char *field = va_arg(args, char *);
			int i = va_arg(args, int);
		}
		else if (*fmt == 'f')
		{
			char *field = va_arg(args, char *);
			double f = va_arg(args, double);
		}
		else if (*fmt == 'l')
		{
			char *field = va_arg(args, char *);
			pv f = va_arg(args, pv);
			for (int i = 0; i < (int)f->size(); ++i)
			{
				(*f)[i]->print(0);
			}
		}
		++fmt;
	}
	if ((astname != NULL) && (astname[0] != '\0'))
		va_end(args);
}

char *stringTocharstar(string str)
{
	char *charstar = const_cast<char *>(str.c_str());
	return charstar;
}
