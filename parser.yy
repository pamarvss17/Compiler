%skeleton "lalr1.cc"
%require  "3.5"

%defines 
%define api.namespace {IPL}
%define api.parser.class {Parser}
%define parse.trace

%code requires{
	#include "ast.hh"
	#include "location.hh"
	#include "symbtab.hh"
	#include "type.hh"
        namespace IPL {
        class Scanner;

   }
}
%printer { std::cerr << $$; } IDENTIFIER
%printer { std::cerr << $$; } INT_CONSTANT
%printer { std::cerr << $$; } FLOAT_CONSTANT
%printer { std::cerr << $$; } STRING_LITERAL
 

%parse-param { Scanner  &scanner  }
%locations
%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>
   #include <string>
   	
   #include "scanner.hh"

   extern vector<string>  PrintLC;
   int lc_token = 0;

#undef yylex
#define yylex IPL::Parser::scanner.yylex
#define YYDEBUG 1

string fname;
std::map<string,abstract_astnode*> ast;
extern SymbTab gst;
SymbTab *st;
SymbTabEntry* currFunc;
extern datatype createVoidStarType();
std::string currStruct;
extern std::map<string,datatype> predefined;
int LOCAL_OFFSET = 0;
int PARAM_OFFSET = 8;
bool struct_declaration = false;

}

%define api.value.type variant
%define parse.assert

%start translation_unit

%token VOID INT FLOAT RETURN WHILE FOR IF ELSE OR_OP STRUCT PRINTF 
%token <std::string> IDENTIFIER  
%token <std::string> INT_CONSTANT FLOAT_CONSTANT 
%token <std::string> STRING_LITERAL 
%token '(' ')'
%token '[' ']' '{' '}'
%token ',' '!'
%token ';'

%nonassoc '='
%left OR_OP
%left AND_OP
%left EQ_OP NE_OP
%left '<' LE_OP 
%left '>' GE_OP
%left '+' '-'
%left '*' '/'
%right '&'
%right USTAR
%right UMINUS

%left PTR_OP
%left '.'
%left INC_OP


%%
%nterm <abstract_astnode*> translation_unit; 
%nterm <abstract_astnode*> struct_specifier; 
%nterm <abstract_astnode*> function_definition; 

%nterm <type_specifier_class*> type_specifier; 
%nterm <declarator_class*> declarator_arr;
%nterm <declarator_class*> declarator;
%nterm <declaration_class*> declaration;
%nterm <declarator_list_class*> declarator_list;
%nterm <declaration_list_class*> declaration_list;

%nterm <exp_astnode*> expression; 
%nterm <fun_declarator_class*> fun_declarator; 
%nterm <parameter_list_class*> parameter_list;
%nterm <parameter_declaration_class*> parameter_declaration;
%nterm <statement_astnode*> statement;
%nterm <assignS_astnode*> assignment_statement;

%nterm <abstract_astnode*> compound_statement;
%nterm <seq_astnode*> statement_list;
%nterm <assignE_astnode*> assignment_expression;
%nterm <exp_astnode*> logical_and_expression;
%nterm <exp_astnode*> equality_expression;
%nterm <exp_astnode*> relational_expression;
%nterm <exp_astnode*> additive_expression;
%nterm <exp_astnode*> multiplicative_expression;
%nterm <exp_astnode*> unary_expression;
%nterm <exp_astnode*> postfix_expression;
%nterm <exp_astnode*> primary_expression;
%nterm <op_unary_astnode*> unary_operator;
%nterm <funcall_astnode*> expression_list;
%nterm <funcall_astnode*> procedure_call;
%nterm <funcall_astnode*>    printf_call;
%nterm <statement_astnode*> selection_statement;
%nterm <statement_astnode*> iteration_statement;


translation_unit: 
    struct_specifier 
	{
		$$ = $1;
	}
  |  function_definition { 
      ast.insert({fname, $1});
    }
  | translation_unit struct_specifier
  | translation_unit function_definition 
    { 
      ast.insert({fname, $2});
    }

;
struct_specifier: 
    STRUCT IDENTIFIER 
{
	  st = new SymbTab();
	  currStruct = "struct "+ $2;
	  if (gst.search("struct "+ $2) != NULL && gst.search("struct "+ $2)->varfun == "struct")
	  {
		  error(@2, "\"" + currStruct + "\"" +  " has a previous definition");
		  
	  }
	  struct_declaration = true;
}
    '{' declaration_list '}' ';'
{
	  struct_declaration = false;
	  currStruct = "";

	  gst.push("struct " + $2,
			   new SymbTabEntry("struct", "global", datatype(), ($5->offset), 0, st));
	  // push the local symbol table in the global one
};

function_definition:  
    
    type_specifier fun_declarator 
    { 
	  if ($1->structname != "" && gst.search($1->structname)== NULL) error(@1, "\""+ $1->structname + "\" " + "not declared");
	  fname = $2->identifier;
      st = new SymbTab();	//create new local symbol table
      datatype dt = createtype($1, $2->deref);
		                    // previous declaration error
      if(gst.search($2->identifier) !=NULL)
      {
        error(@2, "The function \"" + $2->identifier + "\" has a previous definition");
        
      }   
      gst.push($2->identifier , new SymbTabEntry("fun","global",dt,0,0,st));	// push the local symbol table in the global one
	
      currFunc = gst.search($2->identifier);
      parameter_declaration_class* param;
      datatype data_type;
      int total_params_size = 0;			
      parameter_list_class* pl = $2->param;
      if(pl!=NULL){
        for(int i=0;i<(int) (pl->param.size());++i)
        {
          total_params_size += ((pl->param)[i]->t).size();
        }
    
        int cumul = 0;
        for(int i=0; 
		i<(int) (pl->param.size());++i)
        {
		  param = (pl->param)[i];
		  data_type = param->t;
		  // struct is not defined
		  if (data_type.type == 3)
		  {
			  SymbTabEntry *temp = gst.search(data_type.structname);
			  if (temp == NULL || temp->varfun != "struct")
			  {
				  error(@2, "\"" + data_type.structname + "\" is not defined");
			  }
		  }
		  // void data type declaration error
		  if (data_type.type == 0 && data_type.deref == 0)
		  {
			error(@2, "Cannot declare the type of a parameter as  \"void\"");
		
		  }
		  if (st->search(param->identifier) != NULL)
		  {
			error(@2, "\"" + param->identifier + "\"  has a previous declaration");
		  }
          cumul += data_type.size();
		  st->push(param->identifier,
			// add the parameter entry into symbol table
			new SymbTabEntry("var", "param", data_type,
			data_type.size(),
			total_params_size + PARAM_OFFSET - cumul, // offset
			NULL));
        }
      }
    }
    compound_statement 
    {
		$$=$4;  //When you remove the action in between remember to change $4 to $3
	}
