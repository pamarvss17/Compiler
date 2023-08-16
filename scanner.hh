
// Dont know whether this is required or not, keeping it for now.
// #ifndef __IPLSCANNER_HPP__
// #define __IPLSCANNER_HPP__ 1

/*If FlexLexer.h has not been included before */
#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

/* We include mc_parser.tab.hh for the token definitions */

#include "parser.tab.hh"
#include "location.hh"

/*

Declaration of IPL::Scanner

--The constructor IPL_Scanner that inherits from 
  yyFlexLexer

--Declares the type of IPL::Scanner::yylex, which is inline int yylex(IPL::Parser::semantic_type *lval,     IPL::Parser::location_type *location)

*/


namespace IPL
{

   class Scanner : public yyFlexLexer
   {
   public:
      Scanner(std::istream& in) : yyFlexLexer(in, std::cout)
      {
         loc = new IPL::Parser::location_type();
      };

      // get rid of override virtual function warning
      //using FlexLexer::yylex;

      virtual int yylex(IPL::Parser::semantic_type *const lval,
                        IPL::Parser::location_type *location);
      // YY_DECL defined in mc_lexer.l
      // Method body created by flex in mc_lexer.yy.cc

   private:
      /* yyval ptr */
      IPL::Parser::semantic_type *yylval = nullptr;
      /* location ptr */
      IPL::Parser::location_type *loc = nullptr;
   };

} /* end namespace IPL */

// #endif /* END __IPLSCANNER_HPP__ */

