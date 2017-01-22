#ifndef COMMON_HPP
#define COMMON_HPP

#include <map>
#include <vector>
#include <string>
#include <set>
#include <cassert>
#include <list>
#include <stack>
#include <array>
#include <iostream>

using namespace std;

#define YYDEBUG 1
extern int yydebug;

void emit(string const& singleInstruction);
// ---------------------------- Classes definitions ------------------------------

enum TypeEnum {INTEGER, REAL, DEFSTRUCT};


// -------- Instances ---------

class Variable {

  string name;
  string type;
  int offset;
  
public:
  Variable(const string name_, string type_, int offset_) : name(name_), type(type_), offset(offset_) {}
  
  Variable(const string name_, TypeEnum t) : name(name_){
    if(t == INTEGER) {
      type = "integer";
    } else if(t == REAL) {
      type = "real";
    } else {
      type = "defstruct";
    }
  }

  Variable(const string name_, string type_) : name(name_), type(type_) {}

  virtual ~Variable() {}

  void setOffset(int offset) { this->offset = offset; }
  int getOffset() { return this->offset; }
  string const& getName() { return name; }
  string const& getType() { return type; }
};


bool isPrimitive(Variable* var);
bool isPrimitive(string type);

class Defstruct : public Variable {

  int sizeInMemory;

public:
  map<string, Variable> fields;
  
  Defstruct(string name_, map<string, Variable> fields_) : Variable(name_, DEFSTRUCT), fields(fields_) {
    // TODO: get the struct type size from the structType global table
    sizeInMemory = 0;
  }

  // gets only valid field names, if not valid field name passed, falls with assertion failure
  Variable* getField(string name) {
    std::map<string, Variable>::iterator i;
    i = fields.find(name);
    if(i != fields.end())
      return &(i->second);
    assert(0);
  }

  // get the inner Variable of the struct by the path to it
  Variable* getStref(list<string> pathToRef) {
    assert(pathToRef.size() > 0);
    Variable* field = getField(pathToRef.front());
    if(pathToRef.size() == 1)
      return field;
    // pathToRef.size() > 1
    if(isPrimitive(field))
      return NULL;
    else { // field is not primitive => it is Defstruct
      Defstruct* dsField = dynamic_cast<Defstruct*>(field);
      pathToRef.pop_front();
      return dsField->getStref(pathToRef); 
    }
  }

  int getSizeInMemory() { return sizeInMemory; }
};

// -------- Types ---------

/* Primitive types are of size 1. The first non-primitive type declared consists of primitives only, so the first StructType will get in the constructor a map with only primitives (all of initialized size 1). Then the next StructType can contain this StructType (which's size is already valid and calculated) and so on.
*/

class Type {

  string typeName;
  int typeSizeInMemory;

protected:

  Type(string typeName_, int initSize) : typeName(typeName_), typeSizeInMemory(initSize) {}

public:

  Type(string typeName_) : typeName(typeName_), typeSizeInMemory(1) {
    if(typeName != "integer" && typeName != "real")
      assert(0);
  }

  virtual ~Type() {}
  
  string getTypeName() { return typeName; }

  int getTypeSizeInMemory() { return typeSizeInMemory; }

  void setTypeSizeInMemory(int size) { typeSizeInMemory = size; }

  void increaseTypeSizeInMemoryBy(int size) { typeSizeInMemory += size; }
};


class StructType : public Type {

public:
  map<string, Type> fieldTypes; // [field name, field type]
  
  StructType(string typeName_, map<string, Type> fieldTypes_) : Type(typeName_, 0), fieldTypes(fieldTypes_) {
    setTypeSizeInMemory(0);
    for(std::map<string, Type>::iterator i = fieldTypes_.begin(); i != fieldTypes_.end(); ++i) {
      increaseTypeSizeInMemoryBy(i->second.getTypeSizeInMemory());
    }
  }
};


// -------- Blocks ---------

class Block {
private:
  Block* parent;

public:
  map<string, Variable> symbolTable;
  
  Block() : parent(NULL) {}

  Block(Block* parent_): parent(parent_) { 
    // std::* containers should be automatically dynamically allocated on decleration
    symbolTable = parent->symbolTable;
  }

  // find variable in block's symbol table
  Variable* getScopeVariable(string name) {
    std::map<string, Variable>::iterator i;
    if((i = symbolTable.find(name)) != symbolTable.end()) {
      return &(i->second);
    }
    return NULL; 
  }