;

type_specifier:
    VOID {
      $$ = new type_specifier_class(VOID_TYPE);
    } 
  | INT {
      $$ = new type_specifier_class(INT_TYPE);
    } 
  | FLOAT {
      $$ = new type_specifier_class(FLOAT_TYPE);
    } 
  | STRUCT IDENTIFIER {
      $$ = new type_specifier_class(STRUCT_TYPE, "struct "+ $2);
  }
;      

fun_declarator: 
    IDENTIFIER '(' parameter_list ')'  {
      $$ = new fun_declarator_class($1,$3);

    }
  | IDENTIFIER '(' ')' {
      $$ = new fun_declarator_class($1);
  }
;

parameter_list:
    parameter_declaration 
    {
       $$ = new parameter_list_class();
	   $$->push($1);
    }

  | parameter_list ',' parameter_declaration 
    { 
       $1->push($3);
	   $$ = $1;	   		    
    }
;        
            

parameter_declaration:
    type_specifier declarator  
    {
      $$ = new parameter_declaration_class($1, $2);
    }
;


// The array part of the declarator: x, a[10][10], **y

declarator_arr:
    IDENTIFIER {
      $$= new declarator_class($1);
    }
  | declarator_arr '[' INT_CONSTANT ']' 
	{
		
    if (stoi($3) < 0)
    {
    	error(@3, "Size of array is negative");
    }
		
		$1->add_array_index(stoi($3));		
		$$=$1;  
  }
;

// The pointer part of the declarator: **x, *a[10][10]declarator:

declarator:
   declarator_arr {
   $$ = $1;
}
	| '*' declarator {
    	$2->addstar();
    	$$=$2;
    }
; 

declaration_list: 
        declaration 
	{
		
    	declarator_class* decl;
    	type_specifier_class* type = $1->type;
    	int curr_offset = LOCAL_OFFSET;
    	declarator_list_class* dl = $1->decl_list;
    	datatype dt;
		for(int i=0;i<(int)(dl->v.size());++i)
		{   
			decl = dl->v[i];
			dt = createtype(type, decl);
			// struct is not defined 
			if(dt.type == STRUCT_TYPE)
			{
				SymbTabEntry* temp = gst.search(dt.structname);
				if(dt.structname==currStruct && dt.deref ==0) {
					error(@1, "\"" + dt.structname + "\" is not defined");
				}

				if(dt.structname==currStruct && dt.deref !=0) {

				}
				else if(temp==NULL || (temp->varfun != "struct")) {
					
					error(@1, "\"" + dt.structname + "\"  is not defined");
				}	
			}
			// void data type declaration error
			if(dt.type==VOID_TYPE && dt.deref == 0)
			{
				error(@1,"Cannot declare variable of type \"void\"" );
			}
			// previous declaration error
			if(st->search(decl->identifier) !=NULL)
			{
				error(@1, "\"" + decl->identifier + "\" has a previous declaration");
			}

			if (!struct_declaration) curr_offset -= dt.size();
			st->push(decl->identifier,					// pushing in the local symbol table
				new SymbTabEntry("var","local",
					dt,
					dt.size(),
					curr_offset,
					NULL));
		    if (struct_declaration) curr_offset += dt.size();	
		}	
		
		$$ = new declaration_list_class();
		$$->offset=curr_offset;	
    } 					
    | declaration_list declaration {
		
    	declarator_class* decl;
    	type_specifier_class* type = $2->type;
    	int curr_offset = $1->offset;
    	declarator_list_class* dl = $2->decl_list;
    	datatype dt;

		for(int i=0;i<(int)(dl->v.size());++i)
		{
			decl = dl->v[i];
			dt = createtype(type, decl);
			if(dt.type == 3)
			{
				SymbTabEntry* temp = gst.search(dt.structname);
				if(dt.structname==currStruct && dt.deref ==0) {
					error(@2, "\"" + dt.structname + "\" is not defined");
				}
				if(dt.structname==currStruct && dt.deref !=0) {

				}
				else if(temp==NULL || (temp->varfun != "struct")) {

					error(@2, "\"" + dt.structname + "\" is not defined");
				}	
			}
			// void data type declaration error
			if(dt.type==VOID_TYPE && dt.deref == 0)
			{
				error(@2, "Cannot declare variable of type \"void\"");
			}
			// previous declaration error
			if(st->search(decl->identifier) !=NULL)
			{
				error(@2, "\"" + decl->identifier + "\" has a previous declaration");
				exit(1);
			}
			if (!struct_declaration) curr_offset -= dt.size();
			st->push(decl->identifier,					// pushing in the local symbol table
				new SymbTabEntry("var","local",
					dt,
					dt.size(),
					curr_offset,
					NULL));
		    if (struct_declaration) curr_offset += dt.size();	
		}
		$$ = new declaration_list_class();
		$$->offset=curr_offset;		
    }
	;
// int a, b[10], c;
declaration:
        type_specifier declarator_list';' {
		$$ = new declaration_class($1, $2);
	}
	;

