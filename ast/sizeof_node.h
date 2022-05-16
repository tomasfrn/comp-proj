#ifndef __L22_AST_SIZEOF_NODE_H__
#define __L22_AST_SIZEOF_NODE_H__

#include <cdk/ast/lvalue_node.h>

namespace l22 {

  /**
   * Class for describing evaluation nodes.
   */
class sizeof_node: public cdk::basic_node {
    cdk::expression_node *_argument;

  public:
    inline sizeof_node(int lineno, cdk::expression_node *argument) :
        cdk::basic_node(lineno), _argument(argument) {
    }

  public:
    inline cdk::expression_node *argument() {
      return _argument;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_sizeof_node(this, level);
    }

  };

} // l22

#endif
