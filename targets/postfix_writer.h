#ifndef __L22_TARGETS_POSTFIX_WRITER_H__
#define __L22_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"
#include <stack>
#include <set>
#include <sstream>
#include <cdk/emitters/basic_postfix_emitter.h>

namespace l22 {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<l22::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl;
    std::stack<int> _forIni, _forStep, _forEnd; // for break/repeat
    bool _inFunctionBody = false;
    bool _inFunctionArgs = false;
    std::shared_ptr<l22::symbol> _function;
      std::vector<std::string> _next;
      std::vector<std::string> _stop;
    std::set<std::string> _functions_to_declare;
    int _offset = 0;
    bool _errors = false;
      bool _newRetValSeen; // function_name = value; seen in body of function
      bool _specialVarSeen; // function_name seen as var name
    // remember function name for resolving '@'
    std::string _currentFunctionName;
    std::string _currentReturnJumpLabel; // where to jump when on return
    std::stack<l22::symbol> stackNova;



  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<l22::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0) {
    }

  public:
    ~postfix_writer() {
      os().flush();
    }

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // l22

#endif
