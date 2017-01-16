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

using namespace std;


enum Type {INTEGER, REAL, DEFSTRUCT};


class Variable {

  string name;
  string type;
  
public:
  Variable(const string name_, Type t) : name(name_) {
    if(t == INTEGER) {
      type = "integer";
    } else if(t == REAL) {
      type = "real";
    } else {
      type = "defstruct";
    }
  }

  Variable(const string name_, string type_) : name(name_), type(type_) {}
  
  string const& getName() { return name; }
  string const& getType() { return type; }
};


class Defstruct : Variable {

  map<string, Variable> fields;

public:
  Defstruct(string name_, map<string, Variable> fields_) : Variable(name_, DEFSTRUCT), fields(fields_) {}

  Variable getField(string name) {
    std::map<string, Variable>::iterator i;
    i = fields.find(name);
    if(i != fields.end())
      return i->second;
    assert(0);
  }
};


struct Function {

  map<string, Variable> symbolTable;
  vector<Variable> arguments;
  
  Function(vector<Variable> arguments_) : arguments(arguments_) {
    for(std::vector<Variable>::iterator i = arguments_.begin(); i != arguments_.end(); ++i) {
      insertSymbolTable(i->getName(), *i);
    }
  }

  // init with empty vector and empty map (std::* data structures should be automatically dynamically allocated)
  Function() {}

  // insert (name,v) to the symbol table
  void insertSymbolTable(string const& name, Variable& v) {
    std::map<string, Variable>::iterator i;
    if((i = symbolTable.find(name)) != symbolTable.end())
      symbolTable.erase(i);
    symbolTable.insert(std::pair<string, Variable>(name, v));
  }
};

// ---------------------------- S-type definition --------------------------

struct Stype {
  
  string place;

  // for tokens
  string tokenValue;
  
  // for DECLARLIST - 
  map<string, Variable> declarationList;

  // for DCL - the type for the last arguments
  string dcl_type;
  // for DCL - the variables names () ids of the currently declared type
  list<string> dcl_ids;
  
  Stype(string v) : tokenValue(v) {}
};


#define YYSTYPE Stype*


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


                       globalSymbolTable
                     _____________________
                    \       varName      \
                    \____________________\
                    \         .          \
                    \         .          \
                    \         .          \
                    \                    \
                    \____________________\

'funcSymbols'       --- Map from function name to it's symbol table. 

'funcStack'         --- Stack which holds on it's top the currently running function (by currently we mean the function which's call was the last in the parsed code). This stack contains only strings! by this string we can find the relevant symbol table with the environment variables in the 'funcSymbols' map.

'globalSymbolTable' --- Symbol table for the global variables which are included in each function's environment.

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

'memMap'            --- Array which represents the run time memory layout, for each address (0-999) which type is thedata there.



*/

extern map<string, Function> funcSymbols;
extern stack<string> funcStack;
extern Function* currFunction;
extern map<string, Variable> globalSymbolTable;
extern vector<string> codeBuffer;
extern array<Type, 1000> memMap;


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
void emit(string const& singleInstruction);
string getIntReg();
string getRealReg();
bool isUsedIntReg(string& in);
bool isUsedRealReg(string& in);
void createVariablesFromDCL(Stype* DCL);
void Error(string& s);


/* ----------------- Run Time Memory layout: -------------------

 ______________________________________
\                                      \
\                                      \
\          stack(grows down)           \
\                                      \
\______________________________________\
\     .      .      .      .      .    \
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
\         heap (allocated mem)         \
\              (grows up)              \
\______________________________________\
\                                      \
\                                      \
\                                      \
\           code segment               \
\                                      \
\                                      \
\                                      \
\______________________________________\

*/

#endif //COMMON_HPP
