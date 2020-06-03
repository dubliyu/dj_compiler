/* DJ PARSER */
/* Carlos Leon */
/* Fixed precedence of InstanceOf */
/* Fixed appending method declerarion to method decleration */
/* Fixed extra dot id inside dot assign expressions */


%code provides {
  #include "lex.yy.c"
  #include "ast.h"
  #include "symtbl.h"
  #include "typecheck.h"

  /* Function for printing generic syntax-error messages */
  void yyerror(const char *str) {
    printf("Syntax error on line %d at token %s\n",yylineno,yytext);
    printf("(This version of the compiler exits after finding the first ");
    printf("syntax error.)\n");
    exit(-1);
  }

    #define CFG_DEBUG 0
    #define YYSTYPE ASTree *
    
    ASTree *pgmAST;
    ASTree *tempAST;
}

%token CLASS ID EXTENDS MAIN NATTYPE BOOLTYPE
%token TRUELITERAL FALSELITERAL AND NOT IF ELSE FOR
%token NATLITERAL PRINTNAT READNAT PLUS MINUS TIMES EQUALITY GREATER
%token STATIC ASSIGN NUL NEW THIS DOT INSTANCEOF
%token SEMICOLON LBRACE RBRACE LPAREN RPAREN
%token ENDOFFILE

%right ASSIGN
%left AND
%nonassoc EQUALITY
%nonassoc GREATER
%left PLUS MINUS
%left TIMES
%right NOT
%nonassoc INSTANCEOF
%left DOT

%start pgm

%%

pgm   : source ENDOFFILE 
      { pgmAST = $1; return 0; }
      ;

source: classlist MAIN LBRACE reglist expressionlist RBRACE
      { 
        $$ = newAST(PROGRAM, $1, 0, 0, yylineno);
        appendToChildrenList($$, $4); 
        appendToChildrenList($$, $5); 
      }
      ;

classlist : classlist CLASS justanid EXTENDS justanid LBRACE staticlist reglist methodlist RBRACE
          { 
            tempAST = newAST(CLASS_DECL, $3, 0, NULL, yylineno);
            appendToChildrenList(tempAST, $5);
            appendToChildrenList(tempAST, $7);
            appendToChildrenList(tempAST, $8);
            appendToChildrenList(tempAST, $9);
            $$ = appendToChildrenList($1, tempAST);
          }
          |
          { $$ = newAST(CLASS_DECL_LIST, NULL, 0, NULL, yylineno);}
          ;

staticlist  : staticlist STATIC typeoption justanid SEMICOLON
            {
              tempAST = newAST(STATIC_VAR_DECL, $3, 0, NULL, yylineno);
              appendToChildrenList(tempAST, $4);
              $$ = appendToChildrenList($1, tempAST);
            }
            |
            { $$ = newAST(STATIC_VAR_DECL_LIST, NULL, 0, NULL, yylineno);}
            ;

methodlist  : moremethods
            {
              $$ = $1; 
            }
            |
            { $$ = newAST(METHOD_DECL_LIST, NULL, 0, NULL, yylineno);}
            ;

moremethods   : moremethods methoddecl
              { 
                appendToChildrenList($$, $2);
              }
              | methoddecl
              { 
                $$ = newAST(METHOD_DECL_LIST, $1, 0, NULL, yylineno);
              }
              ;

methoddecl : typeoption justanid LPAREN typeoption justanid RPAREN LBRACE reglist expressionlist RBRACE
              {
                $$ = newAST(METHOD_DECL, $1, 0, NULL, yylineno);
                appendToChildrenList($$, $2);
                appendToChildrenList($$, $4);
                appendToChildrenList($$, $5);
                appendToChildrenList($$, $8);
                appendToChildrenList($$, $9);
              }
              ;

reglist : reglist typeoption justanid SEMICOLON
        { 
          tempAST = newAST(VAR_DECL, $2, 0, NULL, yylineno);
          appendToChildrenList(tempAST, $3);
          $$ = appendToChildrenList($1, tempAST); 
        }
        |
        { $$  = newAST(VAR_DECL_LIST, NULL, 0, NULL, yylineno);}
        ;

typeoption  : NATTYPE
            { $$ = newAST(NAT_TYPE, NULL, 0, NULL, yylineno);}
            | BOOLTYPE
            { $$ = newAST(BOOL_TYPE, NULL, 0, NULL, yylineno);}
            | justanid
            ;

expressionlist  : expressionlist expr SEMICOLON
                { $$ = appendToChildrenList($1, $2); }
                | expr SEMICOLON
                { $$ = newAST(EXPR_LIST, $1, 0, NULL, yylineno);}
                ;

expr  : ifexpr
      { $$ = $1;}
      | forexpr
      { $$ = $1;}
      | plusexpr
      { $$ = $1;}
      | minusexpr
      { $$ = $1;}
      | timexexpr
      { $$ = $1;}
      | equalexpr
      { $$ = $1;}
      | greaterexpr
      { $$ = $1;}
      | notexpr
      { $$ = $1;}
      | andexpr
      { $$ = $1;}
      | instanceexpr
      { $$ = $1;}
      | printexpr
      { $$ = $1;}
      | readexpr
      { $$ = $1;}
      | assignmentexpr
      { $$ = $1;}
      | methodexpr
      { $$ = $1;}
      | nestedexpr
      { $$ = $1;}
      | constructexpr
      { $$ = $1;}
      | dotexpr
      { $$ = $1;}
      | basecaseexpr
      { $$ = $1;}
      ;

