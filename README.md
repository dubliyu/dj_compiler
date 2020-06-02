# DJ Compiler
## By Carlos Leon for DT.Ligatti's USF Spring Compilers Class

DJ (Diminshed Java) is a limited subset of Java that contains all the elements needed to produce an Object Orientated Language. Designed for teaching purposes, its not a practical language, but within you can find Objects, inheritence and Polymorphism. This compiler takes in DJ and produces DISM (Diminished Instruction Set Machine). A reduced instruction set containing only 12 differnt instructions. This is sufficient to relate to general assembly code without addressing architecture specific instructions and optimizations. A DISM virtual machine made by Dr. Ligatti can be found [here](https://www.cse.usf.edu/~ligatti/compilers-17/as1/dism/). 

Below is come code provided to highlight the DJ syntx from the definition document by Dr.Ligatii [here](https://www.cse.usf.edu/~ligatti/compilers-17/as1/dj/DJ-definition.pdf).

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

## Process

The making of this compiler was done in 6 Assignments. Each Assignment allowing us to become familliar with the tools necessary to produce a working compiler.

### Assignment 1

Write an example fuctionally equivelent DISM and DJ program. This is to be able to compare the resulting compiled code and to familiarize ourself with the language.
See example.dj

### Assignment 2

Use flex to produce a lexer to tokanize a source program. Lexer definition in dj.l. Flex automatically produces fast, efficient DFAs. However, a notable downsize of this lexer definition is that the compiler will exit on this first lexing error.

To generate DFA do...
```bash
flex dj.l
```

### Assignment 3 and Assignment 4

In Assignment 3, YACC produces a Concrete Syntax Tree from the tokens from a CFG. Then we augment the outputed LALR parser in Assignment 4 to add actions to built up the Abstract Syntax Tree. 
