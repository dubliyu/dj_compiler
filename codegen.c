/* DJ Code Generation File */
/* I pledge my Honor that I have not cheated, and will not cheat, on this assignment */
/* Carlos Leon */

/* 
Instructions
---------------
add d s1 s2 	R[d] <- R[s1] + R[s2]
sub d s1 s2		R[d] <- R[s1] - R[s2] (R[d]<-0 when R[s2]>R[s1])
mul d s1 s2 	R[d] <- R[s1] * R[s2]
mov d n 		R[d] <- n
lod d s i 		R[d] <- M[R[s]+i]
str d i s 		M[R[d]+i] <- R[s]
jmp s i 		PC <- R[s] + i
beq s1 s2 n 	If R[s1] = R[s2] then PC <- n
blt s1 s2 n 	If R[s1] < R[s2] then PC <- n
rdn d 			Read natural number from screen into R[d]
ptn s 			Print natural number R[s] to screen
hlt s 			Halt the DISM with code R[s]

Error Codes
---------------
66 	Out of Heap Memory
77	Out of Stack Memory
88 	Null Pointer Reference
99 	Variable Not Found (vtable entry)

Common Register
---------------
R0	0, which is nat 0, false, and null
R5	Heap Pointer
R6	Stack Pointer
R7	Frame Pointer

Semantics I decided
-------------------
#1			is a branch label
#VTABLES 	is the location of the vtable
#C1M2 		is the location of method 2 in class 1
DEBUG_COMMENTS 0 or 1, write out degbug comments
*/

#include <stdio.h>
#include <string.h>
#include "typecheck.h"
#include "codegen.h"


#define MAX_DISM_ADDR 65535
#define DEBUG_COMMENTS 1
#define PTN_DEBUG() if(DEBUG_COMMENTS){fprintf(f, "lod 1 6 1\n");fprintf(f, "ptn 1\n");}
#define PTN_DEBUG_0(n) if(DEBUG_COMMENTS){fprintf(f, "ptn %d\n", n);}
#define PTN_DEBUG_N(n) if(DEBUG_COMMENTS){fprintf(f, "lod 1 6 %d\n",n);fprintf(f, "ptn 1\n");}

// Globals
FILE* f;
unsigned int labelNum = 0;
unsigned int instanceOfFound = 0;
unsigned int methodFound = 0;

// Helper Functions
void printInternalErrorAndQuit(char* err, int lineNo){
	printf("Internal Code Generation Error for Line %d :\n\t%s\n", lineNo, err);
	exit(-1);
}
int GetClassType(char* name){
	for(int i=0; i < numClasses; i++){
		if(strcmp(classesST[i].className, name) == 0){
			return i;
		}
	}
	return -1;
}
int GetFindVariable(char* comp, int size, VarDecl* li, int must, ASTree* t){
	if(comp == NULL){
		printInternalErrorAndQuit("Cannot Find null ID", t->lineNumber);
	}

	for(int i=0; i < size; i++){
		if(strcmp(comp, li[i].varName) == 0){
			return i;
		}
	}
	if(must){
		printInternalErrorAndQuit("No variable found", t->lineNumber);
	}
	return -1;
}
int GetStaticOffset(ASTree* t, int currentClass){
	// Every class gets a section of the heap for its static
	// in order, so we increment until we reach this classes
	// section, the its just the staticMemberNum pass that
	// Plus 1 since the first address (0) is null
 	int offset = 1;
 	int i;
	for(i=0; i < numClasses; i++){
		offset += classesST[i].numStaticVars;
		if(i == currentClass){
			break;
		}
	}
	if(i == numClasses){
		printInternalErrorAndQuit("No static found", t->lineNumber);
	}
	offset += t->staticMemberNum;
	return offset;
}
int CountFields(int classInt){
	int ret = 0;

	while(classInt != 0){
		ret += classesST[classInt].numVars;
		classInt = classesST[classInt].superclass;
	}

	return ret;
}
int isChildOfClass(int parent, int child){
	if(child == parent){
		return 1;
	}
	if(child == 0 || parent == 0){
		return 0;
	}
	return isChildOfClass(parent, classesST[child].superclass);
}
int doesChildOverrideMethod(int parent, int child, int method){
	char* name = classesST[parent].methodList[method].methodName;
	for(int i=0; i < classesST[child].numMethods; i++){
		if(strcmp(name, classesST[child].methodList[i].methodName) == 0){
			return i;
		}
	}
	return -1;
}

