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

The making of this compiler was done in 6 steps. Each step allowing us to become familliar with the tools necessary to produce a working compiler.

### Step 1

Write an example fuctionally equivelent DISM and DJ program. This is to be able to compare the resulting compiled code and to familiarize ourself with the language.
See example.dj

### Step 2

Use flex to produce a lexer to tokanize a source program. Lexer definition in dj.l.

To run, do the the following. Then do ./dj-flex example.dj to see tokanized output.
```bash
flex dj.l
bison dj.y
gcc fj.tab.c -o dj-flex
```

### Step 3

Use YACC to produce a Concrete Syntax Tree from the tokens. YACC produces a LALR parser from a CFG defined in dj.y
