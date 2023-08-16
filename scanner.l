%{

#include "scanner.hh"
#include <fstream>

#undef  YY_DECL
#define YY_DECL int IPL::Scanner::yylex( IPL::Parser::semantic_type * const lval, IPL::Parser::location_type *loc )


/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H

/* update location on matching */
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);
using token_type = IPL::Parser::token;

    
%}

%option debug
%option nodefault
%option yyclass="IPL::Scanner"
%option noyywrap
%option c++

%%
%{          /** Code executed at the beginning of yylex **/
            yylval = lval;
%}

[ \t] 		;
[\n]              loc->lines();
void 			return token_type::VOID;
int		  	return token_type::INT;
float 		return token_type::FLOAT;
return		return token_type::RETURN;
"<="			return token_type::LE_OP;
"->"              return token_type::PTR_OP;
">="			return token_type::GE_OP;
"=="			return token_type::EQ_OP;
"!="			return token_type::NE_OP;
"||"			return token_type::OR_OP;
"&&"			return token_type::AND_OP;
"++"			return token_type::INC_OP;
if 			return token_type::IF;
else			return token_type::ELSE;
while			return token_type::WHILE;
for			return token_type::FOR;
struct		return token_type::STRUCT;
printf		return token_type::PRINTF;



"/*"[^*]*(\*([^/*][^*]*)?)*"*/" // skip return COMMENT;

([0-9]+)|(0[bB][01]*)|(0[0-7]*)|(0[xX][0-9a-fA-F]*) {
      yylval->emplace< std::string >( YYText ());
      return token_type::INT_CONSTANT;
}

([0-9]+\.([0-9]*([eE]([+\-]?)[0-9]+)?))|([0-9]*\.([0-9]+([eE]([+\-]?)[0-9]+)?))		{
      yylval->emplace< std::string >( YYText ());
  		return token_type::FLOAT_CONSTANT;
}

\"([^"\\\n]|\\\"|\\\\|\\t|\\n)*\"	 {
      yylval->emplace< std::string >( YYText ());
      return token_type::STRING_LITERAL;
}

[[:alpha:]_][[:alpha:][:digit:]_]*	{   
      yylval->emplace< std::string >( YYText() );
       return token_type::IDENTIFIER;
} 



<<EOF>>         yyterminate();
"+"             return  '+';
"-"             return  '-';
">"             return  '>';
"<"             return  '<';
"*"             return  '*';
"/"             return  '/';
"("             return  '(';
")"             return  ')';
"="             return  '=';
"["             return  '[';
"]"             return  ']';
","             return  ',';
"!"             return  '!';
"{"             return  '{';
"}"             return  '}';
";"             return  ';';
"&"             return  '&';
"."             return  '.';
.				        ;


%%

/* const char* tok_to_string(int tok) 
{
   switch ((token_type::yytokentype) tok) 
  {
        case token_type::EOFILE: return "EOFILE";
        case token_type::VOID: return "VOID";
        case token_type::INT: return "INT";
        case token_type::FLOAT: return "FLOAT";
        case token_type::RETURN: return "RETURN";
        case token_type::LE_OP: return "LE_OP";
        case token_type::GE_OP: return "GE_OP";
        case token_type::EQ_OP : return "EQ_OP";
        case token_type::NE_OP : return "NE_OP";
        case token_type::OR_OP : return "OR_OP";
        case token_type::AND_OP : return "AND_OP";
        case token_type::INC_OP : return "INC_OP";
        case token_type::IF : return "IF";
        case token_type::ELSE : return "ELSE";
        case token_type::WHILE : return "WHILE";
        case token_type::FOR : return "FOR";
        case token_type::INT_CONSTANT : return "INT_CONSTANT";
        case token_type::FLOAT_CONSTANT : return "FLOAT_CONSTANT";
        case token_type::STRING_LITERAL : return "STRING_LITERAL";
        case token_type::IDENTIFIER : return "IDENTIFIER";
        case token_type::OTHERS : return "OTHERS";
        
}
return 0;
} */
/* int main(int, char**)
 { 
  using namespace std;
  fstream in_file, out_file;
  in_file.open("armstrong.c", ios::in);
  out_file.open("armstrong_output", ios::out);
  IPL::Scanner scanner(in_file, out_file);
  IPL::Parser::symbol_type yyla;
  int tok = scanner.yylex (&yyla.value, &yyla.location);

  while(tok != token_type::EOFILE)
    {
      out_file << "_______________________________" << endl;
      out_file << scanner.YYText() << endl;
      out_file << tok_to_string(tok) << endl;
      tok = scanner.yylex(&yyla.value, &yyla.location);
    }
    return 0; 

} */