  // find struct stref in block's symbol table
  Variable* getScopeDefstructStref(list<string> path) {
    Variable* temp;
    std::map<string, Variable>::iterator i;
    if((i = symbolTable.find(path.front())) != symbolTable.end()) {
      temp = &(i->second);
    } else assert(0);
    Defstruct* ds = dynamic_cast<Defstruct*>(temp);
    assert(ds);
    // getStref doesn't need the name of the struct
    path.pop_front();
    return ds->getStref(path);
  }
  
  // insert (name,v) to the symbol table
  void addVariable(string const& name, Variable& v) {
    std::map<string, Variable>::iterator i;
    if((i = symbolTable.find(name)) != symbolTable.end())
      symbolTable.erase(i);
    symbolTable.insert(std::pair<string, Variable>(name, v));
    v.setOffset(symbolTable.size());
  }
  
  void insertSymbolTable(map<string, Variable>& vars) {
    for(std::map<string, Variable>::iterator i = vars.begin(); i != vars.end(); ++i) {
      this->addVariable(i->first, i->second);
    }
  }

  void insertSymbolTable(vector<Variable>& vars) {
    for(std::vector<Variable>::iterator i = vars.begin(); i != vars.end(); ++i) {
      addVariable(i->getName(), *i);
    }
  }

};


struct Function : public Block {

  string name;
  string returnType;
  vector<string> arguments;
  bool isImplemented;
  // for unimplemented functions
  vector<int> functionCalls; // an array of line numbers indicating where this function is called
  
  // for implemented functions
  int address;
  
  // the arguments are inserted straight into the symbol table in the constructor.

  // function with arguments
  Function(string name_, string return_type_, bool implemented_ ,vector<Variable> arguments_) :
    name(name_), returnType(return_type_), isImplemented(implemented_) {
    insertSymbolTable(arguments_);
	for(vector<Variable>::iterator i = arguments_.begin() ; i != arguments_.end() ; ++i) {
	  arguments.push_back(i->getType());
	}
  }

  // function without arguments
  Function(string name_) : name(name_) {
	if(name == "main") {
	  isImplemented = true;
	}
    // std::* containers should be automatically dynamically allocated on decleration
  }
  
  string validateCallArguments(vector<string> callArgsList) {
	string err = "";
	int size1 = callArgsList.size();
	int size2 = arguments.size();
	if(size1 != size2) {
	  err = "wrong number of arguments: got " + to_string(size1) + " but requierd " + to_string(size2);
    }
	for(int i = 0 ; i < size1 ; ++i) {
	  if((arguments[i] == "integer" && callArgsList[i].front() == 'R')
          || (arguments[i] == "real" && callArgsList[i].front() == 'I')){
		err = "wrong type of argument at " + to_string(i) + ", should be " + arguments[i]; 
		break;
	  }
	}
	return err;
  }
  
  void putArgumentsOnStack(vector<string> args) {
    for(vector<string>::iterator i = args.begin() ; i != args.end() ; ++i) {
	  string STOR = "";
      if(i->front() == 'I') {
		STOR = "STORI ";
	  } else {
	    STOR = "STORR ";
	  }
	  emit(STOR + *i + " I1 0");
	  emit("ADD2I I1 I1 -1");
    }  
  }
  
};


// ---------------------------- S-type definition --------------------------

struct Stype {
  
  string place;

  // for tokens
  string tokenValue;
  
  // for DECLARLIST and DECLARATIONS - contains the declared Variables 
  map<string, Variable> declarationList;
  map<string, Type> typedefList;
  
  // for DCL - the type for the last arguments
  string dcl_type;
  // for DCL - the variables names () ids of the currently declared type
  list<string> dcl_ids;

  // for BEXP - lists to contain the code lines which will need to be changed on backpatching
  list<int> falseList;
  list<int> trueList;
  
  // for FUNC_ARGLIST
  vector<Variable> argsList;

  // for CALL_ARGS
  vector<string> callArgsList;
  // for M marker
  int quad;

  // for N marker, STMT, LIST, BLK
  list<int> nextList;

  // for LVAL
  string offset;
  string variableName;

  // for EXP and LVAL
  string type;

  // for STREF and LVAL
  list<string> path;
  
  Stype(string v) : tokenValue(v) {}

  Stype() {}
};
#define YYSTYPE Stype


