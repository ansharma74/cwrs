#line 9624 "../../doc/bison.texi"
%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "2.6.2"
%defines
%define parser_class_name "calcxx_parser"
#line 9642 "../../doc/bison.texi"
%code requires {
# include <string>
class calcxx_driver;
}
#line 9655 "../../doc/bison.texi"
// The parsing context.
%parse-param { calcxx_driver& driver }
%lex-param   { calcxx_driver& driver }
#line 9668 "../../doc/bison.texi"
%locations
%initial-action
{
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = &driver.file;
};
#line 9683 "../../doc/bison.texi"
%debug
%error-verbose
#line 9693 "../../doc/bison.texi"
// Symbols.
%union
{
  int          ival;
  std::string *sval;
};
#line 9708 "../../doc/bison.texi"
%code {
# include "calc++-driver.hh"
}
#line 9723 "../../doc/bison.texi"
%token        END      0 "end of file"
%token        ASSIGN     ":="
%token <sval> IDENTIFIER "identifier"
%token <ival> NUMBER     "number"
%type  <ival> exp
#line 9737 "../../doc/bison.texi"
%printer    { yyoutput << *$$; } "identifier"
%destructor { delete $$; } "identifier"

%printer    { yyoutput << $$; } <ival>
#line 9748 "../../doc/bison.texi"
%%
%start unit;
unit: assignments exp  { driver.result = $2; };

assignments:
  /* Nothing.  */        {}
| assignments assignment {};

assignment:
     "identifier" ":=" exp
       { driver.variables[*$1] = $3; delete $1; };

%left '+' '-';
%left '*' '/';
exp: exp '+' exp   { $$ = $1 + $3; }
   | exp '-' exp   { $$ = $1 - $3; }
   | exp '*' exp   { $$ = $1 * $3; }
   | exp '/' exp   { $$ = $1 / $3; }
   | "identifier"  { $$ = driver.variables[*$1]; delete $1; }
   | "number"      { $$ = $1; };
%%
#line 9777 "../../doc/bison.texi"
void
yy::calcxx_parser::error (const yy::calcxx_parser::location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
