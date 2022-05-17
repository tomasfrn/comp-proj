#ifndef __L22_AST_SIZEOF_NODE_H__
#define __L22_AST_SIZEOF_NODE_H__

namespace l22 {


  /**
   * Class for describing dimension of expression.
   */
class sizeof_node: public cdk::expression_node {
    cdk::expression_node *_argument;

  public:
    inline sizeof_node(int lineno, cdk::expression_node *argument) :
        cdk::expression_node(lineno), _argument(argument) {
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