/* ----------------- Compile Time Data Structures: -------------------


               funcSymbols                               funcStack
 _______________________________________               _____________   
\   functionName    \   symbolTable     \             \   funcName  \
\___________________\___________________\             \_____________\      
\                   \                   \             \             \
\___________________\___________________\             \_____________\
\                   \                   \             \      .      \
\___________________\___________________\             \      .      \
\     .      .      \      .      .     \             \      .      \
\     .      .      \      .      .     \             \             \
\     .      .      \      .      .     \             \_____________\
\                   \                   \
\___________________\___________________\


             structTypeTable
 _______________________________________
\   defstruct name   \       Type       \
\____________________\__________________\
\         .          \         .        \
\         .          \         .        \
\         .          \         .        \
\                    \                  \  
\____________________\__________________\

'funcSymbols'       --- Map from function name to it's symbol table and arguments. 

'funcStack'         --- Stack which holds on it's top the currently running function (by currently we mean the function which's call was the last in the parsed code). This stack contains only strings! by this string we can find the relevant symbol table with the environment variables in the 'funcSymbols' map.

'structTypeTable' --- Map from defstruct type to the Type representing it's information. Each 'Type' (second value) is a class which contains a Map from field name to field type (which can also be a defstruct and so on).

*** each symbol table is a Map of [var name (string), var info class(Variable)]***



             codeBuffer                                      memMap
 __________________________________                  _____________________
\         ASM instruction         \ 0               \                    \ 0
\_________________________________\                 \____________________\
\                                 \ 1               \                    \ 1
\_________________________________\                 \____________________\
\                                 \ 2               \                    \ 2
\_________________________________\                 \____________________\
\                .                \ .               \          .         \ .
\                .                \ .               \          .         \ .
\                .                \ .               \          .         \ .
\                                 \                 \                    \
\_________________________________\                 \____________________\ 999

'codeBuffer'        --- Vector (we want the back() function which is not included in std::array) which holds the RISKI instructions. This buffer will be the target of the emit() function and will be modified on backpatching. This buffer should not be larger then 1000 (memory size).

'memMap'            --- Array which represents the run time memory layout, for each address (0-999) which type is the data there.



*/

extern map<string, Function> funcSymbols;
extern stack<string> funcStack;
extern Function* currFunction;
extern Block* currBlock;
extern map<string, StructType> structTypeTable;
extern vector<string> codeBuffer;
extern array<TypeEnum, 1000> memMap;
extern map<string, Defstruct> typdefsTable;

// globals for the linker header
extern string unimplemented;
extern string implemented;


// ----------------- Registers: ------------------
extern set<string> usedIntRegs;
extern set<string> usedRealRegs;

void regSetsInit();
/*
I0 - return address
I1 - stack pointer
I2 - frame pointer
*/

// ----------------- Helper functions: ------------------
//void emit(string const& singleInstruction);
string getIntReg();
string getRealReg();
bool isUsedIntReg(string& in);
bool isUsedRealReg(string& in);
void createVariablesFromDCL(Stype* DCL, Stype* DECLARLIST);
int createTypeFromDCL(Stype* DCL, Stype* DECLARLIST);
void createArgumentsFromDCL(Stype* DCL, Stype* FUNC_ARGLIST);
void addStructToSymbolTable(string name, map<string, Variable> fields);
bool validateStructName(string name);
int nextquad();
bool isInteger(string& in);
bool isReal(string& in);
bool isIntegerVariable(string& in);
bool isRealVariable(string& in);
void Error(string& s);
void printState();
void backpatch(list<int> toFill, int address);
//bool isPrimitive(Variable* var);
//bool isPrimitive(string type);
void copyStruct(Defstruct* lvalVar, string reg);
void addToStructTypeTable(string structName, map<string, Type>typeFields);

Function* getFunction(string name);
void saveUsedRegisters();
void restoreUsedRegisters();

void buildLinkerHeader();

/* ----------------- Run Time Memory layout: -------------------

 ______________________________________
\                                      \  0
\                                      \  1
\             code segment             \  2
\                                      \  .
\______________________________________\  .
\     .      .      .      .      .    \  .
\     .      .      .      .      .    \
\     .      .      .      .      .    \
\                                      \
\                                      \
\                                      \
\                                      \
\     .      .      .      .      .    \
\     .      .      .      .      .    \
\     .      .      .      .      .    \
\______________________________________\
\                                      \
\                                      \
\                                      \  .
\                stack                 \  .
\                                      \  .
\                                      \  997
\                                      \  998
\______________________________________\  999

*/

/* ---------------- Operation Record (for functions) -------------

stack grows downwards, meaning the first operation record starts at the highest address- 999

The address of a local variable (including arguments) is calculated with offset from FP (address = FP - offset)
 ____________________________
\
\      Local Variables
\            :			<-SP
\            :
\____________________________
\
\      calee arguments
\            :
\            :
\____________________________   <-FP
\     calee return value
\____________________________
\           I2
\____________________________
\           I0
\____________________________
\
\     Caller saved register
\            :
\            :
\____________________________
\
\      Local Variables
\            :
\            :
\____________________________


*/

#endif //COMMON_HPP