declarator_list:
        declarator {
		$$ = new declarator_list_class();
		$$->push($1);
	}
	| declarator_list ',' declarator
	{
		$1->push($3);
		$$ = $1;
	}
 	; 

compound_statement: 
          '{' '}' {
		$$=new seq_astnode();
    }
	| '{' statement_list '}' {
		$$=$2;
	}
	| '{' declaration_list '}' {
		$$=new seq_astnode();
	}

    | '{' declaration_list statement_list '}' {
		$$=$3;
		
	}
;
 

statement_list:
	statement   {
		$$=new seq_astnode();
		$$->pushback($1);

	}
    | statement_list statement{
		$$=$1;
		$$->pushback($2);	
	}		
	;


statement:
      ';'  {
		$$ = new empty_astnode();
	  }
    |  '{' statement_list '}' {
    	$$=$2;
    } 
    | selection_statement   {
    	$$=$1;
    }		
    | iteration_statement   {
    	$$=$1;
    }		
    | assignment_statement  {
    	$$=$1;

     }		
	 | printf_call        {
		$$ = new proccall_astnode($1);
	}
    | procedure_call        {
		$$ = new proccall_astnode($1);
	}
    | RETURN expression ';' {
		// check if the return type is correct
		if (!compatible(currFunc->type, $2->data_type))
		{
			error(@2, "Incompatible type " + 
			$2->data_type.sprint() + " returned, expected " +
			currFunc->type.sprint());

		}
		if ($2->data_type.is_array)
		{
			if ($2->data_type.array[0] == -1)
			{
				error(@2, "Function returns address of local variable");
			}
		}
		if (isIntType($2->data_type) && isFloatType(currFunc->type))
		{
			$2 = new op_unary_astnode("TO_FLOAT", $2);
		}
		if (isIntType(currFunc->type) && isFloatType($2->data_type))
		{
			$2 = new op_unary_astnode("TO_INT", $2);
		}
          
        $$ = new return_astnode($2);
	};

assignment_expression:
     unary_expression '=' expression
	 {
		if (!($1->lvalue))
		{

			error(@1, "Left operand of assignment should have an lvalue"); 
		}
		else if (isPointer($1->data_type)  && 
		         $3->astnode_type== IntConstNode && 
				 $3->int_val == 0)
		{
	
			$3->data_type = createVoidStarType(); 
			//Special case of a NULL pointer. We are assuming that operationally, a NULL pointer of a type t* is for all purposes a void * with value 0;
			
		}

		else if ($1->astnode_type==IdentifierNode && 
		         $1->data_type.is_array &&
				 (st->search($1->idname()))->scope=="local"
				 )
		{
			error(@$, "Incompatible assignment when assigning to a constant l-value with type " +
			normalize($1->data_type).sprint() +
			" from type " +
            normalize($3->data_type).sprint());
		}
		else if (!compatible($1->data_type, $3->data_type, true))
		{

			error(@$, "Incompatible assignment when assigning to type " +
			normalize($1->data_type).sprint() +
			" from type " +
            normalize($3->data_type).sprint());
		}

		else if (isIntType($1->data_type) && isFloatType($3->data_type))
		{
			$3 = new op_unary_astnode("TO_INT", $3);
		}
		else if (isIntType($3->data_type) && isFloatType($1->data_type))
		{
			$3 = new op_unary_astnode("TO_FLOAT", $3);
		}
		$$ = new assignE_astnode($1, $3);
		$$->data_type = $1->data_type;
		$$->lvalue = false;
	 };

	 // This may generate programs that are syntactically incorrect.
	 // Eliminate them during semantic analysis.
	 ;

assignment_statement : assignment_expression ';'
{
		$$ = new assignS_astnode($1->left, $1->right, $1->typecast);
		
};

