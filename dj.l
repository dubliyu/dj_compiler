/* LEXER for DJ */
/* Carlos Leon */

%option yylineno
%option noyywrap

%{
  #define FLEX_DEBUG 0
  typedef YYTOKENTYPE Token;
  Token scanned(Token t);
%}

digit	[0-9]
id [a-zA-Z]+[0-9a-zA-Z]*
wspace	[ \t\r\n]+

%%

class   {return scanned(CLASS);}
extends   {return scanned(EXTENDS);} 
main  {return scanned(MAIN);}
static {return scanned(STATIC);}
new {return scanned(NEW);}

instanceof {return scanned(INSTANCEOF);}
printNat  {return scanned(PRINTNAT);}
readNat  {return scanned(READNAT);}
nat   {return scanned(NATTYPE);}
bool   {return scanned(BOOLTYPE);}

this  {return scanned(THIS);}
null  {return scanned(NUL);}
true  {return scanned(TRUELITERAL);}
false  {return scanned(FALSELITERAL);}
else  {return scanned(ELSE);}
if  {return scanned(IF);}
for  {return scanned(FOR);}

"!"  {return scanned(NOT);}
"&&"  {return scanned(AND);}
">"   {return scanned(GREATER);}
"=="   {return scanned(EQUALITY);}
"*"   {return scanned(TIMES);}
"-"   {return scanned(MINUS);}
"+"   {return scanned(PLUS);}
"="   {return scanned(ASSIGN);}
"{"   {return scanned(LBRACE);}
"}"   {return scanned(RBRACE);}
"("   {return scanned(LPAREN);}
")"   {return scanned(RPAREN);}
";"   {return scanned(SEMICOLON);}
"."   {return scanned(DOT);}

"//"  { char c = input(); while(c!='\n' && c!=EOF) c=input();}
{wspace}  { /*skip whitespace*/ }

{digit}+  {return scanned(NATLITERAL);}

{id}  {return scanned(ID);}

<<EOF>>   {return scanned(ENDOFFILE);}
.   { if(FLEX_DEBUG) printf("\n");
        printf("Lex error on line %d: Illegal character %s\n", yylineno,yytext);
        exit(-1);
    }

%%

/* process and return scanned token t */
Token scanned(Token t) {
  if(FLEX_DEBUG==0) return t;
  switch(t) {
    case CLASS: printf("CLASS "); return t;
    case EXTENDS: printf("EXTENDS "); return t;
    case MAIN: printf("MAIN "); return t;
    case NATTYPE: printf("NATTYPE "); return t;
    case BOOLTYPE: printf("BOOLTYPE "); return t;
    case TRUELITERAL: printf("TRUELITERAL "); return t;
    case FALSELITERAL: printf("FALSELITERAL "); return t;
    case AND: printf("AND "); return t;
    case NOT: printf("NOT "); return t;
    case IF: printf("IF "); return t;
    case ELSE: printf("ELSE "); return t;
    case FOR: printf("FOR "); return t;
    case PRINTNAT: printf("PRINTNAT "); return t;
    case READNAT: printf("READNAT "); return t;
    case PLUS: printf("PLUS "); return t;
    case MINUS: printf("MINUS "); return t;
    case TIMES: printf("TIMES "); return t;
    case EQUALITY: printf("EQUALITY "); return t;
    case GREATER: printf("GREATER "); return t;
    case STATIC: printf("STATIC "); return t;
    case ASSIGN: printf("ASSIGN "); return t;
    case NUL: printf("NUL "); return t;
    case NEW: printf("NEW "); return t;
    case THIS: printf("THIS "); return t;
    case DOT: printf("DOT "); return t;
    case INSTANCEOF: printf("INSTANCEOF "); return t;
    case SEMICOLON: printf("SEMICOLON "); return t;
    case LBRACE: printf("LBRACE "); return t;
    case RBRACE: printf("RBRACE "); return t;
    case LPAREN: printf("LPAREN "); return t;
    case RPAREN: printf("RPAREN "); return t;
    case ENDOFFILE: printf("ENDOFFILE "); return t;
    case ID: printf("ID(%s) ",yytext); return t;
    case NATLITERAL: printf("NATLITERAL(%s) ",yytext); return t;
    default: printf("ERROR: invalid token in scanned().\n"); exit(-1);
  }
}

