/* DJ Symbol Table */
/* Carlos Leon */
/* I Changed the typeNameToNumber definition */

#include <stdio.h>
#include <string.h>
#include "symtbl.h"

#define NullCheck(s) if(s==NULL){printf("Error In calloc\n");exit(-1);} 

// HELPER FUNCTIONS

/* Return the number of children an AST node has.
 Note: Children with NULL data are not counted. */
int countChildren(ASTree *parent){
	if(parent == NULL){
		return 0;
	}

	int count = 0;
	for(ASTList* t = parent->children; t != NULL; t = t->next){
		if(t->data != NULL){
			count++;
		}
	}
	return count;
}

// Checks for class
int typeNameToNumber(ASTree* t){
	char* typeName = t->idVal;

	if(t->typ == BOOL_TYPE){
		return -2;
	}
	if(t->typ == NAT_TYPE){
		return -1;
	}

	if(typeName == NULL){
		return -5;
	}

	if(TYPE_DEBUG){
		printf("Looking for type: %s\n", typeName);
	}	

	for(int i=0; i < numClasses; i++){
		if(!strcmp(typeName, classesST[i].className)){
			return i;
		}
	}

	return -5;
}

// HELPER END

// MAIN FUNCTIONS

/* Sets up a new ClassST with an object class*/
void initClassST(){
	classesST = (ClassDecl*) calloc(numClasses, sizeof(ClassDecl));
	NullCheck(classesST);

	classesST[0].className = "Object";
	classesST[0].classNameLineNumber = -1;
	classesST[0].superclass = -4;
	classesST[0].superclassLineNumber = -1;
	classesST[0].numStaticVars = 0;
	classesST[0].staticVarList = NULL; 
	classesST[0].numVars = 0; 
	classesST[0].varList = NULL;
	classesST[0].numMethods = 0; 
	classesST[0].methodList = NULL;
}

/*Adds in variable declerations, also works for static variables*/
void initVarDECL(VarDecl *varList, ASTree* decls, int totalDecls){
	ASTList* currChild = decls->children;
	for(int i=0; i < totalDecls; i++){
		ASTree* decl = currChild->data;
		
		// First child is the type of variable
		ASTList* declChildren = decl->children; // there is a bug here
  		varList[i].type = typeNameToNumber(declChildren->data);
  		varList[i].typeLineNumber = declChildren->data->lineNumber;

  		// Second child is the id if the variable
  		declChildren = declChildren->next;
		varList[i].varName = declChildren->data->idVal;
		varList[i].varNameLineNumber = declChildren->data->lineNumber;

		if(TYPE_DEBUG){
			printf("\tFound variable %s of type %d\n", varList[i].varName, varList[i].type);
		}	

		currChild = currChild->next;
	}
}

/*Adds in method declerations*/
void initMethodDECL(MethodDecl *methodList, ASTree* decls, int totalDecls){
	ASTList* currChild = decls->children;
	for(int i=0; i < totalDecls; i++){
		ASTree* decl = currChild->data;
		
		// First child is the return type
		ASTList* declChildren = decl->children;
		methodList[i].returnType = typeNameToNumber(declChildren->data);
		methodList[i].returnTypeLineNumber = declChildren->data->lineNumber;

		// Second child is the method name
		declChildren = declChildren->next;
		methodList[i].methodName = declChildren->data->idVal;
		methodList[i].methodNameLineNumber = declChildren->data->lineNumber;

		if(TYPE_DEBUG){
			printf("\tFound method %s\n", methodList[i].methodName);
		}

		// Third child is param type
		declChildren = declChildren->next;
		methodList[i].paramType = typeNameToNumber(declChildren->data);
		methodList[i].paramTypeLineNumber = declChildren->data->lineNumber;

		// Fourth child is param name
		declChildren = declChildren->next;
		methodList[i].paramName = declChildren->data->idVal;
		methodList[i].paramNameLineNumber = declChildren->data->lineNumber;

		// fifth child is variable decleration list
		declChildren = declChildren->next;
		methodList[i].numLocals = countChildren(declChildren->data);
		if(TYPE_DEBUG){
			printf("\tMethod %s has %d locals\n", methodList[i].methodName, methodList[i].numLocals);
		}

		if(methodList[i].numLocals != 0){
			methodList[i].localST = (VarDecl*) calloc(methodList[i].numLocals, sizeof(VarDecl));
			NullCheck(methodList[i].localST);
			initVarDECL(methodList[i].localST, declChildren->data, methodList[i].numLocals);
		}	 

		// Sixth child is expression list
		declChildren = declChildren->next;
		methodList[i].bodyExprs = declChildren->data;

		currChild = currChild->next;
	}
}

