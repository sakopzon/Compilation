/* Reverse polish notation calculator.  */
%{
  #include <stdio.h>
  #include <iostream>
  #include "newParser.hpp"
  
  using namespace std;
  
  int yylex (void);
  void yyerror (char const *);
  void semanticError(string s);
%}

%token NUM 
%token Defstruct Extern Main Var Integer Real If Then While Do Return Write Read Call
%token Else 
%token ID
%token STRING
%token ',' ';' ':'

%right ASSIGN
%left OR_OP
%left AND_OP
%left REL_OP
%left ADD_OP
%left MUL_OP
%right NOT_OP
%left '[' '{' '('
%right ']' '}' ')'
%%

PROGRAM: 
		TDEFS FDEFS MAIN_FUNCTION {
		}
;
TDEFS: 
		TDEFS Defstruct '{' DECLARLIST '}' ID ';' {
		  addStructToSymbolTable($6->tokenValue, $4->declarationList);
		}		
	|
		/*epsilon*/ { cout << "TDEFS to e" << endl; }
;		
FDEFS: 
		FDEFS TYPE ID '(' FUNC_ARGLIST_FULL ')' BLK {
		}
	|
		FDEFS Extern TYPE ID '(' FUNC_ARGLIST_FULL ')' ';' {
		}		
	|
		/*epsilon*/ { cout << "FDEFS to e" << endl;}
;
FUNC_ARGLIST_FULL: 
		FUNC_ARGLIST {
		}
	|
		/*epsilon*/ {}
		/*epsilon*/
;		
FUNC_ARGLIST: 
		FUNC_ARGLIST ',' DCL {
		}
	|
		DCL {
		}
;
MAIN_FUNCTION:
		Main {currFunction = new Function("main()");} BLK {
			cout << "MAIN_FUNCTION" <<endl;
		} 
	|
		/*epsilon*/ {}
;		
BLK: 
		DECLARATIONS '{' LIST '}' {
			cout << "BLK" << endl;
		}
;	
DECLARATIONS:
		Var DECLARLIST {
			cout << "DECLARATIONS" << endl;
		  currFunction->insertSymbolTable($2->declarationList);
		} // the symbol table insertions are made during DECLARLIST parsing
	|
		/*epsilon*/ { cout << "DECLARATIONS to e" << endl; }
;		
DECLARLIST:
		DECLARLIST DCL ';' {
			cout << "DECLARLIST1: " << $2->dcl_type << endl;
		  // $$.declarationList = merge with $1
		  createVariablesFromDCL($2, $1);
		  $$->declarationList = $1->declarationList;
		}
	|
		DCL ';' {
			cout << "DECLARLIST2: " << $1->dcl_type << endl;
		  // $$.declarationList = new
		  createVariablesFromDCL($1, $$);
		}
;		
DCL: 
		ID ':' TYPE {
		  cout << "variable "+ $1->tokenValue +" of type "+ $3->tokenValue +" declared" << endl;
		  $$->dcl_type = $3->tokenValue;
		  ($$->dcl_ids).push_front($1->tokenValue);
		} 
	|
		ID ':' ID {
			validateStructName($3->tokenValue);
		    $$->dcl_type = $3->tokenValue;
		    ($$->dcl_ids).push_front($1->tokenValue);
		  //The 2nd ID might be the name of a custom typedef
		}
	|
		ID ',' DCL {
		  ($3->dcl_ids).push_front($1->tokenValue);
		  ($$->dcl_ids).merge($3->dcl_ids);
		  $$->dcl_type = $3->dcl_type;
		}
;	
TYPE: 
		Integer { 
		} // $$->tokenValue is already assigned in .lex file
	|
		Real {
		} // $$->tokenValue is already assigned in .lex file
;	
LIST: 
		LIST STMT {
		}
	|
		/*epsilon*/ { cout << "LIST to e" << endl; }
;	
STMT: 
		ASSN {
		}
	|
		CNTRL {
		}
	|
		READ {
		}
	|
		WRITE {
		}
	|
		STMT_RETURN {
		}
	|
		BLK {
		}
;	
STMT_RETURN: 
		Return '(' EXP ')' ';' {
		}
;	
WRITE: 
		Write '(' EXP ')' ';' {
		}
	|
		Write '(' STRING ')' ';' {
		}
;	
READ: 
		Read '(' LVAL ')' ';' {
		}
;	
ASSN: 
		LVAL ASSIGN EXP ';' {
		}
;
LVAL: 
		ID {
		}
	|
		STREF {
		}
;
CNTRL: 
		If BEXP Then STMT Else STMT {
		}
	|
		If BEXP Then STMT {
		}
	|
		While BEXP Do STMT {
		}
