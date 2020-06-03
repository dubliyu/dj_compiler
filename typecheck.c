/* DJ TypeChecking nonsense Implementation */
/* I pledge my Honor that I have not cheated, and will not cheat, on this assignment */
/* Carlos Leon */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typecheck.h"

/*
 Helpful Error Functions
*/
#define InternalErrorCheck(s) if(s==NULL){printf("Internal Pointer is NULL\n");exit(-1);} 
void printTypeErrorAndQuit(char* err, int li){
	printf("Semantic analysis error on Line %d\n\t%s\n", li, err);
	exit(-1);
}
/*
Returns
0 	: if sub is not a subtype of super
> 0	: if sub is a subtype of super, then int is subtype
*/
int isSubtype(int sub, int super){
	// No illigal types
	if(sub < -4 || super < -4){
		return 0;
	}

	// Every type is a subtype of itself
	if(sub == super){
		// In the case of object and object, this is a subtype so return 1 instead
		if(sub == 0 && super == 0){
			return 1;
		}
		return sub;
	}

	// nat and bools are only subtypes of themselves
	if(sub == -1 || super == -1 || sub == -2 || sub == -2){
		return 0;
	}

	// Null is subtype of every class
	if(sub == -3 && super >= 0){
		return sub;
	}

	// Check classes 
	int curr = sub;
	int count = 0;
	while(curr != super && curr != -4 && count++ < numClasses){
		curr = classesST[curr].superclass;
	}

	if(curr == super){
		return sub;	
	}
	else{
		return 0;
	}
}

/*
 Returns the least upper boind of two types
*/
int join(int t1, int t2){
	// the join of a class with itself is itself
	if(t1 == t2){
		return t1;
	}
	// Null is a subtype of the object class
	else if(t1 == -3 && t2 >= 0){
		return t2;
	}
	else if(t1 >= 0 && t2 == -3){
		return t1;
	}
	else if(isSubtype(t1, t2)){
		return t1;
	}
	else if(isSubtype(t2, t1)){
		return t2;
	}
	else{
		while(t1 != -4){
			if(isSubtype(t2, t1)){
				return t1;
			}
			else{
				t1 = classesST[t1].superclass;
			}
		}
	}
}

void updateAST(ASTree* t, int staticClassNum, int isMemberStaticVar, int staticMemberNum){
	t->staticClassNum = staticClassNum;
	t->isMemberStaticVar = staticClassNum;
	t->staticMemberNum = staticMemberNum;
}

int classnameExists(char* comp){
	if(comp == NULL){
		return -1;
	}

	for(int i=0; i < numClasses; i++){
		if(strcmp(classesST[i].className, comp) == 0){
			return i;
		}
	}
	return -1;
}

int methodExists(char* comp, int classInt){
	if(comp == NULL){
		return -1;
	}

	while(classInt > 0){
		MethodDecl* mlist = classesST[classInt].methodList;
		int size = classesST[classInt].numMethods;
		
		for(int i=0; i < size; i++){
			if(strcmp(mlist[i].methodName, comp) == 0){
				return i;
			}
		}
		classInt = classesST[classInt].superclass;
	}
	return -1;
}

int checkIDexists(VarDecl* li, int size, char* find){
	if(find == NULL){
		return -6;
	}

	for(int i=0; i < size; i++){
		if(strcmp(li[i].varName, find) == 0){
			return li[i].type;
		}
	}
	return -6;
}

int getIDpos(VarDecl* li, int size, char* find){
	if(find == NULL){
		return -1;
	}

	for(int i=0; i < size; i++){
		if(strcmp(li[i].varName, find) == 0){
			return i;
		}
	}
	return -1;
}

void validateVarDecl(VarDecl* li, int size){
	for(int k=0; k < size; k++){
		// Confirm valid types
		if(li[k].type < -3 && li[k].type < numClasses){
			printTypeErrorAndQuit("Invalid variable type", li[k].typeLineNumber);
		}

		// Check for duplicates
		char* a = li[k].varName;
		for(int p= k+1; p < size; p++){
			char* b = li[p].varName;
			if(strcmp(a,b) == 0){
				printTypeErrorAndQuit("Variable duplicate found", li[p].varNameLineNumber);
			}
		}
	}  
}