/*Adds in a decleration for a class and triggers regular and method declerations*/
void initClassDECL(int i, ASTree* aClass){
	// A Class has 5 children, first child is id expression i.e. class name
	ASTList* currChild = aClass->children->next;

	// Second child is id expression
	int super = typeNameToNumber(currChild->data);
	classesST[i].superclass = super;
	if(super != -4){
		classesST[i].superclassLineNumber = classesST[super].classNameLineNumber;
	}
	else{
		classesST[i].superclassLineNumber = 0;
	}
	
	if(TYPE_DEBUG){
		printf("Class %s has super of type %d\n", classesST[i].className, classesST[i].superclass);
	}
	currChild = currChild->next;

	// Third child is statics
	classesST[i].numStaticVars = countChildren(currChild->data);
	if(TYPE_DEBUG){
		printf("Class %s has %d statics\n", classesST[i].className, classesST[i].numStaticVars);
	}

	if(classesST[i].numStaticVars != 0){
		classesST[i].staticVarList = (VarDecl*) calloc(classesST[i].numStaticVars, sizeof(VarDecl));
		NullCheck(classesST[i].staticVarList);
		initVarDECL(classesST[i].staticVarList, currChild->data, classesST[i].numStaticVars);
	}
	currChild = currChild->next;

	// Fourth Child is regular variables
	classesST[i].numVars = countChildren(currChild->data);
	if(TYPE_DEBUG){
		printf("Class %s has %d regulars\n", classesST[i].className, classesST[i].numVars);
	}

	if(classesST[i].numVars != 0){
		classesST[i].varList = (VarDecl*) calloc(classesST[i].numVars, sizeof(VarDecl));;
		NullCheck(classesST[i].varList);
		initVarDECL(classesST[i].varList, currChild->data, classesST[i].numVars);
	} 
	currChild = currChild->next;

	// Fifth Child is methods
	classesST[i].numMethods = countChildren(currChild->data);
	if(TYPE_DEBUG){
		printf("Class %s has %d methods\n", classesST[i].className, classesST[i].numMethods);
	}	 

	if(classesST[i].numMethods != 0){
		classesST[i].methodList = (MethodDecl*) calloc(classesST[i].numMethods, sizeof(MethodDecl));;
		NullCheck(classesST[i].methodList);
		initMethodDECL(classesST[i].methodList, currChild->data, classesST[i].numMethods);
	}
}

// MAIN END

// EXPOSED FUNCTIONS

void setupSymbolTables(ASTree *fullProgramAST){
	// Setup Classes
	wholeProgram = fullProgramAST;
	numClasses = 1 + countChildren(wholeProgram->children->data); 
	initClassST();
	
	if(TYPE_DEBUG){
		printf("Found %d classes\n", numClasses -1);
	}
	
	// Setup classesSt to have all the types
	ASTList* curr = wholeProgram->children->data->children;
	for(int i=1; i < numClasses; i++){
		ASTree* className = curr->data->children->data;
		classesST[i].className = className->idVal;
		classesST[i].classNameLineNumber = className->lineNumber;

		if(TYPE_DEBUG){
			printf("Class %s is now type %d\n", classesST[i].className, i);
		}

		curr = curr->next;
	}

	// Setup super, statics, variables, and methods 
	curr = wholeProgram->children->data->children;
	for(int i=1; i < numClasses; i++){
		initClassDECL(i, curr->data);
		curr = curr->next;
	}

	// Setup main locals, reglist
	ASTree* regs = wholeProgram->children->next->data;
	numMainBlockLocals = countChildren(regs);

	if(TYPE_DEBUG){
		printf("Found %d main locals\n", numMainBlockLocals);
	}

	if(numMainBlockLocals != 0){
		mainBlockST = (VarDecl*) calloc(numMainBlockLocals, sizeof(VarDecl));;
		NullCheck(mainBlockST);
		initVarDECL(mainBlockST, regs, numMainBlockLocals);
	} 

	// Setup main expressiosn
	mainExprs = wholeProgram->children->next->next->data;
}

// EXPOSED END
