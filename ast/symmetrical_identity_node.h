#ifndef __L22_AST_SYMMETRICAL_IDENTITY_NODE_H__
#define __L22_AST_SYMMETRICAL_IDENTITY_NODE_H__

namespace l22 {

  /**
   * Class for describing symmetrical or identity node.
   */
class symmetrical_identity_node: public cdk::basic_node {
    cdk::expression_node *_argument;

  public:
    inline symmetrical_identity_node(int lineno, cdk::literal_node<int> *argument) :
            cdk::basic_node(lineno), _argument(argument) {
    }
    inline symmetrical_identity_node(int lineno, cdk::literal_node<double> *argument) :
            cdk::basic_node(lineno), _argument(argument) {
    }

  public:
    inline cdk::expression_node *argument() {
      return _argument;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_symmetrical_identity_node(this, level);
    }

  };

} // l22

#endif