void validateMethods(MethodDecl* li, int size, int classInt){
	for(int k=0; k < size; k++){
		// STEP 3.1 Check Return type
		if(li[k].returnType < -3){
			printTypeErrorAndQuit("Invalid return type", li[k].returnTypeLineNumber);
		}

		// STEP 3.2 Check param type
		if(li[k].paramType < -3){
			printTypeErrorAndQuit("Invalid param type", li[k].paramNameLineNumber);
		}
		else{
			// Check special case of paramname being redicalred as variable
			char* a = li[k].paramName;
			for(int y=0; y< li[k].numLocals; y++){
				char* b = li[k].localST[y].varName;
				if(strcmp(a,b) == 0){
					printTypeErrorAndQuit("The parameter name cannot be rediclared", li[k].localST[y].varNameLineNumber);
				}
			}
		}

		// STEP 3.3 Check locals are valid types
		validateVarDecl(li[k].localST, li[k].numLocals);

		// STEP 3.4 Check method names within class are unique
		char* a = li[k].methodName;
		for(int p=k+1; p < size; p++){
			char* b = li[p].methodName;
			if(strcmp(a,b) == 0){
				printTypeErrorAndQuit("Method names must be unique", li[p].methodNameLineNumber);
			}
		}

		// STEP 3.5 Check overriden methods have same return and parameter type
		int curr = classesST[classInt].superclass;
		int stop_loop = 0;
		while(curr != 0 && !stop_loop){
			// Check if method name matches
			MethodDecl* li_curr = classesST[curr].methodList;
			int size = classesST[curr].numMethods;

			for(int p=0; p < size && !stop_loop; p++){
				char* b = li_curr[p].methodName;
				if(strcmp(a,b) == 0){
					if(li_curr[p].returnType != li[k].returnType){
						printTypeErrorAndQuit("overriden methods must have the same return type", li[k].returnTypeLineNumber);
					}else if (li_curr[p].paramType != li[k].paramType){
						printTypeErrorAndQuit("overriden methods must have the same param type", li[k].paramTypeLineNumber);
					}else{
						stop_loop = 1;
					}
				}
			}

			// Move up
			curr = classesST[curr].superclass;
		}

		// STEP 3.6 Validate Method body expressions
		int foundReturnType = typeExprs(li[k].bodyExprs, classInt, k);
		if(isSubtype(foundReturnType, li[k].returnType) == 0){
			printTypeErrorAndQuit("Method return type does not match expression return type", li[k].returnTypeLineNumber);
		}
	}  
}


// MAIN FUNCTIONS
void typecheckProgram(){
	for(int i=0; i < numClasses; i++){
		char* a = classesST[i].className;
		for(int j= i+1; j < numClasses; j++){
			char* b = classesST[j].className;
			// STEP 1 Check class names are unique
			if(strcmp(a,b) == 0){
				printTypeErrorAndQuit("Class names must be unique", classesST[j].classNameLineNumber);
			}
		}
		// Skip superclass, variable, and method checks for Object class
		if(i == 0){
			continue;
		}

		// STEP 2.1 Confirm valid superclass
		if(classesST[i].superclass < 0 || classesST[i].superclass >= numClasses){
			printTypeErrorAndQuit("Superclass not defined", classesST[i].classNameLineNumber);
		}

		// STEP 2.2 Check classes are not their own superclass
		if(classesST[i].superclass == i){
			printTypeErrorAndQuit("A Class cannot be their own superclass", classesST[i].superclassLineNumber);
		}

		// STEP 2.3 Check for cycle in hierarchy
		if(isSubtype(classesST[i].superclass, i)){
			printTypeErrorAndQuit("Class hierchy must be acyclic", classesST[i].classNameLineNumber);
		}
		// STEP 2.4 Check regular variables have proper return types
		validateVarDecl(classesST[i].varList, classesST[i].numVars);

		// STEP 2.5 Check regular variables have proper return types
		validateVarDecl(classesST[i].staticVarList, classesST[i].numStaticVars);

		// STEP 2.6 Check no duplicates in variable list and static
		for(int p=0; p<classesST[i].numVars; p++){
			char* a = classesST[i].varList[p].varName;
			for(int f=0; f < classesST[i].numStaticVars; f++){
				char* b = classesST[i].staticVarList[f].varName;
				if(strcmp(a,b) == 0){
					printTypeErrorAndQuit("Cannot Redeclare Static variable with same name", classesST[i].staticVarList[f].varNameLineNumber);
				}
			}
		}
		// STEP 2.7 Check no rediclared variables in superclass
		int curr = classesST[i].superclass;
		int res = 0;
		char* find;
		int count = 0;
		while(curr != 0 && count++ < numClasses){
			VarDecl* reg_li = classesST[curr].varList;
			int reg_size = classesST[curr].numVars;
			VarDecl* static_li = classesST[curr].staticVarList;
			int static_size = classesST[curr].numStaticVars;

			for(int w=0; w< classesST[i].numVars; w++){
				find = classesST[i].varList[w].varName;
				res = checkIDexists(reg_li, reg_size, find);
				if(res != -6){
					printTypeErrorAndQuit("Superclass variable name cannot be reused", classesST[i].varList[w].varNameLineNumber);
				}
				res = checkIDexists(static_li, static_size, find);
				if(res != -6){
					printTypeErrorAndQuit("Superclass static variable name cannot be reused", classesST[i].staticVarList[w].varNameLineNumber);
				}
			}
			for(int w=0; w< classesST[i].numStaticVars; w++){
				find = classesST[i].staticVarList[w].varName;
				res = checkIDexists(reg_li, reg_size, find);
				if(res != -6){
					printTypeErrorAndQuit("Superclass variable name cannot be reused", classesST[i].varList[w].varNameLineNumber);
				}
				res = checkIDexists(static_li, static_size, find);
				if(res != -6){
					printTypeErrorAndQuit("Superclass static variable name cannot be reused", classesST[i].staticVarList[w].varNameLineNumber);
				}
			}

			// Move up
			curr = classesST[curr].superclass;
		}
	
		// STEP 3 Check Methods
		validateMethods(classesST[i].methodList, classesST[i].numMethods, i);
	}

	// STEP 4.1 Check regular main variables are valid
	validateVarDecl(mainBlockST, numMainBlockLocals);

	// STEP 4.2 Check main expressions are valid
	ASTree* main_expr = 	wholeProgram->children->next->next->data;	
	typeExprs(main_expr, -1, -1);	
}

