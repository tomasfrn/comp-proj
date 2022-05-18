#ifndef __L22_AST_WRITE_NODE_H__
#define __L22_AST_WRITE_NODE_H__

#include <cdk/ast/expression_node.h>

namespace l22 {
//TODO ver writeln

  /**
   * Class for describing write nodes.
   */
  class write_node: public cdk::basic_node {
    cdk::expression_node *_argument;

  public:
    inline write_node(int lineno, cdk::expression_node *argument) :
        cdk::basic_node(lineno), _argument(argument) {
    }

  public:
    inline cdk::expression_node *argument() {
      return _argument;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_write_node(this, level);
    }

  };

} // l22

#endif
