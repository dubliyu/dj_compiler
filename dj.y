/* 
  File provided by Dr. Ligatti.
*/

/* This bison-input file simply declares tokens for DJ programs
     (enumerated below on all the lines that begin with %token)
     and has a main funtion to repeatedly invoke the lexer.
*/

%code provides {
  #include "lex.yy.c"

  /* Function for printing generic syntax-error messages */
  void yyerror(const char *str) {
    printf("Syntax error on line %d at token %s\n",
           yylineno,yytext);
    printf("Halting compiler.\n");
    exit(-1);
  }
}

%token CLASS ID EXTENDS MAIN NATTYPE BOOLTYPE
%token TRUELITERAL FALSELITERAL AND NOT IF ELSE FOR
%token NATLITERAL PRINTNAT READNAT PLUS MINUS TIMES EQUALITY GREATER
%token STATIC ASSIGN NUL NEW THIS DOT INSTANCEOF
%token SEMICOLON LBRACE RBRACE LPAREN RPAREN
%token ENDOFFILE

%start pgm

%%

pgm : 
    ;

%%

int main(int argc, char **argv) {
  if(argc!=2) {
    printf("Usage: dj-lex  input-filename\n");
    exit(-1);
  }
  // open the input file
  yyin = fopen(argv[1],"r");
  if(yyin==NULL) {
    printf("ERROR: could not open file %s\n",argv[1]);
    exit(-1);
  }
  // tokenize the input file
  while(yylex()!=ENDOFFILE) { }
  return 0;
}