procedure_call:
    IDENTIFIER '(' ')' ';'
{
		if (predefined.find($1) == predefined.end())
		{
			SymbTabEntry *temp = gst.search($1);
			if (temp == NULL)
			{
				error(@$, "Procedure \"" + $1 + "\"  not declared");
			}
			if (temp->varfun != "fun")
			{
				error(@$, "Called object \"" + $1 + "\"  is not a procedure");
			}
			std::vector<pair<int, datatype>> v = temp->symbtab->getparameters();
			if (v.size() > 0)
			{
				error(@$, "Procedure \"" + $1 + "\"  called with too few arguments");
			}
			// function call has no datatype
		}
		$$ = new funcall_astnode(new identifier_astnode($1));
		if (predefined.find($1) == predefined.end())
			$$->data_type = gst.search($1)->type;
		else
			$$->data_type = predefined[$1];
		$$->lvalue = false;
}

	| IDENTIFIER '(' expression_list ')' ';'
	{

		if (predefined.find($1) == predefined.end())
		{
			SymbTabEntry *temp = gst.search($1);
			if (temp == NULL)
			{
				error(@$, "Procedure \"" + $1 + "\"  not declared");
			}
			if (temp->varfun != "fun")
			{
				error(@$, "Called object \"" + $1 + "\"  is not a procedure");
			}
			std::vector<pair<int, datatype>> v = temp->symbtab->getparameters();
			if (v.size() < $3->children.size())
			{
				error(@$, "Procedure \"" + $1 + "\"  called with too many arguments");
			}
			if (v.size() > $3->children.size())
			{
				error(@$, "Procedure \"" + $1 + "\"  called with too few arguments");
			}
			for (int i = 0; i < (int)v.size(); ++i)
			{
				if (isPointer(convert(v[i].second)) &&
					$3->children[i]->astnode_type == IntConstNode && $3->int_val == 0)
				{
					$3->data_type = createVoidStarType();
				}

				else if (!compatible(normalize(convert(v[i].second)), normalize(convert($3->children[i]->data_type)), true))
				{
					error(@3, "Expected " +
								  normalize(convert(v[i].second)).sprint() +
								  " but argument is of type " +
								  normalize(convert($3->children[i]->data_type)).sprint());
				}

				if (isIntType($3->children[i]->data_type) && isFloatType(v[i].second))
				{
					$3->children[i] = new op_unary_astnode("TO_FLOAT", $3->children[i]);
				}
				else if (isFloatType($3->children[i]->data_type) && isIntType(v[i].second))
				{
					$3->children[i] = new op_unary_astnode("TO_INT", $3->children[i]);
				}
			}
	};
	$3->setname($1);
	$$ = $3;
	if (predefined.find($1) == predefined.end())
		$$->data_type = gst.search($1)->type;
	else
		$$->data_type = predefined[$1];
	$$->lvalue = false;
	}
	;

	printf_call 
	: PRINTF '(' STRING_LITERAL ')' ';'
	{ 
		stringstream ss1;
		ss1 << lc_token;
		string count = ss1.str();
		string s = "";
		string s1 = ".section	.rodata\n";
		string s2 = ".LC" + count + ":\n";
		string s3 = ".string	" + $3 + "\n.text";
		s = s1 + s2 + s3;
		PrintLC.push_back(s);
		$$ = new funcall_astnode(new identifier_astnode("printf"));
		$$->lc_token = lc_token;
		lc_token++;
		$$->lvalue = false;

	}
	| PRINTF '(' STRING_LITERAL ',' expression_list ')' ';' 
	{
		stringstream ss1;
		ss1 << lc_token;
		string count = ss1.str();
		string s = "";
		string s1 = ".section	.rodata\n";
		string s2 = ".LC" + count + ":\n";
		string s3 = ".string	" + $3 + "\n.text";
		s = s1 + s2 + s3;
		PrintLC.push_back(s);
		$5->setname("printf");
		$$ = $5;
		$$->lc_token = lc_token;
		lc_token++;
		$$->lvalue = false;

	}
	

	expression : logical_and_expression
	{
    	$$=$1;
    }
    | expression OR_OP logical_and_expression {
		  exp_astnode* arg1=$1, *arg3=$3;
		if (!isScalarOrPointerType(convert($1->data_type)))
		 error(@1, "Invalid operand of ||, not scalar or pointer");
		else if (!isScalarOrPointerType(convert($3->data_type)))
		 error(@3, "Invalid operand of ||, not scalar or pointer");
	
    	$$=new op_binary_astnode("OR_OP", $1, $3);
    	if(arg3->is_calculable || arg1->is_calculable)
		{
			$$->is_calculable = true;
			$$->int_val = (arg1->int_val) && (arg3->int_val);
		}
    	$$->data_type = createtype(INT_TYPE);
    	$$->lvalue = false;
   }
 ;


logical_and_expression:
  	equality_expression {
    	$$=$1;
    }
  | logical_and_expression AND_OP equality_expression
  {
		exp_astnode *arg1 = $1, *arg3 = $3;
		if (!isScalarOrPointerType(convert($1->data_type)))
		 error(@1, "Invalid operand of &&,  not scalar or pointer");
		else if (!isScalarOrPointerType(convert($3->data_type)))
		 error(@3, "Invalid operand of &&,  not scalar or pointer");

		$$ = new op_binary_astnode("AND_OP", $1, $3);
		if (arg3->is_calculable && arg1->is_calculable)
		{
			$$->is_calculable = true;
			$$->int_val = (arg1->int_val) && (arg3->int_val);
		}
		$$->data_type = createtype(INT_TYPE);
		$$->lvalue = false;
  };
equality_expression :
    relational_expression {
		$$=$1;
	}
  | equality_expression EQ_OP relational_expression {
		exp_astnode* arg1=$1, *arg3=$3;
		if(isPointer($1->data_type) && $3->astnode_type== IntConstNode && $3->int_val==0) {
    		$3->data_type = createVoidStarType();
    	}

    	if(isPointer($3->data_type) && $1->astnode_type== IntConstNode && $1->int_val==0) {
    		$1->data_type = createVoidStarType();
    	}
        
		if (!isScalarOrPointerType(normalize(convert($1->data_type))) ||
			!isScalarOrPointerType(normalize(convert($3->data_type))) ||
			!compatible($1->data_type,$3->data_type, true))
			error(@$, "Invalid operands types for binary ==, " +
			          normalize($1->data_type).sprint() +
					  " and " +
					  normalize($3->data_type).sprint());

    	if(isFloatType($1->data_type) || isFloatType($3->data_type))
		{

			if(isIntType($1->data_type))
				$1 = new op_unary_astnode("TO_FLOAT", $1);
			if(isIntType($3->data_type))
				$3 = new op_unary_astnode("TO_FLOAT", $3);
			$$=new op_binary_astnode("EQ_OP_FLOAT", $1, $3);
		}
		else {
			$$=new op_binary_astnode("EQ_OP_INT", $1, $3);
			if(arg3->is_calculable && arg1->is_calculable)
			{
				$$->is_calculable = true;
				$$->int_val = (arg1->int_val)==(arg3->int_val);
			}
		}
    	$$->data_type = createtype(INT_TYPE);
    	$$->lvalue = false;
    }	
	| equality_expression NE_OP relational_expression {
		if(isPointer($1->data_type) && $3->is_calculable && $3->int_val==0) {
    		$3-> data_type = createVoidStarType();
    	}

    	if(isPointer($3->data_type) && $1->is_calculable && $1->int_val==0) {
    		$1->data_type = createVoidStarType();
    	}

		if (!isScalarOrPointerType(normalize(convert($1->data_type))) ||
			!isScalarOrPointerType(normalize(convert($3->data_type))) ||
			!compatible($1->data_type,$3->data_type, true))
		{
    		error(@$, "Invalid operands types for binary !=, " +
			          normalize($1->data_type).sprint() +
					  " and " +
					  normalize($3->data_type).sprint());  
		
		}
		if(isFloatType($1->data_type) || isFloatType($3->data_type))
		{
			if(isIntType($1->data_type))
				$1 = new op_unary_astnode("TO_FLOAT", $1);
			if(isIntType($3->data_type))
				$3 = new op_unary_astnode("TO_FLOAT", $3);
			$$=new op_binary_astnode("NE_OP_FLOAT", $1, $3);
		}
		else {
			$$=new op_binary_astnode("NE_OP_INT", $1, $3);
		}
    	$$->data_type = createtype(INT_TYPE);
		$$->lvalue = false;
		
	}
	;