// Generate Snippets of DISM
void _log(char* msg){
	if(DEBUG_COMMENTS){
		printf("Writting out msg: %s\n", msg);
		fprintf(f, "mov 0 0; %s\n", msg); 
	}
}
void decSP(){
	fprintf(f, "mov 1 1\n"); 
	fprintf(f, "sub 6 6 1\n"); 
	fprintf(f, "blt 5 6 #%d\n", labelNum); 
	fprintf(f, "mov 1 77\n"); 
	fprintf(f, "hlt 1\n"); 
	fprintf(f, "#%d: mov 0 0\n", labelNum++); 
} 
void incSP(){
	fprintf(f, "mov 1 1\n"); 
	fprintf(f, "add 6 6 1\n"); 
}
void incHP(){
	int branch1 = labelNum++;
	fprintf(f, "mov 1 1\n");
	fprintf(f, "add 5 1 5\n");
	fprintf(f, "blt 5 6 #%d\n", branch1); 
	fprintf(f, "mov 1 66\n"); 
	fprintf(f, "hlt 1\n"); 
	fprintf(f, "#%d: mov 0 0\n", branch1);
} 
void checkNullDereference(){
	int branch1 = labelNum++;
	int branch2 = labelNum++;

	fprintf(f, "lod 1 6 1\n"); 
	fprintf(f, "beq 1 0 #%d\n", branch1);
	fprintf(f, "jmp 0 #%d\n", branch2);
	fprintf(f, "#%d:  mov 1 88\n", branch1);   
	fprintf(f, "hlt 1\n");
	fprintf(f, "#%d: mov 0 0\n", branch2);  
}
void fromMethodGetObjAddr(){
	//PTN_DEBUG_N(7);
	fprintf(f, "mov 1 1\n");
	fprintf(f, "sub 1 7 1\n"); // R1 = addr = FP - 1
	fprintf(f, "lod 1 1 0\n"); // R1 = M[addr]
	fprintf(f, "str 6 0 1\n"); // store atop stack
	decSP(); // decrement
}
void GetID(int returnVal, int isDot, int classInt, int methodInt, ASTree* t){
	// If returnVal == 0 return the address
	// Else return the value
	int temp;
	int size;
	VarDecl* varLi;
	char* name;

	// Check if id is main local
	if(classInt < 0 && !isDot){
		_log("Getting Main Local Start");

		// main local value is FP - local index
		temp = GetFindVariable(t->children->data->idVal, numMainBlockLocals, mainBlockST, 1, t);
		//printf("ID name is: %s OFFSET = %d\n", t->children->data->idVal, temp);
		fprintf(f, "mov 1 %d\n", temp);
		fprintf(f, "sub 1 7 1\n");

		if(returnVal){
			fprintf(f, "lod 1 1 0\n");
		}
		fprintf(f, "str 6 0 1\n");
		decSP();
		_log("Getting Main Local End");
	}
	else if(t->isMemberStaticVar != 0){
		_log("Getting Static Variable Start");
		// Static variable, calculate offset
		temp = GetStaticOffset(t,t->staticClassNum);
		//printf("Returning Value?%d for Static offset %d\n", returnVal, temp);
		if(returnVal){
			fprintf(f, "lod 1 0 %d\n", temp);
		}
		else{
			fprintf(f, "mov 1 %d\n", temp);
		}
		if(isDot){
			// Override object location atop stack
			fprintf(f, "str 6 1 1\n");
		}
		else{
			// Push new thing onto stack
			fprintf(f, "str 6 0 1\n");
			decSP();
		}
		_log("Getting Static Variable End");
	}
	else if(isDot){
		_log("Getting Object Field Start");
		// Get object location from top of stack
		fprintf(f, "lod 1 6 1\n");

		// Get offset for field of object
		size = t->staticMemberNum + 1 + CountFields(classesST[t->staticClassNum].superclass);
		//printf("e.id - Offset from  t is %d is for %s\n", size, name);
		fprintf(f, "mov 2 %d\n", size);
		fprintf(f, "sub 1 1 2\n");

		if(returnVal){
			fprintf(f, "lod 1 1 0\n");
		}
		fprintf(f, "str 6 1 1\n");
		_log("Getting Object Field End");
	}
	else{
		// If we are not in a method, error out
		if(methodInt < 0){
			printInternalErrorAndQuit("No method found", t->lineNumber);
		}

		// This could be a method parameter
		if(strcmp(t->children->data->idVal, classesST[classInt].methodList[methodInt].paramName) == 0){
			// Our value is at FP - 3 - 1
			_log("Variable is function Parameter");
			fprintf(f, "mov 1 4\n");
			fprintf(f, "sub 1 7 1\n");
			if(returnVal){
				fprintf(f, "lod 1 1 0\n");
			}
			fprintf(f, "str 6 0 1\n");
			decSP();
		}
		else{
			// This could be a method local
			_log("Considering method local");
			varLi = classesST[classInt].methodList[methodInt].localST;
			size = classesST[classInt].methodList[methodInt].numLocals;
			if(t->idVal == NULL){
				name = t->children->data->idVal;
			}
			else{
				name = t->idVal;
			}
			temp = GetFindVariable(name, size, varLi, 0, t);
			if(temp != -1){
				fprintf(f, "mov 1 %d\n", 5 + temp);
				fprintf(f, "sub 1 7 1\n");
				if(returnVal){
					fprintf(f, "lod 1 1 0\n");
				}
				fprintf(f, "str 6 0 1\n");
				decSP();
			}
			else{
				_log("Getting this.id");
				// This has to be this.id
				fprintf(f, "mov 1 1\n");
				fprintf(f, "sub 1 7 1\n");
				fprintf(f, "lod 1 1 0\n");

				// R[1] holds address of object in heap
				size = t->staticMemberNum + 1 + CountFields(classesST[t->staticClassNum].superclass);
				//printf("this.id - Offset from  t is %d is for %s\n", size, name);

				fprintf(f, "mov 2 %d\n", size);
				fprintf(f, "sub 1 1 2\n");
				if(returnVal){
					fprintf(f, "lod 1 1 0\n");
				}
				fprintf(f, "str 6 0 1\n");
				decSP();
			}
		}
	}
}
void pushAndDecrement(int isLabel, int num){
	if(isLabel){
		fprintf(f, "mov 1 #%d\n", num);
	}
	else{
		fprintf(f, "mov 1 %d\n", num);
	}
	fprintf(f, "str 6 0 1\n");
	decSP();
}
void genMainPrologue(){
	// STEP 2.1: Setup registers
	_log("Main Prologue - Start");
	fprintf(f, "mov 0 0\n"); 
	fprintf(f, "mov 5 1\n"); 
	fprintf(f, "mov 6 %d\n", MAX_DISM_ADDR); 
	fprintf(f, "mov 7 %d\n", MAX_DISM_ADDR);
	fprintf(f, "str 0 0 0\n"); 

	// STEP 2.2: Allocate Main Locals
	for(int i=0; i < numMainBlockLocals; i++){
		fprintf(f, "str 6 0 0\n");
		decSP();
	}

	// STEP 2.3: Allocate Statics, excluding object
	for(int i=1; i < numClasses; i++){
		for(int j=0; j < classesST[i].numStaticVars; j++){
			fprintf(f, "str 5 0 0\n");
			incHP();
		}
	}
	_log("Main Prologue - End");
}
void genMainEpilogue() {
	// STEP 4.1: Exit Normally
	_log("Main Epilogue - Start");
	fprintf(f, "hlt 0\n");
	_log("Main Epilogue - End");
}
void genMethodPrologue(int classNumber, int methodNumber) {
	// STEP 5.1: Add label to jump to this method
	_log("Method Prologue - Start");
	fprintf(f, "#C%dM%d: mov 0 0\n", classNumber, methodNumber); 

	//PTN_DEBUG_N(1);
	//PTN_DEBUG_N(2);
	//PTN_DEBUG_N(3);
	//PTN_DEBUG_N(4);

	// STEP 5.2: Push the method locals ontop the stack
	MethodDecl curr = classesST[classNumber].methodList[methodNumber];
	for(int i=0; i < curr.numLocals; i++){
		fprintf(f, "str 6 0 0\n");
		decSP();
	}

	// STEP 5.3: Save the old FP on the stack
	fprintf(f, "str 6 0 7\n");
	decSP();

	// STEP 5.4: Set the current FP
	// Stack - 1 = old FP
	// Stack - 1 - num locals = first local
	// Stack - 1 - num locals - 1 = argument
	// Stack - 1 - num locals - 1 - 3 = object caller 
	// Stack - 1 - num locals - 1 - 3 - 1 = return address 
	int loc = 1 + curr.numLocals + 1 + 3 + 1;
	fprintf(f, "mov 1 %d\n", loc);
	fprintf(f, "add 7 6 1\n");

	_log("Method Prologue - End");
}
void genMethodEpilogue(int classNumber, int methodNumber) {
	// STEP 7.1: Save method result, old FP, current FP
	_log("Method Epilogue - Start");
	//PTN_DEBUG_N(1);
	//PTN_DEBUG_N(2);
	//PTN_DEBUG_N(3);
	//PTN_DEBUG_N(4);
	//PTN_DEBUG_N(5);
	//PTN_DEBUG_N(6);
	//PTN_DEBUG_N(7);
	fprintf(f, "lod 1 6 1\n"); // Result
	//PTN_DEBUG_0(1);
	fprintf(f, "lod 2 6 2\n"); // Old FP
	fprintf(f, "lod 3 7 0\n"); // Return Address
	//fprintf(f, "ptn 3\n"); // Return Address
	//fprintf(f, "hlt 1\n"); // Return Address

	// STEP 7.2: Set SP to FP - 1, clear the stack
	fprintf(f, "mov 4 1\n"); 
	fprintf(f, "sub 6 7 4\n"); 

	// STEP 7.3: Store the method result
	fprintf(f, "str 7 0 1\n");
	
	// STEP 7.3: Set FP to old FP
	fprintf(f, "add 7 2 0\n");

	// STEP 7.4: Return to Callee isntruction
	//fprintf(f, "hlt 1\n"); // DELETE THIS
	fprintf(f, "jmp 3 0\n");
	_log("Method Epilogue - End");
}
void genVtable(){
	// STEP 8.1: Add VTable starting label
	_log("VTABLE - START");
	fprintf(f, "#VTABLE: mov 0 0\n");  

	// fprintf(f, "lod 1 6 3\n");
	// fprintf(f, "lod 2 6 2\n");
	// fprintf(f, "lod 3 6 4\n");
	// fprintf(f, "lod 3 3 0\n");
	// PTN_DEBUG_0(1);
	// PTN_DEBUG_0(2);
	// PTN_DEBUG_0(3);

	// For All classes, excluding object
	for(int i=1; i < numClasses; i++){
		// labels
		int moveOnNextClass = labelNum++;
		int classLookup = labelNum++;

		// STEP 8.2: Check if class i is being looked up
		fprintf(f, "lod 1 6 3\n");
		fprintf(f, "mov 2 %d\n", i);
		fprintf(f, "beq 1 2 #%d\n", classLookup);

		// This class is not being looked up so move on to next class
		fprintf(f, "jmp 0 #%d\n", moveOnNextClass);

		// Inspect methods
		fprintf(f, "#%d: mov 0 0\n", classLookup);

		// For All the methods in this class
		for(int j=0; j < classesST[i].numMethods; j++){
			// labels
			int methodLookup = labelNum++;
			int moveOnNextMethod = labelNum++;
			int gotoThisClass = labelNum++;
			int childrenCheck = labelNum++;

			// STEP 8.2 Check if this method is being looked up
			fprintf(f, "lod 1 6 2\n");
			fprintf(f, "mov 2 %d\n", j);
			fprintf(f, "beq 1 2 #%d\n", methodLookup);

			// Not this method, check next method
			fprintf(f, "jmp 0 #%d\n", moveOnNextMethod);

			// STEP 8.3: Check the object type
			fprintf(f, "#%d: lod 1 6 4\n", methodLookup);
			fprintf(f, "lod 1 1 0\n");
			fprintf(f, "mov 2 %d\n", i);
			fprintf(f, "beq 1 2 #%d\n", gotoThisClass);

			// This object type is not the class itself, so check children
			fprintf(f, "jmp 0 #%d\n", childrenCheck);

			// Go to method for this class
			fprintf(f, "#%d: jmp 0 #C%dM%d\n", gotoThisClass, i, j); 

			// STEP 8.4 Begin Children check
			fprintf(f, "#%d: mov 0 0\n", childrenCheck);

			// For Each other class that derives from this class
			for(int k=i + 1; k < numClasses; k++){
				// VTable only consistders object type of children
				if(isChildOfClass(i, k) == 1){
					// STEP 8.5: Check child class against obj type
					int vtableEntry = labelNum++;
					int nextVTEntry = labelNum++;
					fprintf(f, "mov 2 %d\n", k);
					fprintf(f, "beq 1 2 #%d\n", vtableEntry);

					// This is not the right type, check next entry
					fprintf(f, "jmp 0 #%d\n", nextVTEntry);

					// STEP 8.6: Determine if method is overrident in child
					int ovMethod = doesChildOverrideMethod(i, k, j);
					if(ovMethod != -1){
						// This child overrides function j
						fprintf(f, "#%d: jmp 0 #C%dM%d\n", vtableEntry, k, ovMethod);
					}
					else{
						// This child does not override function j
						fprintf(f, "#%d: jmp 0 #C%dM%d\n", vtableEntry, i, j);   
					}

					fprintf(f, "#%d: mov 0 0\n", nextVTEntry);
				}
				// This is not a child of this class
			}  

			// Move on
			fprintf(f, "#%d: mov 0 0\n", moveOnNextMethod);
		}

		// Move on
		fprintf(f, "#%d: mov 0 0\n", moveOnNextClass);
	}

	// STEP 8.7: Sanity Check, this code should never be hit
	fprintf(f, "mov 1 99\n");
	fprintf(f, "hlt 1\n");
	_log("VTABLE - END");
}
void genItableResult(int loc, int res){
	// Return 1 as true
	fprintf(f, "#%d: mov 1 %d\n", loc, res);
	fprintf(f, "lod 2 6 3\n"); // Save ret addr
	fprintf(f, "str 6 3 1\n"); // store res in ret place
	incSP(); // remove class num
	incSP(); // remove object addr
	fprintf(f, "jmp 2 0\n"); // return to instance of
}
void genItable(){
	// variables
	int returnFalse = labelNum++;
	int returnTrue = labelNum++;
	int classCheck = labelNum++;

	/*
		Expects stack to have
		----------------
		A return address 		SP + 3
		Dynamic Object Address 	SP + 2
		Static Class number 	SP + 1
		SP 						SP
	*/

	// STEP 9.1: Add ITABLE label, prepare ret true/false
	_log("ITables - Start");
	genItableResult(returnTrue, 1);
	genItableResult(returnFalse, 0);
	fprintf(f, "#ITABLE: mov 0 0\n");

	// STEP 9.2: Load in the object type from its location
	fprintf(f, "lod 1 6 2\n");
	fprintf(f, "beq 1 0 #%d\n", returnFalse); // dont reference null 
	fprintf(f, "lod 1 1 0\n");

	// STEP 9.3: If Object is class, or obj == class then comparison is true
	fprintf(f, "lod 2 6 1\n");
	fprintf(f, "beq 2 0 #%d\n", returnTrue);
	fprintf(f, "beq 2 1 #%d\n", returnTrue); 
	fprintf(f, "jmp 0 #%d\n", classCheck);


	// For all classes, Object allready checked
	fprintf(f, "#%d: mov 0 0\n", classCheck);
	for(int i=0; i < numClasses; i++){
		// Generate labels
		int moveOnNextClass = labelNum++;
		int inspectClass = labelNum++;

		// STEP 9.4: Check if this class is the type we want to lookup
		fprintf(f, "mov 3 %d\n", i);
		fprintf(f, "beq 3 1 #%d\n", inspectClass); 

		// Move on
		fprintf(f, "jmp 0 #%d\n", moveOnNextClass);

		// STEP 9.5: Compare against parents
		fprintf(f, "#%d: mov 0 0\n", inspectClass);
		int t = classesST[i].superclass;
		while(t != -4){
			fprintf(f, "mov 3 %d\n", t);
			fprintf(f, "beq 3 2 #%d\n", returnTrue); 
			t = classesST[t].superclass;
		}

		// STEP 9.6: This class is not an isntance of
		fprintf(f, "jmp 0 #%d\n", returnFalse);

		// Check next class 
		fprintf(f, "#%d: mov 0 0\n", moveOnNextClass);
	}

	// Sanity Check
	//PTN_DEBUG_0(1);
	fprintf(f, "mov 1 98\n");
	fprintf(f, "hlt 1\n");
	_log("ITables - End");
}