int typeExpr(ASTree *t, int classContainingExpr, int methodContainingExpr){
	InternalErrorCheck(t);
	
	int c = classContainingExpr;
	int m = methodContainingExpr;
	int res1;
	int res2;
	int id_type;
	int expr_type;
	int class_type;
	int size;
	int param_type;
	char* name;
	ASTList* temp;
	ASTree* child;
	MethodDecl* mlist;
	VarDecl* li;
	MethodDecl method; 

	/*
		USER DEFINED CLASS > 0
		OBJECT 0
		NATURAL NUMBER -1
		BOOLEAN -2
		NULL -3
		OBJECT SUPERCLASS -4
		UNDEFINED < -4
	*/

	switch(t->typ){
		case READ_EXPR:
		case NAT_LITERAL_EXPR: return -1;
		case TRUE_LITERAL_EXPR:
		case FALSE_LITERAL_EXPR: return -2;
		case NULL_EXPR: return -3;

		case NOT_EXPR:
			if(typeExpr(t->children->data,c,m) == -2){
				return -2;
			}else{
				printTypeErrorAndQuit("Not Expression Expected boolean value", t->lineNumber);
			}
		case AND_EXPR:
			res1 = typeExpr(t->children->data,c,m);
			res2 = typeExpr(t->children->next->data,c,m);

			if(res1 == -2 && res2 == -2){
				return -2;
			}else{
				printTypeErrorAndQuit("AND Expression Expected boolean values", t->lineNumber);
			}
		case EQUALITY_EXPR:
			res1 = typeExpr(t->children->data,c,m);
			res2 = typeExpr(t->children->next->data,c,m);

			if(res1 == -1 && res2 == -1){
				return -2;
			}else if(res1 == -2 && res2 == -2){
				return -2;
			}else if(isSubtype(res1, res2) || isSubtype(res2, res1)){
				return -2;
			}else{
				printTypeErrorAndQuit("Equality Expression Expected comparible values", t->lineNumber);
			}

		case GREATER_THAN_EXPR:
			if(typeExpr(t->children->data,c,m) == -1 &&
				typeExpr(t->children->next->data,c,m) == -1){
				return -2;
			}else{
				printTypeErrorAndQuit("Greater-than Expected natural numbers", t->lineNumber);
			}
		case PLUS_EXPR:
		case MINUS_EXPR:
		case TIMES_EXPR:
			if(typeExpr(t->children->data,c,m) == -1 &&
				typeExpr(t->children->next->data,c,m) == -1){
				return -1;
			}else{
				printTypeErrorAndQuit("Expected natural numbers", t->lineNumber);
			}
		case PRINT_EXPR:
			if(typeExpr(t->children->data,c,m) == -1){
				return -1;
			}else{
				printTypeErrorAndQuit("printNat(n) expected natural number", t->lineNumber);
			}

		case THIS_EXPR:
			if(classContainingExpr > 0){
				return classContainingExpr;
			}
			else{
				printTypeErrorAndQuit("'this' must used inside a class", t->lineNumber);
			}

		case NEW_EXPR:
			res1 = classnameExists(t->children->data->idVal);
			if(res1 != -1){
				return res1;
			}else{
				printTypeErrorAndQuit("Cannot Instantiate unknown class", t->lineNumber);
			}

		case FOR_EXPR:
			temp = t->children;

			// Check init
			typeExpr(temp->data,c,m);
			temp = temp->next;

			// Check compare
			if(typeExpr(temp->data,c,m) != -2){
				printTypeErrorAndQuit("for loop test expression must be boolean", temp->data->lineNumber);
			}
			temp = temp->next;

			// check update
			typeExpr(temp->data,c,m);
			temp = temp->next;

			// check body
			typeExprs(temp->data,c,m);

			return -1;

		case IF_THEN_ELSE_EXPR:
			temp = t->children;

			if(typeExpr(temp->data,c,m) != -2){
				printTypeErrorAndQuit("if test expression must be boolean", t->lineNumber);
			}
			temp = temp->next;
			res1 = typeExprs(temp->data,c,m);
			temp = temp->next;
			res2 = typeExprs(temp->data,c,m);

			if(res1 == -1 && res2 == -1){
				return -1;
			}
			else if(res1 == -2 && res2 == -2){
				return -2;
			}
			else{
				if(res1 == -1 || res2 == -1){
					printTypeErrorAndQuit("if branch expression contains unmatched nat type", t->lineNumber);
				}
				if(res1 == -2 || res2 == -2){
					printTypeErrorAndQuit("if branch expression contains unmatched bool type", t->lineNumber);
				}
				return join(res1, res2);
			}

		case INSTANCEOF_EXPR:
			res1 = typeExpr(t->children->data,c,m);
			if(res1 < 0 && res1 != -3){
				printTypeErrorAndQuit("Instanceof expected object type", t->lineNumber);
			}

			name = t->children->next->data->idVal;
			res1 = classnameExists(name);

			if(res1 != -1){
				return -2;
			}
			else{
				printTypeErrorAndQuit("Instanceof expected class name", t->lineNumber);
			}

		case ASSIGN_EXPR:
			id_type = typeExpr(t->children->data, c, m);
			expr_type = typeExpr(t->children->next->data,c,m);
			if(isSubtype(expr_type, id_type)){
				child = t->children->data;
				updateAST(t,child->staticClassNum,child->isMemberStaticVar,child->staticMemberNum);
				child->staticClassNum = 0;
				child->isMemberStaticVar = 0;
				child->staticMemberNum = 0;
				return id_type;
			}else{
				printTypeErrorAndQuit("Mismatch type in assignment", t->lineNumber);
			}

		case DOT_ASSIGN_EXPR:
			class_type = typeExpr(t->children->data, c, m);
			if(class_type < 0){
				printTypeErrorAndQuit("Dot assignment contains unknown class", t->lineNumber);
			}

			// If this expr, check ID only against class variables
			if(t->children->data->typ == THIS_EXPR){
				id_type = typeExpr(t->children->next->data, class_type, -1);
			}
			else{
				id_type = typeExpr(t->children->next->data, class_type, m);
			}

			expr_type = typeExpr(t->children->next->next->data,c,m);

			if(isSubtype(expr_type, id_type)){
				child = t->children->next->data;
				updateAST(t,child->staticClassNum,child->isMemberStaticVar,child->staticMemberNum);
				child->staticClassNum = 0;
				child->isMemberStaticVar = 0;
				child->staticMemberNum = 0;
				return id_type;
			}else{
				printTypeErrorAndQuit("Mismatch type in assignment", t->lineNumber);
			}

		case ID_EXPR:
			res1 = typeExpr(t->children->data,c,m);
			if(classContainingExpr < 0){
				updateAST(t,0,0,0);
			}
			else{
				// Pass info from ast_id and clear it
				child = t->children->data;
				updateAST(t,child->staticClassNum,child->isMemberStaticVar,child->staticMemberNum);
				child->staticClassNum = 0;
				child->isMemberStaticVar = 0;
				child->staticMemberNum = 0;
			}
			return res1;

		case DOT_ID_EXPR:
			class_type = typeExpr(t->children->data,c,m);
			if(class_type < 0){
				printTypeErrorAndQuit("Dot expression contains unknown class", t->lineNumber);
			}
			// If this expr, check ID only against class variables
			if(t->children->data->typ == THIS_EXPR){
				res1 = typeExpr(t->children->next->data, class_type, -1);
			}
			else{
				res1 = typeExpr(t->children->next->data, class_type, m);
			}
			
			child = t->children->data;
			updateAST(t,child->staticClassNum,child->isMemberStaticVar,child->staticMemberNum);
			child->staticClassNum = 0;
			child->isMemberStaticVar = 0;
			child->staticMemberNum = 0;
			return res1;

		case METHOD_CALL_EXPR:
			if(classContainingExpr < 0){
				printTypeErrorAndQuit("No Dot Method call expected class scope", t->lineNumber);
			}
			name = t->children->data->idVal;
			res1 = methodExists(name, c);
			if(res1 == -1){
				printTypeErrorAndQuit("Method not found", t->lineNumber);
			}

			param_type = typeExpr(t->children->next->data,c,m);
			if(isSubtype(param_type, classesST[c].methodList[res1].paramType)){
				updateAST(t,c,0,res1);
				return classesST[c].methodList[res1].returnType;
			}else{
				printTypeErrorAndQuit("Incorrect function paramter type", t->lineNumber);
			}

		case DOT_METHOD_CALL_EXPR:
			class_type =  typeExpr(t->children->data,c,m);
			if(class_type < 0){
				printTypeErrorAndQuit("Dot Method call expected class", t->lineNumber);
			}

			name = t->children->next->data->idVal;
			res1 = methodExists(name, class_type);
			if(res1 == -1){
				printTypeErrorAndQuit("Method not found", t->lineNumber);
			}

			param_type = typeExpr(t->children->next->next->data,c,m);
			if(isSubtype(param_type, classesST[class_type].methodList[res1].paramType)){
				updateAST(t,class_type,0,res1);
				return classesST[class_type].methodList[res1].returnType;
			}else{
				printTypeErrorAndQuit("Incorrect dot function paramter type", t->lineNumber);
			}

		case EXPR_LIST:
			return typeExprs(t,c,m);

		case AST_ID:

			if(classContainingExpr < 0){
				// inside main
				res1 = checkIDexists(mainBlockST, numMainBlockLocals, t->idVal);
				if(res1 != -6){
					updateAST(t,0,0,0);
					return res1;
				}else{
					printTypeErrorAndQuit("Local does not exists in main", t->lineNumber);
				}
			}
			else if(methodContainingExpr >= 0){
				method = classesST[c].methodList[m];
				li = method.localST;
				size = method.numLocals;
				res1 = checkIDexists(li, size, t->idVal);
				if(res1 != -6){
					updateAST(t,0,0,0);
					return res1;
				}

				// Consider case of id being param name
				if(strcmp(t->idVal, classesST[c].methodList[m].paramName) == 0){
					updateAST(t,0,0,0);
					return classesST[c].methodList[m].paramType;
				}
			}

			while(c != 0){
				// field of class, check regular vars
				li = classesST[c].varList;
				size = classesST[c].numVars;
				res1 = getIDpos(li, size, t->idVal);
				if(res1 != -1){
					updateAST(t,c,0,res1);
					return li[res1].type;
				}

				// Check statics
				li = classesST[c].staticVarList;
				size = classesST[c].numStaticVars;
				res1 = getIDpos(li, size, t->idVal);
				if(res1 != -1){
					updateAST(t,c,1,res1);
					return li[res1].type;
				}

				c = classesST[c].superclass;
			}

			printTypeErrorAndQuit("Could not find id in static or regular variables", t->lineNumber);

		default:
			printTypeErrorAndQuit("Unkown expression", t->lineNumber);
	}

}

int typeExprs(ASTree *t, int classContainingExprs, int methodContainingExprs){
	ASTList* li = t->children;
	int res = -5;

	while(li != NULL){
		res = typeExpr(li->data, classContainingExprs, methodContainingExprs);
		li = li->next;
	}

	return res;
}