relational_expression:
      additive_expression{
		$$=$1;
	}
    | relational_expression '<' additive_expression {
		exp_astnode* arg1=$1, *arg3=$3;

		if(isPointer($1->data_type) && $3->is_calculable && $3->int_val==0) {
    		$3->data_type = createVoidStarType();
    	}

    	if(isPointer($3->data_type) && $1->is_calculable && $1->int_val==0) {
    		$1->data_type = createVoidStarType();
    	}

    	if (!isScalarOrPointerType(convert($1->data_type)) ||
			!isScalarOrPointerType(convert($3->data_type)) ||
			!compatible(normalize(convert($1->data_type)),normalize(convert($3->data_type))))

		{
			error(@$, "Invalid operands types for binary < , " +
			          normalize($1->data_type).sprint() +
					  " and " +
					  normalize($3->data_type).sprint()); 					  
		}
		if(isFloatType($1->data_type) || isFloatType($3->data_type))
		{
			if(isIntType($1->data_type))
				$1 = new op_unary_astnode("TO_FLOAT", $1);
			if(isIntType($3->data_type))
				$3 = new op_unary_astnode("TO_FLOAT", $3);
			$$=new op_binary_astnode("LT_OP_FLOAT", $1, $3);
		}
		else
		{
			$$=new op_binary_astnode("LT_OP_INT", $1, $3);
			if(arg3->is_calculable && arg1->is_calculable)
			{
				$$->is_calculable = true;
				$$->int_val = (arg1->int_val)<(arg3->int_val);
			}
		}
		$$->data_type = createtype(INT_TYPE);
   		$$->lvalue = false;
   	}
	| relational_expression '>' additive_expression  {
		exp_astnode* arg1=$1, *arg3=$3;

		if(isPointer($1->data_type) && $3->is_calculable && $3->int_val==0) {
    		$3->data_type = createVoidStarType();
    	}

    	if(isPointer($3->data_type) && $1->is_calculable && $1->int_val==0) {
    		$1->data_type = createVoidStarType();
    	}

		
		if (!isScalarOrPointerType(convert($1->data_type)) ||
			!isScalarOrPointerType(convert($3->data_type)) ||
			!compatible(normalize(convert($1->data_type)),normalize(convert($3->data_type))))
		{
			error(@$, "Invalid operand types for binary > , " +
			          normalize($1->data_type).sprint() +
					  " and " +
					  normalize($3->data_type).sprint()); 					  
		}
		
		
		if(isFloatType($1->data_type) || isFloatType($3->data_type))
		{
			if(isIntType($1->data_type))
				$1 = new op_unary_astnode("TO_FLOAT", $1);
			if(isIntType($3->data_type))
				$3 = new op_unary_astnode("TO_FLOAT", $3);
			$$=new op_binary_astnode("GT_OP_FLOAT", $1, $3);
		}
		else
		{
			$$=new op_binary_astnode("GT_OP_INT", $1, $3);
			if(arg3->is_calculable && arg1->is_calculable)
			{
				$$->is_calculable = true;
				$$->int_val = (arg1->int_val)>(arg3->int_val);
			}
		}
		$$->data_type = createtype(INT_TYPE);
		$$->lvalue = false;
	}
	| relational_expression LE_OP additive_expression  {
		exp_astnode* arg1=$1, *arg3=$3;
		if(isPointer($1->data_type) && $3->is_calculable && $3->int_val==0) {
    		$3->data_type = createVoidStarType();
    	}

    	if(isPointer($3->data_type) && $1->is_calculable && $1->int_val==0) {
    		$1->data_type = createVoidStarType();
    	}

		if (!isScalarOrPointerType(convert($1->data_type)) ||
			!isScalarOrPointerType(convert($3->data_type)) ||
			!compatible(normalize(convert($1->data_type)),normalize(convert($3->data_type))))
		
		{
			error(@$, "Invalid operand types for binary <= , " +
			          normalize($1->data_type).sprint() +
					  " and " +
					  normalize($3->data_type).sprint()); 					  
		}
		
		if(isFloatType($1->data_type) || isFloatType($3->data_type))
		{
			if(isIntType($1->data_type))
            	$1 = new op_unary_astnode("TO_FLOAT", $1);
			if(isIntType($3->data_type))
				$3 = new op_unary_astnode("TO_FLOAT", $3);
			$$=new op_binary_astnode("LE_OP_FLOAT", $1, $3);
		}
		else
		{
			$$=new op_binary_astnode("LE_OP_INT", $1, $3);
			if(arg3->is_calculable && arg1->is_calculable)
			{
				$$->is_calculable = true;
				$$->int_val = (arg1->int_val)<=(arg3->int_val);
			}
		}
		$$->data_type = createtype(INT_TYPE);
		$$->lvalue = false;
	}
    | relational_expression GE_OP additive_expression {
		exp_astnode* arg1=$1, *arg3=$3;

		if(isPointer($1->data_type) && $3->is_calculable && $3->int_val==0) {
    		$3->data_type = createVoidStarType();
    	}

    	if(isPointer($3->data_type) && $1->is_calculable && $1->int_val==0) {
    		$1->data_type = createVoidStarType();
    	}

    	
		if (!isScalarOrPointerType(convert($1->data_type)) ||
			!isScalarOrPointerType(convert($3->data_type)) ||
			!compatible(normalize(convert($1->data_type)),normalize(convert($3->data_type))))
			
		{
			error(@$, "Invalid operand types for binary >= , " +
			          normalize($1->data_type).sprint() +
					  " and " +
					  normalize($3->data_type).sprint()); 					  
		}
		
		
		if(isFloatType($1->data_type) || isFloatType($3->data_type))
		{
			if(isIntType($1->data_type))
            	$1 = new op_unary_astnode("TO_FLOAT", $1);
			if(isIntType($3->data_type))
				$3 = new op_unary_astnode("TO_FLOAT", $3);
			$$=new op_binary_astnode("GE_OP_FLOAT", $1, $3);
		}
		else
		{
			$$=new op_binary_astnode("GE_OP_INT", $1, $3);
			if(arg3->is_calculable && arg1->is_calculable)
			{
				$$->is_calculable = true;
				$$->int_val = (arg1->int_val)>=(arg3->int_val);
			}
		}
		$$->data_type = createtype(INT_TYPE);
    	$$->lvalue = false;
    } 
	;