;	
BEXP: 
		BEXP OR_OP BEXP {
		  // backpatching
		}
	|
		BEXP AND_OP BEXP {
		  // backpatching
		}
	|
		EXP REL_OP EXP {
		  string tempReg = getIntReg();
		  string SLET = "";
		  string SGRT = "";
		  string SEQU = "";
		  string SNEQ = "";
		  string opCommand = "";
		  
		  if(isInt($1->place) && isInt($3->place)) {
		    SLET = "SLETI";
		    SGRT = "SGRTI";
		    SEQU = "SEQUI";
		    SNEQ = "SNEQI";
		  } else if(isReal($1->place) && isReal($3->place)) {
		    SLET = "SLETR";
		    SGRT = "SGRTR";
		    SEQU = "SEQUR";
		    SNEQ = "SNEQR";		    
		  } else {
		    semanticError("type missmatch on relop \'" + $2->tokenValue + "\'");
		  }
		  
		  if($2->tokenValue == "<" || $2->tokenValue == "<=") {
		    opCommand = SLET;
		  } else if($2->tokenValue == ">" || $2->tokenValue == ">=") {
		    opCommand = SGRT;
		  } else if($2->tokenValue == "==") {
		    opCommand = SEQU;
		  } else if($2->tokenValue == "<>") {
		    opCommand = SNEQ;
		  } else {
		    semanticError("can't recognize RELOP " + $2->tokenValue);
		  }
		  
		  bool equalsFlag = false;
		  if($2->tokenValue == "<=" || $2->tokenValue == ">=") {
		    equalsFlag = true;
		  }
		  
		  emit(opCommand + " " + tempReg + " " + $1->place + " " + $2->place);
		  $$->trueList.push_back(nextquad());
		  emit("BNEQZ " + tempReg + " " + "___");
		  if(equalsFlag) {
		    // if tempReg == 0 we should continue down the flow (no jump needed)
		    emit(SEQU + " " + tempReg + " " + $1->place + " " + $2->place);
		    $$->trueList.push_back(nextquad());
		    emit("BNEQZ " + tempReg + " " + "___");
		  }
		  $$->falseList.push_back(nextquad());
		  // TODO: this should be uncomented!!! I don't have any idea why is it giving compilation error, when I replace this with emit("BNEQZ " + tempReg + " " + "___"); it is not complaining - WTF maybe I am tiered.
		  //emit("UJUMP " + "___");
		}

	|
		NOT_OP BEXP {
		  $$->trueList.swap($$->falseList);
		}
	|
		'(' BEXP ')' {
		  $$->trueList = $2->trueList;
		  $$->falseList = $2->falseList;
		}
;	
EXP: 
		EXP ADD_OP EXP {
		  // EXP->place is always in register!
		  if(isUsedIntReg($1->place) && isUsedIntReg($3->place)) {
		    $$->place = getIntReg();
		    emit("ADD2I " + $$->place + " " + $1->place + " " + $2->place);
		  } else if(isUsedRealReg($1->place) && isUsedRealReg($3->place)) {
		    $$->place = getRealReg();
		    emit("ADD2R " + $$->place + " " + $1->place + " " + $2->place);
		  } else {
		    semanticError("type missmatch within add operation");
		  }
		}
	|
		EXP MUL_OP EXP {
		  if(isUsedIntReg($1->place) && isUsedIntReg($3->place)) {
		    $$->place = getIntReg();
		    emit("MULTI " + $$->place + " " + $1->place + " " + $2->place);
		  } else if(isUsedRealReg($1->place) && isUsedRealReg($3->place)) {
		    $$->place = getRealReg();
		    emit("MULTR " + $$->place + " " + $1->place + " " + $2->place);
		  } else {
		    semanticError("type missmatch within mul operation");
		  }
		}
	|
		'(' EXP ')' {
		  $$->place = $2->place;
		}
	|
		'(' TYPE ')' EXP {
		  if($2->tokenValue == "Integer") {
		    if(isUsedIntReg($4->place)) {
		      $$->place = $4->place;
		    } else if(isUsedRealReg($4->place)) {
		      $$->place = getIntReg();
		      emit("CRTOI " + $$->place + " " + $4->place);
		    }
		  } else if($2->tokenValue == "Real") {
		    if(isUsedRealReg($4->place)) {
		      $$->place = $4->place;
		    } else if(isUsedIntReg($4->place)) {
		      $$->place = getRealReg();
		      emit("CITOR " + $$->place + " " + $4->place);
		    }
		  } else {
		    semanticError("undefined explicit cast");
		  }
		}
	|
		ID {
		}
	|
		STREF {
		}
	|
		NUM {
		}
	|
		CALL {
		}
;	
STREF: 
		ID '[' ID ']' {
		}
	|
		STREF '[' ID ']' {
		}
;	
CALL: 
		Call ID '(' CALL_ARGS_FULL ')' {
		}
;	
CALL_ARGS_FULL: 
		CALL_ARGS {
		}
	|
		/*epsilon*/
		/*epsilon*/ {}
;	
CALL_ARGS:
		EXP {
		}
	|
		CALL_ARGS ',' EXP {
		}
;
	
%%

extern char* yytext;
extern int yylineno;

void yyerror (char const *err){
   printf("Syntax error: '%s' in line number %d\n", yytext, yylineno);
   exit(1);
}

void semanticError(string s) {
  printf("Semantic error: '%s' in line number %d\n", s.c_str(), yylineno);
  exit(1);
}