// Main Functions
void codeGenExpr(ASTree *t, int classContainingExpr, int methodContainingExpr){
	if(t == NULL){
		printInternalErrorAndQuit("ASTree is NULL", t->lineNumber);
	}
	//if(DEBUG_COMMENTS){
//		printNodeType(t);
//	}

	// Temporary variables
	ASTList* li;
	VarDecl* varLi;
	char* name;
	int size;
	int branch1;
	int branch2;
	int fieldCount;
	int classType;
	int offset;
	int i;

	int c = classContainingExpr;
	int m = methodContainingExpr;

	switch(t->typ){
		case NAT_LITERAL_EXPR:
			// Add to top of stack the natvalue, move stack down 
			fprintf(f, "mov 1 %d\n", t->natVal);
			fprintf(f, "str 6 0 1\n");
			decSP();
			break;

		case FALSE_LITERAL_EXPR:
		case NULL_EXPR:
			// Add to top of stack 0, move stack down
			fprintf(f, "str 6 0 0\n");
			decSP();
			break;
		
		case PRINT_EXPR:
			_log("Print Started");
			// Generate expression, add result to top
			codeGenExpr(t->children->data,c,m);
			
			// Get the top pf the stack and print it
			// Return of print is value printed
			fprintf(f, "lod 1 6 1\n");
			fprintf(f, "ptn 1\n");
			_log("Print Ended");
			break;

		case PLUS_EXPR:
			// Generate two results add to top of stack
			codeGenExpr(t->children->data,c,m);
			codeGenExpr(t->children->next->data,c,m);

			// Load operands
			fprintf(f, "lod 1 6 2\n");
			fprintf(f, "lod 2 6 1\n");

			// Store operands at stack top
			// which is the place of the first operand Reg 6 + 2
			// Move stack up to discard result of second operand
			fprintf(f, "add 1 1 2\n");
			fprintf(f, "str 6 2 1\n");
			incSP();
			break;

		case MINUS_EXPR:
			// Generate two results add to top of stack
			codeGenExpr(t->children->data,c,m);
			codeGenExpr(t->children->next->data,c,m);
			
			// Place operand in operand 1 position
			fprintf(f, "lod 1 6 2\n");
			fprintf(f, "lod 2 6 1\n");
			fprintf(f, "sub 1 1 2\n");
			fprintf(f, "str 6 2 1\n");
			incSP();
			break;

		case TIMES_EXPR:
			// Generate two results add to top of stack
			codeGenExpr(t->children->data,c,m);
			codeGenExpr(t->children->next->data,c,m);

			// Place operand in operand 1 position
			fprintf(f, "lod 1 6 2\n");
			fprintf(f, "lod 2 6 1\n");
			fprintf(f, "mul 1 1 2\n");
			fprintf(f, "str 6 2 1\n");
			incSP();
			break;

		case EXPR_LIST:
			// For each expression generate code
			li = t->children;
			while(li != NULL){
				codeGenExpr(li->data,c,m);
				li = li->next;

				// Discard results of expression of all but last one
				if(li != NULL){
					incSP();
				}
			}
			break;

		case READ_EXPR:
			// Read a number and add to top of stack
			fprintf(f, "rdn 1\n");
			fprintf(f, "str 6 0 1\n");
			decSP();
			break;

		case TRUE_LITERAL_EXPR:
			// Add 1 into stack
			fprintf(f, "mov 1 1\n");
			fprintf(f, "str 6 0 1\n");
			decSP();
			break;

		case EQUALITY_EXPR:
			// Generate two results
			codeGenExpr(t->children->data,c,m);
			codeGenExpr(t->children->next->data,c,m);

			branch1 = labelNum++;
			branch2 = labelNum++;
			
			// Put ex 1 into 1 and ex2 into 2
			fprintf(f, "lod 1 6 2\n");
			fprintf(f, "lod 2 6 1\n");

			// If ex1 > ex2, goto branch 1
			fprintf(f, "beq 2 1 #%d\n", branch1);

			// Not greater than, return false
			fprintf(f, "str 6 2 0\n");
			fprintf(f, "jmp 0 #%d\n", branch2);

			// Branch 1, return true 
			fprintf(f, "#%d: mov 1 1\n", branch1);
			fprintf(f, "str 6 2 1\n");

			// Move on and remove temp value
			fprintf(f, "#%d: mov 0 0\n", branch2);
			incSP();
			break;

		case GREATER_THAN_EXPR:
			// Generate two results
			codeGenExpr(t->children->data,c,m);
			codeGenExpr(t->children->next->data,c,m);

			branch1 = labelNum++;
			branch2 = labelNum++;
			
			// Put ex 1 into 1 and ex2 into 2
			fprintf(f, "lod 1 6 2\n");
			fprintf(f, "lod 2 6 1\n");

			// If ex1 > ex2, goto branch 1
			fprintf(f, "blt 2 1 #%d\n", branch1);

			// Not greater than, return false
			fprintf(f, "str 6 2 0\n");
			fprintf(f, "jmp 0 #%d\n", branch2);

			// Branch 1, return true 
			fprintf(f, "#%d: mov 1 1\n", branch1);
			fprintf(f, "str 6 2 1\n");

			// Move on and remove temp value
			fprintf(f, "#%d: mov 0 0\n", branch2);
			incSP();
			break;
		
		case NOT_EXPR:
			// Generate 1 result
			codeGenExpr(t->children->data,c,m);
			
			branch1 = labelNum++;
			branch2 = labelNum++;
			
			// Load in op
			fprintf(f, "lod 1 6 1\n");
			//PTN_DEBUG_0(1);

			// Check if result is 0 Then its false
			// So go to branch 1 and return true
			fprintf(f, "beq 1 0 #%d\n", branch1);

			// Else, return false
			fprintf(f, "str 6 1 0\n");
			fprintf(f, "jmp 0 #%d\n", branch2);

			// Branch 1, return true
			fprintf(f, "#%d: mov 1 1\n", branch1);
			//PTN_DEBUG_0(1);
			fprintf(f, "str 6 1 1\n");

			// Move on
			fprintf(f, "#%d: mov 0 0\n", branch2);
			//PTN_DEBUG_N(1);
			break;

		case AND_EXPR:
			branch1 = labelNum++;
			branch2 = labelNum++;
			
			// Generate first results for expression
			codeGenExpr(t->children->data,c,m);
			fprintf(f, "lod 1 6 1\n");
			fprintf(f, "beq 1 0 #%d\n", branch1);

			// Generate second result for expression
			incSP();
			codeGenExpr(t->children->next->data,c,m);
			fprintf(f, "lod 1 6 1\n");
			fprintf(f, "beq 1 0 #%d\n", branch1);
			
			// Neither was equal to zero, returm true
			fprintf(f, "mov 1 1\n");
			fprintf(f, "str 6 1 1\n");
			fprintf(f, "jmp 0 #%d\n", branch2);

			// If ex1 is not equal to ex2
			fprintf(f, "str 6 2 0\n");
			fprintf(f, "jmp 0 #%d\n", branch2);

			// If ex1 && ex2 -> false
			fprintf(f, "#%d: str 6 1 0\n", branch1);

			// Move on and remove expression 2 results
			fprintf(f, "#%d: mov 0 0\n", branch2);
			break;
		
		case IF_THEN_ELSE_EXPR:
			// Generate code for the test expression
			codeGenExpr(t->children->data,c,m);

			branch1 = labelNum++;
			branch2 = labelNum++;

			// Get test result and remove them
			fprintf(f, "lod 1 6 1\n");
			fprintf(f, "beq 1 0 #%d\n", branch1);
			incSP(); 

			// Generate True branch and move on
			codeGenExpr(t->children->next->data,c,m);
			fprintf(f, "jmp 0 #%d\n", branch2);

			// Generate False Branch
			fprintf(f, "#%d: mov 0 0\n", branch1);
			incSP(); 
			codeGenExpr(t->children->next->next->data,c,m);

			// Move on and set branch result as if expr result
			fprintf(f, "#%d: mov 0 0\n", branch2);
			break;

		case FOR_EXPR:
			branch1 = labelNum++;
			branch2 = labelNum++;

			// Generate code for the init expression, remove results
			codeGenExpr(t->children->data,c,m);

			// Generate test expression, and note loop start
			fprintf(f, "#%d: mov 0 0\n", branch2);
			codeGenExpr(t->children->next->data,c,m);

			// load in test results, remove temp results
			fprintf(f, "lod 1 6 1\n");
			//PTN_DEBUG_0(1);
			fprintf(f, "beq 1 0 #%d\n", branch1);
			incSP();

			// Perform loop body and remove results
			codeGenExpr(t->children->next->next->next->data,c,m);
			incSP();

			// Update loop counter and cotinue loop
			codeGenExpr(t->children->next->next->data,c,m);
			incSP();
			fprintf(f, "jmp 0 #%d\n", branch2);

			// Loop test failed, return 0 ontop of test result
			fprintf(f, "#%d: str 6 1 0\n", branch1);
			break;
		
		case NEW_EXPR:
			_log("New Expression Started");
			classType = GetClassType(t->children->data->idVal);
			fieldCount = CountFields(classType);
			branch1 = labelNum++;
			branch2 = labelNum++;

			// Check HP + n + 1 < SP
			fprintf(f, "mov 1 %d\n", (fieldCount + 1));
			fprintf(f, "add 1 1 5\n");
			fprintf(f, "blt 1 6 #%d\n", branch1); 

			// HP + n + 1 is not less than SP
			fprintf(f, "mov 1 66\n"); 
			fprintf(f, "hlt 1\n"); 
			
			// Move on
			fprintf(f, "#%d: mov 1 1\n", branch1);

			// for each field: allocate field, set field as 0, HP++
			for(i=0; i < fieldCount; i++){
				fprintf(f, "str 5 0 0\n"); // Store 0 at HP
				fprintf(f, "add 5 1 5\n"); // Add 1 to HP
			}

			// Store t into HP (reg 5)
			fprintf(f, "mov 1 %d\n", classType);
			fprintf(f, "str 5 0 1\n");

			// Store address of object into stack, SP--, HP++
			fprintf(f, "str 6 0 5\n");
			fprintf(f, "mov 1 1\n");
			fprintf(f, "add 5 1 5\n");
			decSP();
			_log("New Expression Ended");
			break;

		case THIS_EXPR:
			// Can only be used inside a method
			// Refers to object that called this method
			// The Address of the object is at the frame pointer - 1
			fromMethodGetObjAddr();
			//PTN_DEBUG_N(1);
			break;

		case METHOD_CALL_EXPR:
			// Add label we can return to
			branch1 = labelNum++;
			methodFound = 1;
			pushAndDecrement(1, branch1);

			// A un dotted method call can only happen within a function
			// Meaning that the its effectivelly this.method(e)
			fromMethodGetObjAddr();

			// push static class number
			pushAndDecrement(0,  t->staticClassNum);

			// push static method number
			pushAndDecrement(0,  t->staticMemberNum);

			// Get Argument
			codeGenExpr(t->children->next->data,c,m);

			// Jump to the method within this class
			fprintf(f, "jmp 0 #C%dM%d\n", t->staticClassNum, t->staticMemberNum);
			fprintf(f, "#%d: mov 0 0\n", branch1);
			break;

		case DOT_METHOD_CALL_EXPR:
			// Add label we can return to
			branch1 = labelNum++;
			methodFound = 1;
			//printf("Return label is %d\n", branch1);
			pushAndDecrement(1, branch1);

			// Generate code for the object id
			// After the code executes,
			// The objects dynamic address should be atop the stack
			codeGenExpr(t->children->data,c,m);
			checkNullDereference();
			//PTN_DEBUG_0(0);
			//PTN_DEBUG_N(1);

			// push static class number
			pushAndDecrement(0,  t->staticClassNum);

			// push static method number
			pushAndDecrement(0,  t->staticMemberNum);

			// Get Argument
			codeGenExpr(t->childrenTail->data,c,m);

			// Jump to vTable and prepare return
			//PTN_DEBUG_N(4);
			fprintf(f, "jmp 0 #VTABLE\n");
			fprintf(f, "#%d: mov 1 1\n", branch1);
			//PTN_DEBUG_N(1);
			//PTN_DEBUG_0(0);
			break;

		case INSTANCEOF_EXPR:
			/*
				Expects stack to have
				----------------
				A return address 		SP + 3
				Dynamic Object Address 	SP + 2
				Static Class number 	SP + 1
				SP 						SP
			*/
			instanceOfFound = 1;
			branch1 = labelNum++;

			// Put return address on the stack
			pushAndDecrement(1, branch1);

			// code gen left side, leaves obj ontop
			codeGenExpr(t->children->data,c,m);

			// Put in static class number
			name = t->children->next->data->idVal;
			pushAndDecrement(0,  GetClassType(name));
			//PTN_DEBUG_N(1);
			//PTN_DEBUG_N(2);
			//PTN_DEBUG_N(3);

			fprintf(f, "jmp 0 #ITABLE\n");
			fprintf(f, "#%d: mov 0 0\n", branch1);
			//PTN_DEBUG_N(1);
			break;

		case ID_EXPR:
			GetID(1,0,c,m,t);
			break;

		case DOT_ID_EXPR:
			// Evaluate code for E
			// Dont decrement stack, replace e value	
			codeGenExpr(t->children->data, c, m);

			// Object address is atop stack, confirm its not null
			// ! NOTE !
			// Ok, so Good 31 says that null can be used to access
			// statics, that makes no sense, but whatever 
			// I commented it out
			//checkNullDereference();

			GetID(1,1,c,m,t);
			break;

		case ASSIGN_EXPR:

			// Generate code for expression
			_log("Starting Assign Expression");
			GetID(0,0,c,m,t);


			//TEST
			//fprintf(f, "lod 1 6 1\n"); // Get ID lvalue
			//fprintf(f, "lod 1 1 0\n"); // Get ID lvalue
			//fprintf(f, "ptn 1\n"); // Get ID lvalue

			// Get result of expression
			codeGenExpr(t->children->next->data, c,m);
			//PTN_DEBUG_N(1);

			// Store the result at the location
			fprintf(f, "lod 1 6 2\n"); // Get ID lvalue
			fprintf(f, "lod 2 6 1\n"); // Get result value
			fprintf(f, "str 1 0 2\n"); // store result at lvalue
			//fprintf(f, "ptn 2\n"); // store result at lvalue

			// trim stack so that result is ontop
			fprintf(f, "str 6 2 2\n");
			incSP();
			_log("Ending Assign Expression");

			break;
		case DOT_ASSIGN_EXPR:
			// Compute result first
			codeGenExpr(t->childrenTail->data, c,m);
			//PTN_DEBUG();

			// Compute left side object	
			codeGenExpr(t->children->data, c, m);
			//PTN_DEBUG();

			// Object address is atop stack, confirm its not null
			checkNullDereference();

			// Get lvalue
			GetID(0,1,c,m,t);

			// Store the result at the location
			fprintf(f, "lod 1 6 1\n"); // Get ID lvalue
			fprintf(f, "lod 2 6 2\n"); // Get result value
			fprintf(f, "str 1 0 2\n"); // store result at lvalue

			// trim stack so that result is ontop
			fprintf(f, "str 6 1 2\n");
			incSP();

			break;

		case AST_ID:
			printInternalErrorAndQuit("AST_ID is not codegened by itself", t->lineNumber);
		
		default:
			printInternalErrorAndQuit("Unkown expression", t->lineNumber);
	}
}

void generateDISM(FILE *outputFile){
	// STEP 1: Setup variables for generetor
	MethodDecl *li;
	ASTree* t;
	f = outputFile;

	// STEP 2: Generate Main prologue
	genMainPrologue();

	// STEP 3: Generate Main expression block
	t = wholeProgram->children->next->next->data;	
	codeGenExpr(t,-1,-1);

	// STEP 4: Generate Main epilogue
	genMainEpilogue();

	// For Each class, excluding object
	for(int i=1; i < numClasses; i++){
		// For each method
		li = classesST[i].methodList;
		for(int j=0; j < classesST[i].numMethods; j++){
			// STEP 5: Generate prologue
			genMethodPrologue(i, j);

			// STEP 6: Generate exoression block
			codeGenExpr(li[j].bodyExprs, i, j);

			// STEP 7: Generate epilogue
			genMethodEpilogue(i, j);
		}
	}

	// STEP 8: Generate vTable
	if(methodFound == 1){
		// Dont generate if no method is ever called
		_log("Found Function call, generating VTable");
		genVtable();
	}

	// STEP 9: Generate iTable
	if(instanceOfFound == 1){
		// Dont generate if no instance of ever called
		_log("Found InstanceOf, generating ITable");
		genItable();
	}
}