additive_expression:
    multiplicative_expression{
		$$=$1;
	}
	| additive_expression '+' multiplicative_expression
	{
		if (isPointer($1->data_type) &&
			isPointer($3->data_type))
			error(@$, "Invalid operand types for binary + , " +
						  normalize($1->data_type).sprint() +
						  " and " +
						  normalize($3->data_type).sprint());

		else if ((isPointer($1->data_type) &&
			 isIntType($3->data_type)) ||
			(isPointer($3->data_type) &&
			 isIntType($1->data_type)))
		{
			$$ = new op_binary_astnode("PLUS_INT", $1, $3);
			$$->data_type = isPointer($1->data_type) ? $1->data_type : $3->data_type;
		}
		else if ((isArray($1->data_type) &&
			 isIntType($3->data_type)) ||
			(isArray($3->data_type) &&
			 isIntType($1->data_type)))
		{
			$$ = new op_binary_astnode("PLUS_INT", $1, $3);
			$$->data_type = isArray($1->data_type) ? convert(normalize($1->data_type)) : convert(normalize($3->data_type));
		}

		else
		{
            // none of the operands are pointers
			if (!(isIntOrFloatType($1->data_type) && isIntOrFloatType($3->data_type)))
			
             error(@$, "Invalid operand types for binary + , " +
			          normalize($1->data_type).sprint() +
					  " and " +
					  normalize($3->data_type).sprint()); 

			if (isFloatType($1->data_type) || isFloatType($3->data_type))
			{
				if (isIntType($1->data_type))
                	$1 = new op_unary_astnode("TO_FLOAT", $1);
			    if(isIntType($3->data_type))
				    $3 = new op_unary_astnode("TO_FLOAT", $3);
				$$ = new op_binary_astnode("PLUS_FLOAT", $1, $3);
				$$->data_type = createtype(FLOAT_TYPE);
			}
			else
			{

				$$ = new op_binary_astnode("PLUS_INT", $1, $3);
				$$->data_type = createtype(INT_TYPE);
			}
		}
		$$->lvalue = false;
	}
	| additive_expression '-' multiplicative_expression  {

		if (isPointer(convert($1->data_type)) &&
			isPointer(convert($3->data_type)) &&
			compatible(normalize(convert($1->data_type)), normalize(convert($3->data_type))))
		{
			$$ = new op_binary_astnode("MINUS_INT", $1, $3);
			$$->data_type = createtype(INT_TYPE);
		}
		else if (isArray($1->data_type) &&
				 isArray($3->data_type) &&
				 compatible(normalize(convert($1->data_type)), normalize(convert($3->data_type))))
			{
				$$ = new op_binary_astnode("MINUS_INT", $1, $3);
				$$->data_type = createtype(INT_TYPE);
			}
		else if (isPointer($1->data_type) &&
				 isIntType($3->data_type))
		{
			$$ = new op_binary_astnode("MINUS_INT", $1, $3);
			$$->data_type = $1->data_type;
		}
        else if (isArray($1->data_type) &&
				 isIntType($3->data_type))
		{
			$$ = new op_binary_astnode("MINUS_INT", $1, $3);
			$$->data_type = normalize(convert($1->data_type));
		}
		else if (isFloatType($1->data_type) || isFloatType($3->data_type))
		{
			if (isIntType($1->data_type))
				$1 = new op_unary_astnode("TO_FLOAT", $1);
			if (isIntType($3->data_type))
				$3 = new op_unary_astnode("TO_FLOAT", $3);
			$$ = new op_binary_astnode("MINUS_FLOAT", $1, $3);
			$$->data_type = createtype(FLOAT_TYPE);
		}
		else if (isIntType($1->data_type) && isIntType($3->data_type))
		{
			$$ = new op_binary_astnode("MINUS_INT", $1, $3);
			$$->data_type = createtype(INT_TYPE);
		}
		else
			error(@$, "Invalid operand types for binary - , " +
						  normalize($1->data_type).sprint() +
						  " and " +
						  normalize($3->data_type).sprint());
		$$->lvalue = false;
	}
		;

unary_expression:
 postfix_expression  {
		$$=$1;
	}  								
	| unary_operator unary_expression  {
		exp_astnode* arg2=$2;
		$$=new op_unary_astnode($1->getoperator(),$2);
		if($1->getoperator()=="DEREF") {
			if(!isDereferenceAllowed($2->data_type)) {
				error(@2, "Invalid operand type " +  arg2->data_type.sprint() + " of unary *");			}
			$$->lvalue=true;
			$$->data_type=extract_type(arg2->data_type, TYPE_DEREF);
		}
		else if($1->getoperator()=="ADDRESS") {
			if(!arg2->lvalue) {
				error(@2, "Operand of & should  have lvalue");	
				
			}

			$$->lvalue = false;
			$$->data_type = arg2->data_type;
			$$->data_type.addpointer();

		}
		else if($1->getoperator()=="UMINUS")
		{

			if(!isIntOrFloatType(arg2->data_type)){
				error(@2, " Operand of unary - should be an int or float");
			}
			if(arg2->is_calculable)
			{
				$$->is_calculable = true;
				if($1->getoperator()=="UMINUS")
				{
					$$->int_val = -1*arg2->int_val ;
				}
				else
				{
					if(arg2->int_val !=0)
					{
						$$->int_val = 0;
					}
					else
					{
						$$->int_val = 1;
					}
				}
			}
			$$->data_type = $2->data_type;
		}
        else if ($1->getoperator()=="NOT")
		{
			if(!isScalarOrPointerType(normalize(convert (arg2->data_type)))){
				error(@2, " Operand of NOT should be an int or float or pointer");
			}
			$$->lvalue = false;
			$$->data_type = createtype(INT_TYPE);

		}
	}   												// unary_operator can only be '*' on the LHS of '='
	;                                     				// you have to enforce this during semantic analysis


