/* DJ AST Implementation */
/* I pledge my Honor that I have not cheated, and will not cheat, on this assignment */
/* Carlos Leon */

// Helper Functions
#include "ast.h"
#include <stdio.h>
#include <string.h>

#define show(s) printf(#s); break 

void printErrAndQuit(char* err){
	printf("AST Error: %s\n", err);
	exit(-1);
}

void printNodeType(ASTree* thisTree){
	if(thisTree != NULL){
		switch(thisTree->typ){
			case PROGRAM: 				show(PROGRAM);
			case CLASS_DECL_LIST: 		show(CLASS_DECL_LIST);
			case CLASS_DECL: 			show(CLASS_DECL);
			case STATIC_VAR_DECL_LIST: 	show(STATIC_VAR_DECL_LIST);
			case STATIC_VAR_DECL: 		show(STATIC_VAR_DECL);
			case VAR_DECL_LIST: 		show(VAR_DECL_LIST);
			case VAR_DECL: 				show(VAR_DECL);
			case METHOD_DECL_LIST: 		show(METHOD_DECL_LIST);
			case METHOD_DECL: 			show(METHOD_DECL);
			case NAT_TYPE: 				show(NAT_TYPE);
			case BOOL_TYPE: 			show(BOOL_TYPE);
			case EXPR_LIST: 			show(EXPR_LIST);
			case DOT_METHOD_CALL_EXPR: 	show(DOT_METHOD_CALL_EXPR);
			case METHOD_CALL_EXPR:		show(METHOD_CALL_EXPR);
			case DOT_ID_EXPR: 			show(DOT_ID_EXPR); 
			case ID_EXPR: 				show(ID_EXPR);
			case DOT_ASSIGN_EXPR: 		show(DOT_ASSIGN_EXPR);
			case ASSIGN_EXPR: 			show(ASSIGN_EXPR);
			case PLUS_EXPR: 			show(PLUS_EXPR);
			case MINUS_EXPR: 			show(MINUS_EXPR);
			case TIMES_EXPR: 			show(TIMES_EXPR);
			case EQUALITY_EXPR: 		show(EQUALITY_EXPR);
			case GREATER_THAN_EXPR: 	show(GREATER_THAN_EXPR);
			case NOT_EXPR: 				show(NOT_EXPR);
			case AND_EXPR: 				show(AND_EXPR);
			case INSTANCEOF_EXPR: 		show(INSTANCEOF_EXPR);
			case IF_THEN_ELSE_EXPR: 	show(IF_THEN_ELSE_EXPR);
			case FOR_EXPR: 				show(FOR_EXPR);
			case PRINT_EXPR: 			show(PRINT_EXPR);
			case READ_EXPR: 			show(READ_EXPR);
			case THIS_EXPR: 			show(THIS_EXPR);
			case NEW_EXPR: 				show(NEW_EXPR);
			case NULL_EXPR: 			show(NULL_EXPR);
			case TRUE_LITERAL_EXPR: 	show(TRUE_LITERAL_EXPR);
			case FALSE_LITERAL_EXPR: 	show(FALSE_LITERAL_EXPR);
			case NAT_LITERAL_EXPR: 
			printf("NAT_LITERAL_EXPR(%d)", thisTree->natVal);
			break;
			case AST_ID: 
			printf("AST_ID(%s)", thisTree->idVal);
			break;
			default:
			printErrAndQuit("Could not printout Node Type");
		}
		printf("  ");
	}
}

void recursiveNodePrint(ASTree* curr, unsigned int depth){
	if(curr != NULL){
		printf("%d:", depth);

		for(int i=0; i < depth; i++){
			printf("  ");
		}

		printNodeType(curr);
		printf("(ends on line %d)\n", curr->lineNumber);
		ASTList* iter = curr->children;
		while(iter != NULL){
			recursiveNodePrint(iter->data, depth+1);
			iter = iter->next;
		}
	}
}

// ast.h functions
ASTree *newAST(ASTNodeType t, ASTree *child, unsigned int natAttribute, 
	char *idAttribute, unsigned int lineNum){

	ASTree* newtree = (ASTree*) malloc(sizeof(ASTree));
	if(newtree == NULL){
		printErrAndQuit("Could not allocate memory for new tree");
	}

	newtree->typ = t;
	newtree->lineNumber = lineNum;

	ASTList* newListChildren = (ASTList*) malloc(sizeof(ASTList));
	if(newListChildren == NULL){
		printErrAndQuit("Could not allocate memory for new tree children");
	}
	newListChildren->next = NULL;
	newListChildren->data = child;
	
	newtree->children = newListChildren;
	newtree->childrenTail = newListChildren;

	if(idAttribute != NULL){
		char* idString = (char*) malloc(strlen(idAttribute) + 1);
		strcpy(idString, idAttribute);
		newtree->idVal = idString;
	}
	else{
		newtree->idVal = NULL;
	}

	if(t == NAT_LITERAL_EXPR){
		newtree->natVal = natAttribute;
	}
	
	return newtree;
}


ASTree *appendToChildrenList(ASTree *parent, ASTree *newChild){
	if(parent == NULL){
		printErrAndQuit("Cannot append to NULL parent");
	}
	else if(parent->children == NULL || parent->childrenTail == NULL){
		printErrAndQuit("Cannot append to NULL children list");
	}
	else if(newChild == NULL){
		// This is on purpose, nothing saying i cant do this
		return parent;
	}

	if(parent->childrenTail->data == NULL){
		parent->childrenTail->data = newChild;
	}
	else{
		ASTList* nextChild = (ASTList*) malloc(sizeof(ASTList));
		if(nextChild == NULL){
			printErrAndQuit("Could not allocate memory for next child");
		}

		nextChild->data = newChild;
		nextChild->next = NULL;

		parent->childrenTail->next = nextChild;
		parent->childrenTail = nextChild;
	}

	return parent;
}

void printAST(ASTree *t){
	recursiveNodePrint(t, 0);
}