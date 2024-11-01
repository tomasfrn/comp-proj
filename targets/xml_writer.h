#ifndef __L22_TARGETS_XML_WRITER_H__
#define __L22_TARGETS_XML_WRITER_H__

#include "targets/basic_ast_visitor.h"
#include <cdk/ast/basic_node.h>
#include <sstream>
#include <stack>

namespace l22 {

  /**
   * Print nodes as XML elements to the output stream.
   */
  class xml_writer: public basic_ast_visitor {
    cdk::symbol_table<l22::symbol> &_symtab;

    std::ostringstream  _namestream;
        // TODO TOMAS
      // semantic analysis
      bool _errors, _inFunctionArgs, _inFunctionBody;
      bool _inForInit;
      bool _returnSeen; // when building a function
      std::stack<int> _forIni, _forStep, _forEnd; // for break/repeat
      std::stack<bool> _globals; // for deciding whether a variable is global or not
      std::shared_ptr<l22::symbol> _function; // for keeping track of the current function and its arguments
      int _offset; // current framepointer offset (0 means no vars defined)
      cdk::typename_type _lvalueType;

  public:
    xml_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<l22::symbol> &symtab) :
        basic_ast_visitor(compiler), _symtab(symtab) {
    }

  public:
    ~xml_writer() {
      os().flush();
    }
  private:
      void error(int lineno, std::string s) {
          std::cerr << "error: " << lineno << ": " << s << std::endl;
      }

  private:
    void openTag(const std::string &tag, int lvl) {
      os() << std::string(lvl, ' ') + "<" + tag + ">" << std::endl;
    }
    void openTag(const cdk::basic_node *node, int lvl) {
      openTag(node->label(), lvl);
    }
    void closeTag(const std::string &tag, int lvl) {
      os() << std::string(lvl, ' ') + "</" + tag + ">" << std::endl;
    }
    void closeTag(const cdk::basic_node *node, int lvl) {
      closeTag(node->label(), lvl);
    }

  protected:
    void do_binary_operation(cdk::binary_operation_node *const node, int lvl);
    void do_unary_operation(cdk::unary_operation_node *const node, int lvl);
    template<typename T>
    void process_literal(cdk::literal_node<T> *const node, int lvl) {
      os() << std::string(lvl, ' ') << "<" << node->label() << ">" << node->value() << "</" << node->label() << ">" << std::endl;
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