multiplicative_expression:
    unary_expression {
		$$=$1;
	}
	| multiplicative_expression '*' unary_expression {
		exp_astnode* arg1=$1, *arg3=$3;
		if(!(isIntOrFloatType($1->data_type) && isIntOrFloatType($3->data_type)))
		{
			error(@$, "Invalid operand types for binary * , " +
			          normalize($1->data_type).sprint() +
					  " and " +
					  normalize($3->data_type).sprint());
		}
		if(isFloatType($1->data_type) || isFloatType($3->data_type))
		{
			if(isIntType($1->data_type))
				$1 = new op_unary_astnode("TO_FLOAT", $1);
			if(isIntType($3->data_type))
				$3 = new op_unary_astnode("TO_FLOAT", $3);
			$$=new op_binary_astnode("MULT_FLOAT", $1, $3);
			$$->data_type = createtype(FLOAT_TYPE);
		}
		else
		{
			$$=new op_binary_astnode("MULT_INT", $1, $3);
			if(arg3->is_calculable && arg1->is_calculable)
			{
				$$->is_calculable = true;
				$$->int_val = (arg1->int_val)*(arg3->int_val);
			}
			$$->data_type = createtype(INT_TYPE);
		}
		$$->lvalue = false;
	} 
	| multiplicative_expression '/' unary_expression {
		exp_astnode* arg1=$1, *arg3=$3;
		if(!(isIntOrFloatType($1->data_type) && isIntOrFloatType($3->data_type)))
		{
			error(@$, "Invalid operand types for binary / , " +
			          normalize($1->data_type).sprint() +
					  " and " +
					  normalize($3->data_type).sprint()); 
					  
		}
		if(isFloatType($1->data_type) || isFloatType($3->data_type))
		{
			if(isIntType($1->data_type))
            	$1 = new op_unary_astnode("TO_FLOAT", $1);
			if(isIntType($3->data_type))
				$3 = new op_unary_astnode("TO_FLOAT", $3);
			$$=new op_binary_astnode("DIV_FLOAT", $1, $3);
			$$->data_type = createtype(FLOAT_TYPE);
		}
		else
		{
			$$=new op_binary_astnode("DIV_INT", $1, $3);
			if(arg3->is_calculable && arg1->is_calculable)
			{
				$$->is_calculable = true;
				$$->int_val = (arg1->int_val)*(arg3->int_val);
			}
			$$->data_type = createtype(INT_TYPE);
		}
		$$->lvalue = false;
	} 
 ;

  
// pe, pe(x,y), le++