ifexpr  : IF LPAREN expr RPAREN LBRACE expressionlist RBRACE ELSE LBRACE expressionlist RBRACE
        { 
          $$ = newAST(IF_THEN_ELSE_EXPR, $3, 0, NULL, yylineno);
          appendToChildrenList($$, $6);
          appendToChildrenList($$, $10);
        }
        ;

forexpr : FOR LPAREN expr SEMICOLON expr SEMICOLON expr RPAREN LBRACE expressionlist RBRACE
        { 
          $$ = newAST(FOR_EXPR, $3, 0, NULL, yylineno);
          appendToChildrenList($$, $5);
          appendToChildrenList($$, $7);
          appendToChildrenList($$, $10);
        }
        ;

plusexpr  : expr PLUS expr
          { 
            $$ = newAST(PLUS_EXPR, $1, 0, NULL, yylineno);
            appendToChildrenList($$, $3);
          }
          ;

minusexpr : expr MINUS expr
          {
            $$ = newAST(MINUS_EXPR, $1, 0, NULL, yylineno);
            appendToChildrenList($$, $3);
          }
          ;

timexexpr : expr TIMES expr
          { 
            $$ = newAST(TIMES_EXPR, $1, 0, NULL, yylineno);
            appendToChildrenList($$, $3);
          }
          ;

equalexpr : expr EQUALITY expr
          { 
            $$ = newAST(EQUALITY_EXPR, $1, 0, NULL, yylineno);
            appendToChildrenList($$, $3);
          }
          ;

greaterexpr : expr GREATER expr
            { 
              $$ = newAST(GREATER_THAN_EXPR, $1, 0, NULL, yylineno);
              appendToChildrenList($$, $3);
            }
            ;

notexpr : NOT expr
        { $$ = newAST(NOT_EXPR, $2, 0, NULL, yylineno);}
        ; 

andexpr : expr AND expr
        { 
          $$ = newAST(AND_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
        ;

instanceexpr  : expr INSTANCEOF expr
              { 
                $$ = newAST(INSTANCEOF_EXPR, $1, 0, NULL, yylineno);
                appendToChildrenList($$, $3);
              }
              ;

printexpr : PRINTNAT LPAREN expr RPAREN
          { $$ = newAST(PRINT_EXPR, $3, 0, NULL, yylineno);}
          ;

readexpr  : READNAT LPAREN RPAREN
          { $$ = newAST(READ_EXPR, NULL, 0, NULL, yylineno);}
          ;

assignmentexpr  : justanid ASSIGN expr
                { 
                  $$ = newAST(ASSIGN_EXPR, $1, 0, NULL, yylineno);
                  appendToChildrenList($$, $3);
                }
                | expr DOT justanid ASSIGN expr
                { 
                  $$ = newAST(DOT_ASSIGN_EXPR, $1, 0, NULL, yylineno);
                  appendToChildrenList($$, $3);
                  appendToChildrenList($$, $5);
                }
                ;

methodexpr  : justanid LPAREN expr RPAREN
            { 
              $$ = newAST(METHOD_CALL_EXPR, $1, 0, NULL, yylineno);
              appendToChildrenList($$, $3);
            }
            | expr DOT justanid LPAREN expr RPAREN
            { 
              $$ = newAST(DOT_METHOD_CALL_EXPR, $1, 0, NULL, yylineno);
              appendToChildrenList($$, $3);
              appendToChildrenList($$, $5);
            }
            ;

nestedexpr  : LPAREN expr RPAREN
            {$$ = $2;}
            ;

constructexpr : NEW justanid LPAREN RPAREN
              { $$ = newAST(NEW_EXPR, $2, 0, NULL, yylineno);}
              ;

basecaseexpr  : NATLITERAL
              {$$ = newAST(NAT_LITERAL_EXPR, NULL, atoi(yytext), NULL, yylineno);}
              | TRUELITERAL
              {$$ = newAST(TRUE_LITERAL_EXPR, NULL, 0, NULL, yylineno);}
              | FALSELITERAL
              {$$ = newAST(FALSE_LITERAL_EXPR, NULL, 0, NULL, yylineno);}
              | NUL
              {$$ = newAST(NULL_EXPR, NULL, 0, NULL, yylineno);}
              | justanid
              {$$ = newAST(ID_EXPR, $1, 0, NULL, yylineno);}
              | THIS
              {$$ = newAST(THIS_EXPR, NULL, 0, NULL, yylineno);}
              ;

justanid  : ID
          { $$ = newAST(AST_ID, NULL, 0, yytext, yylineno); }
          ;

dotexpr : expr DOT justanid
        {
          $$ = newAST(DOT_ID_EXPR, $1, 0, NULL, yylineno);
          appendToChildrenList($$, $3);
        }
        ;

%%

int main(int argc, char **argv) {
  if(argc!=2) {
    printf("Usage: dj-parse filename\n");
    exit(-1);
  }
  yyin = fopen(argv[1],"r");
  if(yyin==NULL) {
    printf("ERROR: could not open file %s\n",argv[1]);
    exit(-1);
  }
  /* parse the input program */
  unsigned int out = yyparse();

  if(CFG_DEBUG){
    printAST(pgmAST);
  }

  setupSymbolTables(pgmAST);
  typecheckProgram();

  return out;
}

