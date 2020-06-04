# DJ Compiler
## By Carlos Leon for Dr. Ligatti's USF Spring Compilers Class

DJ (Diminshed Java) is a limited subset of Java that contains all the elements needed to produce an Object Orientated Language. Designed for teaching purposes, its not a practical language, but within you can find Objects, inheritence and Polymorphism. This compiler takes in DJ and produces DISM (Diminished Instruction Set Machine). A reduced instruction set containing only 12 differnt instructions. This is sufficient to relate to general assembly code without addressing architecture specific instructions and optimizations. A DISM virtual machine made by Dr. Ligatti can be found [here](https://www.cse.usf.edu/~ligatti/compilers-17/as1/dism/). 

Below is example code provided to highlight the DJ syntax from the definition document by Dr.Ligatii [here](https://www.cse.usf.edu/~ligatti/compilers-17/as1/dj/DJ-definition.pdf). The relation to Java is obvious.

```java
class Summer extends Object {
	nat sum(nat n) {
 		nat toReturn;

 		while(0<n) {
 			toReturn = toReturn + n;
 			n = n - 1;
 		};
 		toReturn;
 	}
 
 }
 
main {
	Summer s;
 	s = new Summer();

	printNat( s.sum(100) );
}
```

## Phases

The making of this compiler was done in 6 Assignments, 5 of which represent a specific phase of the compilation process. The front-end is made in tryed and true compilers fashion using Flex and Bison. The back-end features a more literal translation into DISM from an AST. This is because this is not an optimizing compiler, that was beyond the scope of the project. Although, in hindsight, simple optimizations like code folding and forward propagation would greatly improve its efficiency. 

### Assignment 1

An example DJ and DISM functionally equivelent program was written. This was to be able to compare the resulting compiled code and to familiarize ourself with the language.
See example.dj

### Phase 1 - Lexing

A lexing pattern defined using regular expressions was realized by Flex (Fast Lexer). The Lexer definition are in dj.l. Flex automatically produces fast, efficient DFAs.

To generate DFA do...
```bash
flex dj.l
```

### Phase 2 and Phase 3 - Parsing

In phase 2, YACC produces a Concrete Syntax Tree from the tokens from a CFG. Then we augment the outputed LALR parser in phase 4 to add actions to built up the Abstract Syntax Tree. Since phase 3 was written before phase 4, I keep it distinct, but these are both done at the same time. The CFG recongnizes a series of tokens and then bison parser actions are done to create an AST. 

The AST is defined in terms of two mutually recursive C structs.

```C
// ASTrees contain children in ASTlists
typedef struct astlistnode {
  struct astnode *data;
  struct astlistnode *next;
} ASTList;

// ASTree
typedef struct astnode {
  ASTNodeType typ; // node type
  ASTList *children; // head
  ASTList *childrenTail;
 
  unsigned int lineNumber; // in src file

  unsigned int natVal; // in case of nat
  char *idVal; // variable name

  // Following attributes meant to be used for code generation
  unsigned int staticClassNum; 
  unsigned int isMemberStaticVar;
  unsigned int staticMemberNum;
} ASTree;
```  

To create the LALR parser do...
```bash
bison -v dj.y
sed -i '/extern YYSTYPE yylval/d' dj.tab.c
```

### Phase 4 - Typechecking + Symbol Table

The ASTs need to be validated and the symbol table need to be generated. For these steps, the logic needs to be written by hand in C as the logic pertains specifically to the language definition. DJ is a statically programming language meaning that types need to be checked for all expressions. As polymorphism is allowed, objects of a certain type can contain within objects of a parent type i.e. subtypes are allowed.

There are at least two symbol tables: one for main locals and one for classes. The main locals might be empty. But otherwise they contain the variable declerations of main block in the former and the variable, static, and method declerations in the latter. The classes symbol table always at least has the object class type.

### Assignment 6 - Code Generation

With the valid ASTs and the symbol tables we generate DISM code from it. The Virtual machine for DISM has 8 registers; 4 of which are for general purpose use. Specifically R5 is treated as the Heap pointer, R6 as the stack pointer, and R7 as the frame pointer. Static variables are at the bottom of the memory space and the heap follows that growing upward. The stack starts at the top (64k) and grows downward. Since methods can be overwritten in DJ, variable calls first go through a virtual addressing table. Similarlly a instanceOf table is computed for calls to the operation to check subtypes.

The other expressions are codegened much more literally, but still maintaining the invarient that each expression only leaves one thing atop the stack.

To Compile the entire dj2dism compiler...
```bash
gcc dj.tab.c ast.c symtbl.c typecheck.c codegen.c -o dj2dism 
```