postfix_expression: 
    primary_expression {
        $$=$1;
}
  | postfix_expression '[' expression ']'
  {

		exp_astnode *arg1 = $1;
		datatype expr_dt = $3->data_type;
		if (!isIntType(expr_dt))
		{
			error(@3, "Array subscript is not an integer");
		}
		if (!$1->data_type.is_array && $1->data_type.deref == 0)
		{
			error(@1, "Subscripted value is neither array nor pointer");
		}
		$$ = new arrayref_astnode($1, $3);
		if (arg1->data_type.is_array)
		{
			$$->data_type = extract_type(arg1->data_type, TYPE_ARRAY_REF);
		}
		else
		{
			$$->data_type = arg1->data_type;
			$$->data_type.deref--;
		}
		$$->lvalue = true;
  }

  | IDENTIFIER '(' ')'
  {
		if (predefined.find($1) == predefined.end())
		{
			SymbTabEntry *temp = gst.search($1);
			if (temp == NULL)
			{
				error(@$, "Function \"" + $1 + "\"  not declared");
			}
			if (temp->varfun != "fun")
			{
				error(@$, "Called object \"" + $1 + "\"  is not a function");
			}
			std::vector<pair<int, datatype>> v = temp->symbtab->getparameters();
			if (v.size() > 0)
			{
				error(@$, "Function \"" + $1 + "\"  called with too few arguments");
			}
		}
		// function call has no datatype
		$$ = new funcall_astnode(new identifier_astnode($1));
		if (predefined.find($1) == predefined.end())
			$$->data_type = gst.search($1)->type;
		else
			$$->data_type = predefined[$1];
		$$->lvalue = false;
  }
	| IDENTIFIER '(' expression_list ')'
	{
		if (predefined.find($1) == predefined.end())
		{
			SymbTabEntry *temp = gst.search($1);
			if (temp == NULL)
			{
				error(@$, "Function \"" + $1 + "\"  not declared");
			}
			if (temp->varfun != "fun")
			{
				error(@$, "Called object \"" + $1 + "\"  is not a function");
			}
			std::vector<pair<int, datatype>> v = temp->symbtab->getparameters();
			if (v.size() < $3->children.size())
			{
				error(@$, "Function \"" + $1 + "\"  called with too many arguments");
			}
			if (v.size() > $3->children.size())
			{   
				error(@$, "Function \"" + $1 + "\"  called with too few arguments");
			}
			for (int i = 0; i < (int)v.size(); ++i)
			{   
				if (!compatible(normalize(convert(v[i].second)), normalize(convert($3->children[i]->data_type)), true))
				{   
					error(@3, "Expected " +
					v[i].second.sprint() +
					" but argument is of type " +
					$3->children[i]->data_type.sprint());
				}
				if (isIntType( $3->children[i]->data_type) && isFloatType(v[i].second)) 
				{
					$3->children[i] = new op_unary_astnode("TO_FLOAT", $3->children[i]);
				}
				else if (isFloatType( $3->children[i]->data_type) && isIntType(v[i].second)) 
				{
					$3->children[i] = new op_unary_astnode("TO_INT", $3->children[i]);
				}
			}
		}
		$3->setname($1);
		$$ = $3;
		if (predefined.find($1) == predefined.end())
			$$->data_type = gst.search($1)->type;
		else
			$$->data_type = predefined[$1];
		$$->lvalue = false;
	}

  | postfix_expression '.' IDENTIFIER {
		// check if data type of postfix exp is struct
		datatype tempdt = $1->data_type;
		if (!isStructType(tempdt))
		{
			error(@1, "Left operand of \".\"  is not a  structure");
		}
		// if postfix's type is struct, check if it has member named identifier
		SymbTabEntry *temp = gst.search(tempdt.structname);
		SymbTab *struct_table = (temp -> symbtab);
		SymbTabEntry *identifier_entry = struct_table->search($3);
		if (identifier_entry == NULL)
		{
			error(@1, "Struct \"" + tempdt.structname + "\" has no member named \"" + $3 + "\"");
		}
		$$ = new member_astnode($1, new identifier_astnode($3)); // Add in lex
		$$->data_type = identifier_entry->type;
		$$->lvalue = true;
		$$->offset = $1->offset + identifier_entry->offset ;
		$$->second_offset = identifier_entry->offset;
  }
  | postfix_expression PTR_OP IDENTIFIER{
		datatype left_dt = $1->data_type;
		if (!isStructPointerType(convert(left_dt)))
		{
			error(@1, "Left operand of \"->\" is not a pointer to structure");
		}
		SymbTabEntry *temp = gst.search(left_dt.structname);
		SymbTab *struct_table = (temp->symbtab);
		SymbTabEntry *identifier_entry = struct_table->search($3);
		if (identifier_entry == NULL)
		{
			error(@1, "Struct \"" + left_dt.structname + "\" has no member named " + $3);
		}
		$$ = new arrow_astnode($1, new identifier_astnode($3)); // Add in lex
		$$->data_type = identifier_entry->type;
		$$->lvalue = true;
		$$->offset = $1->offset ;
		$$->second_offset = -1*(10*identifier_entry->offset)/(20 -35 +5);
  }
  | postfix_expression INC_OP {
    	exp_astnode* arg1=$1;
    	if(!(isScalarOrPointerType($1->data_type)))
    	{
    		error(@1, "Operand of \"++\" should be a int, float or pointer");
    	}
    	if(!$1->lvalue)
    	{
    		error(@1, "Operand of \"++\" should have lvalue");
    	}							

		$$=new op_unary_astnode("PP",$1);				// type checking for incop
		$$->data_type = arg1->data_type;
		$$->lvalue = false;
	} 	
  
  
  ;

// le, 2, "string", (x+y)
primary_expression:
    IDENTIFIER {
		SymbTabEntry *temp = st->search($1);
		if (temp == NULL)
		{
			error(@1, "Variable \"" +  $1 + "\" not declared");
		}
		$$ = new identifier_astnode($1);
		$$->data_type = temp->type;
		$$->lvalue = true;
		$$->tag = 1;
		$$->offset = temp->offset;
		if(temp->scope == "param"){
        	$$->offset = temp->offset;
		}
		else{
        	$$->offset = temp->offset - 4;
		}
}
  
  | INT_CONSTANT {
		int arg1 = std::stoi($1);
		$$ = new intconst_astnode(std::stoi($1)); // the values of these expressions are already stored through the constructor function
		$$->astnode_type = IntConstNode;
		$$->data_type = createtype(INT_TYPE);
		$$->lvalue = false;
		$$->is_calculable = true;
		$$->int_val = arg1;
		$$->tag = 1;

  }
  | FLOAT_CONSTANT {
		$$ = new floatconst_astnode(std::stof($1));
		$$->data_type = createtype(FLOAT_TYPE);
		$$->lvalue = false;
  }
  | STRING_LITERAL {
		$$ = new stringconst_astnode($1); // did not set the data type for primary_expression here
		$$->lvalue = false;
		$$->data_type = createtype(7);
  }
  | '(' expression ')'  {
		$$ = $2;
  }
;

expression_list:
  expression {
    	$$=new funcall_astnode();
    	$$->pushback($1);	
    }
    | expression_list ',' expression {
    	$$=$1;
    	$$->pushback($3);
    }
	;

unary_operator:
  '-' {
		$$ = new op_unary_astnode("UMINUS");
	}
 | '!'  {
		$$=new op_unary_astnode("NOT");
	}
 | '&'  {
		$$=new op_unary_astnode("ADDRESS");					
	} 
 | '*' {
		$$=new op_unary_astnode("DEREF");
	}
	;

selection_statement:
 IF '(' expression ')' statement ELSE statement {
	    if (!isScalarOrPointerType($3->data_type))
			error(@3, "The guard should be a int or float or a pointer");
    	$$=new if_astnode($3,$5,$7);
    }
	;

iteration_statement: 
    WHILE '(' expression ')' statement {
		if (!isScalarOrPointerType($3->data_type))
			error(@3, "The guard should be a int or float or a pointer");
		$$ = new while_astnode($3, $5);
	}
  | FOR '(' assignment_expression ';' expression ';' assignment_expression ')' statement {
	
	if (!isScalarOrPointerType($5->data_type))
		error(@5, "The guard should be a int or float or a pointer");
   $$=new for_astnode($3,$5,$7,$9);   
  }
;

%%
void 
IPL::Parser::error( const location_type &l, const std::string &err_message )
{
	std::cout << "Error at line " << l.begin.line << ": " << err_message << "\n";
//    std::cout << "Error at location " << l << ": " << err_message << "\n";
    exit(1);

}