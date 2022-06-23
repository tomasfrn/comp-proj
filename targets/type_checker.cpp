#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>
#include <stack>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void l22::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
//    for(size_t i = 0; i < node->size(); i++) {
//        node->node(i)->accept(this, lvl + 2);
//        cdk::expression_node *expression = dynamic_cast<cdk::expression_node *>(node->node(i));
//        if(expression != nullptr && expression->is_typed(cdk::TYPE_UNSPEC)) {
//            // TODO ver condicao do OG
//            l22::input_node *input = dynamic_cast<l22::input_node*>(expression);
//            input->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
//        }
//    }
    auto symbol = new_symbol();
    for (size_t i = 0; i < node->size(); i++)
        node->node(i)->accept(this, lvl);
}

//---------------------------------------------------------------------------

void l22::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}

void l22::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

void l22::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

// TODO leicmyballs
void l22::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->argument()->accept(this, lvl + 2);
    if (node->argument()->is_typed(cdk::TYPE_INT))
        node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *input = dynamic_cast<l22::input_node*>(node->argument());

        if(input != nullptr) {
            node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
            node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        }
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else
        throw std::string("Wrong type in argument of unary expression (Integer expected).");

}

void l22::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
    do_BooleanLogicalExpression(node, lvl);
}

void l22::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
    do_BooleanLogicalExpression(node, lvl);
}

void l22::type_checker::do_address_of_node(l22::address_of_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->argument()->accept(this, lvl + 2);
    if (!node->argument()->is_typed(cdk::TYPE_UNSPEC) &&
        !node->argument()->is_typed(cdk::TYPE_VOID))
        node->type(cdk::reference_type::create(4, node->argument()->type()));
    else
        throw std::string("Wrong type in unary logical expression.");
}

void l22::type_checker::do_sizeof_node(l22::sizeof_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->argument()->accept(this, lvl + 2);
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_again_node(l22::again_node *const node, int lvl) {
    // EMPTY TODO
}

void l22::type_checker::do_return_node(l22::return_node *const node, int lvl) {
//    TODO checkar se isto esta bem e faz sentido
    if (node->retval()) {
        if (_function->type() != nullptr && _function->is_typed(cdk::TYPE_VOID)) throw std::string(
                    "initializer specified for void function.");

        node->retval()->accept(this, lvl + 2);

        // function is auto: copy type of first return expression
        if (_function->type() == nullptr) {
            _function->set_type(node->retval()->type());
            return; // simply set the type
        }

        if (_inBlockReturnType == nullptr) {
            _inBlockReturnType = node->retval()->type();
        } else {
            if (_inBlockReturnType != node->retval()->type()) {
                _function->set_type(cdk::primitive_type::create(0, cdk::TYPE_ERROR));  // probably irrelevant
                throw std::string("all return statements in a function must return the same type.");
            }
        }

        std::cout << "FUNCT TYPE " << (_function->type() == nullptr ? "auto" : cdk::to_string(_function->type())) << std::endl;
        std::cout << "RETVAL TYPE " << cdk::to_string(node->retval()->type()) << std::endl;

        if (_function->is_typed(cdk::TYPE_INT)) {
            if (!node->retval()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type for initializer (integer expected).");
        } else if (_function->is_typed(cdk::TYPE_DOUBLE)) {
            if (!node->retval()->is_typed(cdk::TYPE_INT) && !node->retval()->is_typed(cdk::TYPE_DOUBLE)) {
                throw std::string("wrong type for initializer (integer or double expected).");
            }
        } else if (_function->is_typed(cdk::TYPE_STRING)) {
            if (!node->retval()->is_typed(cdk::TYPE_STRING)) {
                throw std::string("wrong type for initializer (string expected).");
            }
        } else if (_function->is_typed(cdk::TYPE_POINTER)) {
            //DAVID: FIXME: trouble!!!
            int ft = 0, rt = 0;
            auto ftype = _function->type();
            while (ftype->name() == cdk::TYPE_POINTER) {
                ft++;
                ftype = cdk::reference_type::cast(ftype)->referenced();
            }
            auto rtype = node->retval()->type();
            while (rtype != nullptr && rtype->name() == cdk::TYPE_POINTER) {
                rt++;
                rtype = cdk::reference_type::cast(rtype)->referenced();
            }

            std::cout << "FUNCT TYPE " << cdk::to_string(_function->type()) << " --- " << ft << " -- " << ftype->name() << std::endl;
            std::cout << "RETVAL TYPE " << cdk::to_string(node->retval()->type()) << " --- " << rt << " -- " << cdk::to_string(rtype)
                      << std::endl;

            bool compatible = (ft == rt) && (rtype == nullptr || (rtype != nullptr && ftype->name() == rtype->name()));
            if (!compatible) throw std::string("wrong type for return expression (pointer expected).");

        } else {
            throw std::string("unknown type for initializer.");
        }
    }
}

void l22::type_checker::do_stop_node(l22::stop_node *const node, int lvl) {
    // EMPTY TODO
}

void l22::type_checker::do_block_node(l22::block_node * const node, int lvl) {
    if (node->declarations())
        node->declarations()->accept(this, lvl + 2);

    if (node->instructions())
        node->instructions()->accept(this, lvl + 2);
}

void l22::type_checker::do_function_call_node(l22::function_call_node * const node, int lvl) {
// f(args)
// @(args)
// (func_def)(args)
    // node type
    std::shared_ptr<cdk::functional_type> node_type = cdk::functional_type::cast(node->type());
    // ponteiro para o node type
    std::shared_ptr<cdk::functional_type> init_type = cdk::functional_type::cast(node->expr()->type());

    if (node_type->output(0) != init_type->output(0)) {
        throw std::string("Outputs are of different types");
    }

    if (node_type->input_length() != init_type->input_length()) {
        throw std::string("Error: different number of arguments");
    }

    for (size_t ax = 0; ax < node_type->input_length(); ax++) {
        if(node_type->input(ax)->name() == init_type->input(ax)->name()) continue;
        if(node_type->input(ax)->name() == cdk::TYPE_INT && init_type->input(ax)->name() == cdk::TYPE_DOUBLE) continue;

        throw std::string("Error: mismatched arguments");
    }
}

//        function.push(l22::symbol(0, node->type(), "begin", true, 0, false));
void l22::type_checker::do_function_definition_node(l22::function_definition_node * const node, int lvl) {
    // Empty
}
// TODO ver isto
void l22::type_checker::do_index_node(l22::index_node * const node, int lvl) {
    ASSERT_UNSPEC;
    std::shared_ptr < cdk::reference_type > btype;
    // TODO não e possivel indexar ponteiros que designem funcoes
    if (node->base()) {
        node->base()->accept(this, lvl + 2);
        btype = cdk::reference_type::cast(node->base()->type());
        if (!node->base()->is_typed(cdk::TYPE_POINTER)) throw std::string("pointer expression expected in index left-value");
    } else {
        btype = cdk::reference_type::cast(_function->type());
        if (!_function->is_typed(cdk::TYPE_POINTER)) throw std::string("return pointer expression expected in index left-value");
    }

    node->index()->accept(this, lvl + 2);
    if (!node->index()->is_typed(cdk::TYPE_INT)) throw std::string("integer expression expected in left-value index");

    node->type(btype->referenced());
}

void l22::type_checker::do_null_ptr_node(l22::null_ptr_node * const node, int lvl) {
    ASSERT_UNSPEC;
    node->type(cdk::reference_type::create(4, nullptr));
}
// TODO leicmyballs
void l22::type_checker::do_stack_alloc_node(l22::stack_alloc_node * const node, int lvl) {
    ASSERT_UNSPEC;
    node->argument()->accept(this, lvl + 2);
    if(node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *input = dynamic_cast<l22::input_node *>(node->argument());

        if(input != nullptr)
            node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else if (!node->argument()->is_typed(cdk::TYPE_INT))
        throw std::string("Integer expression expected in allocation expression.");

    node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

// certo!
void l22::type_checker::do_variable_declaration_node(l22::variable_declaration_node * const node, int lvl) {
    // TODO use
    // var

    if(node->type() == nullptr) {
        if (node->initializer() != nullptr)
            node->type(node->initializer()->type());
        else
            throw std::string("Type not defined");
    }
    // quando o initializer existe
    else if (node->initializer() != nullptr) {
        node->initializer()->accept(this, lvl + 2);

        // se for do tipo unspec
        if (node->initializer()->is_typed(cdk::TYPE_UNSPEC)){

            l22::input_node *input = dynamic_cast<l22::input_node*>(node->initializer());
            l22::stack_alloc_node *stack = dynamic_cast<l22::stack_alloc_node*>(node->initializer());

            if(input != nullptr) {
                if(node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE))
                    node->initializer()->type(node->type());
                else
                    throw std::string("Unable to read input.");
            }

            else if (stack != nullptr) {
                if (node->is_typed(cdk::TYPE_POINTER))
                    node->initializer()->type(node->type());
            }
            else
                throw std::string("Unknown node with unspecified type.");
        }

        // é do tipo int
        else if (node->is_typed(cdk::TYPE_INT)) {
            if (!node->initializer()->is_typed(cdk::TYPE_INT))
                throw std::string("Wrong type for initializer (integer expected).");
        }
        // é do tipo double
        else if (node->is_typed(cdk::TYPE_DOUBLE)) {
            if (!node->initializer()->is_typed(cdk::TYPE_DOUBLE) && !node->initializer()->is_typed(cdk::TYPE_INT))
                throw std::string("Wrong type for initializer (integer or double expected).");
        }
        // é do tipo string
        else if (node->is_typed(cdk::TYPE_STRING)) {
            if (!node->initializer()->is_typed(cdk::TYPE_STRING))
                throw std::string("Wrong type for initializer (string expected).");
        }
        // é do tipo pointer
        else if (node->is_typed(cdk::TYPE_POINTER)) {
            if (!node->initializer()->is_typed(cdk::TYPE_POINTER))
                throw std::string("Wrong type for initializer (pointer expected).");
        }
        else if (node->is_typed(cdk::TYPE_FUNCTIONAL)) {

            if (!node->initializer()->is_typed(cdk::TYPE_FUNCTIONAL))
                throw std::string("Wrong type for initializer (functional expected).");

            //int<int> x = (text) -> int :
             //               hil

            std::shared_ptr<cdk::functional_type> node_type = cdk::functional_type::cast(node->type());
            std::shared_ptr<cdk::functional_type> init_type = cdk::functional_type::cast(node->initializer()->type());

            if (node_type->output(0) != init_type->output(0)) {
                throw std::string("Outputs are of different types");
            }

            if (node_type->input_length() != init_type->input_length()) {
                throw std::string("Error: different number of arguments");
            }

            for (size_t ax = 0; ax < node_type->input_length(); ax++) {
                if(node_type->input(ax)->name() == init_type->input(ax)->name()) continue;
                if(node_type->input(ax)->name() == cdk::TYPE_INT && init_type->input(ax)->name() == cdk::TYPE_DOUBLE) continue;

                throw std::string("Error: mismatched arguments");
            }
            const std::string &id = node->identifier();
            std::shared_ptr<l22::symbol> symbol = std::make_shared<l22::symbol>(node->qualifier(),
                                                                                node->type(),
                                                                                id,
                                                                                true,
                                                                                0,
                                                                                false);
            if (_symtab.insert(id, symbol))
                _parent->set_new_symbol(symbol);
            else
                throw std::string("Variable '" + id + "' has been redeclared.");
        }
        if (!node->is_typed(cdk::TYPE_FUNCTIONAL)) {
            const std::string &id = node->identifier();
            std::shared_ptr<l22::symbol> symbol = std::make_shared<l22::symbol>(node->qualifier(),
                                                                              node->type(),
                                                                              id,
                                                                              false,
                                                                              0,
                                                                              false);

            if (_symtab.insert(id, symbol))
                _parent->set_new_symbol(symbol);
            else
                throw std::string("Variable '" + id + "' has been redeclared.");
        }
        else
            throw std::string("Unknown type for variable initializer.");
    }
    std::cout << "            ENTROU222           " << std::endl;

}


void l22::type_checker::do_identity_node(l22::identity_node * const node, int lvl) {
    processUnaryExpression(node, lvl);
}
//---------------------------------------------------------------------------

void l22::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void l22::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

//---------------------------------------------------------------------------

void l22::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->argument()->accept(this, lvl + 2);
    if (node->argument()->is_typed(cdk::TYPE_INT))
        node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else if (node->argument()->is_typed(cdk::TYPE_DOUBLE))
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *input = dynamic_cast<l22::input_node*>(node->argument());

        if(input != nullptr) {
            node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
            node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        }
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else
        throw std::string("Wrong type in argument of unary expression (Integer or double expected).");
}

void l22::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

// TODO checkar para ver se isto se aplica
//void l22::type_checker::processBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
//  ASSERT_UNSPEC;
//  node->left()->accept(this, lvl + 2);
//  if (!node->left()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of binary expression");
//
//  node->right()->accept(this, lvl + 2);
//  if (!node->right()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of binary expression");
//
//   in Simple, expressions are always int
//  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
//}

void l22::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->left()->accept(this, lvl + 2);
    node->right()->accept(this, lvl + 2);

    if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
        node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT))
        node->type(node->left()->type());
    else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER))
        node->type(node->right()->type());
    else if(node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node*>(node->left());
        l22::input_node *inputr = dynamic_cast<l22::input_node*>(node->right());

        if(inputl != nullptr)
            node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        else
            throw std::string("Unknown node with unspecified type.");

        if(inputr != nullptr)
            node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        else
            throw std::string("Unknown node with unspecified type.");
        node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node*>(node->left());

        if(inputl != nullptr) {
            if(node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT))
                node->left()->type(node->right()->type());
            else
                throw std::string("Invalid expression in right argument of binary expression.");
        }
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputr = dynamic_cast<l22::input_node*>(node->right());

        if(inputr != nullptr) {
            if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->left()->is_typed(cdk::TYPE_INT))
                node->right()->type(node->left()->type());
            else
                throw std::string("Invalid expression in left argument of binary expression.");
        }
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else
        throw std::string("Wrong types in binary expression.");
}
void l22::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->left()->accept(this, lvl + 2);
    node->right()->accept(this, lvl + 2);

    if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
        node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT))
        node->type(node->left()->type());
    else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER))
        node->type(node->right()->type());
    else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER)) {
        typeOfPointer(cdk::reference_type::cast(node->left()->type()),cdk::reference_type::cast(node->right()->type()));
        node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else if(node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node*>(node->left());
        l22::input_node *inputr = dynamic_cast<l22::input_node*>(node->right());

        if(inputl != nullptr)
            node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        else
            throw std::string("Unknown node with unspecified type.");

        if(inputr != nullptr)
            node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        else
            throw std::string("Unknown node with unspecified type.");
        node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node*>(node->left());

        if(inputl != nullptr) {
            if(node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT))
                node->left()->type(node->right()->type());
            else
                throw std::string("Invalid expression in right argument of binary expression.");
        }
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputr = dynamic_cast<l22::input_node*>(node->right());

        if(inputr != nullptr) {
            if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->left()->is_typed(cdk::TYPE_INT))
                node->right()->type(node->left()->type());
            else
                throw std::string("Invalid expression in left argument of binary expression.");
        }
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else
        throw std::string("Wrong types in binary expression.");}
void l22::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
    do_IDExpression(node,lvl);
}
void l22::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
    do_IDExpression(node,lvl);
}
void l22::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
    do_IntOnlyExpression(node,lvl);
}
void l22::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
    do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
    do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
    do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
    do_ScalarLogicalExpression(node, lvl);
}
void l22::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
    do_GeneralLogicalExpression(node, lvl);
}
void l22::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
    do_GeneralLogicalExpression(node, lvl);
}

//---------------------------------------------------------------------------

void l22::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<l22::symbol> symbol = _symtab.find(id);
    std::cout << symbol << std::endl;

  if (symbol != nullptr) {
      std::cout << symbol->value() << "ESTE0" << std::endl;
      node->type(symbol->type());
  } else {
      throw "undeclared variable '" + id + "'";
  }
}

void l22::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}
// TODO ver este
void l22::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->lvalue()->accept(this, lvl + 2);
    node->rvalue()->accept(this, lvl + 2);
    if(node->lvalue()->is_typed(cdk::TYPE_UNSPEC))
        throw std::string("Left value must have a type.");

    if(node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputr = dynamic_cast<l22::input_node *>(node->rvalue());
        l22::stack_alloc_node *stackr = dynamic_cast<l22::stack_alloc_node *>(node->rvalue());

        if(inputr != nullptr) {
            if(node->lvalue()->is_typed(cdk::TYPE_INT) || node->lvalue()->is_typed(cdk::TYPE_DOUBLE))
                node->rvalue()->type(node->lvalue()->type());
            else
                throw std::string("Invalid expression for lvalue node.");
        }
        else if(stackr != nullptr) {
            if(node->lvalue()->is_typed(cdk::TYPE_POINTER))
                node->rvalue()->type(node->lvalue()->type());
            else
                throw std::string("A pointer is required to allocate.");
        }
        else
            throw std::string("Unknown node with unspecified type");
    }

    if(node->lvalue()->is_typed(cdk::TYPE_INT) && node->rvalue()->is_typed(cdk::TYPE_INT)) {
        node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
    else if(node->lvalue()->is_typed(cdk::TYPE_DOUBLE) && (node->rvalue()->is_typed(cdk::TYPE_DOUBLE) || node->rvalue()->is_typed(cdk::TYPE_INT))) {
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    }
    else if(node->lvalue()->is_typed(cdk::TYPE_STRING) && node->rvalue()->is_typed(cdk::TYPE_STRING)) {
        node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
    }
    else if(node->lvalue()->is_typed(cdk::TYPE_POINTER) && node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
        l22::null_ptr_node *n = dynamic_cast<l22::null_ptr_node *>(node->rvalue());

        if(n == nullptr)
            typeOfPointer(cdk::reference_type::cast(node->lvalue()->type()), cdk::reference_type::cast(node->rvalue()->type()));
        node->type(node->lvalue()->type());
    }
    else if(node->lvalue()->is_typed(cdk::TYPE_STRUCT) && node->rvalue()->is_typed(cdk::TYPE_STRUCT))
        throw std::string("Cheguei");
    else {
        throw std::string("wrong types in assignment");
    }
}

//---------------------------------------------------------------------------

void l22::type_checker::do_program_node(l22::program_node *const node, int lvl) {
  // EMPTY
}

void l22::type_checker::do_evaluation_node(l22::evaluation_node *const node, int lvl) {
    node->argument()->accept(this, lvl + 2);

    if(node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *input = dynamic_cast<l22::input_node *>(node->argument());

        if(input != nullptr) {
            node->argument()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
        }
        else {
            throw std::string("Unknown node with unspecified type.");
        }
    }
}

void l22::type_checker::do_write_node(l22::write_node *const node, int lvl) {
    node->arguments()->accept(this, lvl + 2);

    for(size_t i = 0; i < node->arguments()->size(); i++) {
        cdk::expression_node *expression = dynamic_cast<cdk::expression_node *>(node->arguments()->node(i));
        if (expression != nullptr && expression->is_typed(cdk::TYPE_VOID)){
            throw std::string("Wrong type in write argument.");
        }

    }
}

//---------------------------------------------------------------------------
// TODO assert???
void l22::type_checker::do_input_node(l22::input_node *const node, int lvl) {
    node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void l22::type_checker::do_while_node(l22::while_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void l22::type_checker::do_if_node(l22::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void l22::type_checker::do_if_else_node(l22::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}
//----------------------------------------------------------------------------

// TODO leicmybox
void l22::type_checker::do_IntOnlyExpression(cdk::binary_operation_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->left()->accept(this, lvl + 2);
    if (node->left()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node *>(node->left());

        if (inputl != nullptr)
            node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        else
            throw std::string("Unknown node with unspecified type.");

    }
    node->right()->accept(this, lvl + 2);
    if (node->left()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node *>(node->left());

        if (inputl != nullptr)
            node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        else
            throw std::string("Unknown node with unspecified type.");

    }
    if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
        node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
        throw std::string("Integer expression expected in (left and right) binary operators.");
}
// TODO professor
void l22::type_checker::do_IDExpression(cdk::binary_operation_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->left()->accept(this, lvl + 2);
    node->right()->accept(this, lvl + 2);

    if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
        node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
        node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else if (node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node*>(node->left());
        l22::input_node *inputr = dynamic_cast<l22::input_node*>(node->right());

        if(inputl != nullptr && inputr != nullptr) {
            node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
            node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
            node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        }
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node*>(node->left());

        if(inputl != nullptr) {
            node->left()->type(node->right()->type());
            node->type(node->right()->type());
        }
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputr = dynamic_cast<l22::input_node*>(node->right());

        if(inputr != nullptr) {
            node->right()->type(node->left()->type());
            node->type(node->left()->type());
        }
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else
        throw std::string("Wrong types in binary expression.");
}

// TODO leicmyballs
void l22::type_checker::do_ScalarLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->left()->accept(this, lvl + 2);

    if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node*>(node->left());

        if(inputl != nullptr)
            node->left()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else if(!node->left()->is_typed(cdk::TYPE_INT) && !node->left()->is_typed(cdk::TYPE_DOUBLE))
        throw std::string("Wrong binary logical expression (expected integer or double).");

    node->right()->accept(this, lvl + 2);
    if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputr = dynamic_cast<l22::input_node*>(node->right());

        if(inputr != nullptr)
            node->right()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else if(!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE))
        throw std::string("Wrong binary logical expression (expected integer or double).");

    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}
// TODO leimyballs
void l22::type_checker::do_BooleanLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
    ASSERT_UNSPEC;
    node->left()->accept(this, lvl + 2);

    if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node *>(node->left());

        if(inputl != nullptr)
            node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else if(!node->left()->is_typed(cdk::TYPE_INT))
        throw std::string("Integer expression expected in (left and right) binary operators.");

    node->right()->accept(this, lvl + 2);
    if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputr = dynamic_cast<l22::input_node *>(node->right());

        if(inputr != nullptr)
            node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        else
            throw std::string("Unknown node with unspecified type.");
    }
    else if(!node->right()->is_typed(cdk::TYPE_INT))
        throw std::string("Integer expression expected in (left and right) binary operators.");

    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

// TODO leicmyballs
void l22::type_checker::do_GeneralLogicalExpression(cdk::binary_operation_node *const node, int lvl) {

    ASSERT_UNSPEC;
    node->left()->accept(this, lvl + 2);

    if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputl = dynamic_cast<l22::input_node *>(node->left());

        if(inputl != nullptr)
            node->left()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
        else
            throw std::string("Unknown node with unspecified type.");
    }

    node->right()->accept(this, lvl + 2);
    if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
        l22::input_node *inputr = dynamic_cast<l22::input_node *>(node->right());

        if(inputr != nullptr)
            node->left()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
        else
            throw std::string("Unknown node with unspecified type.");
    }

    if(node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER))
        typeOfPointer(cdk::reference_type::cast(node->left()->type()),cdk::reference_type::cast(node->right()->type()));
    else if(node->left()->type()->name() != node->right()->type()->name()) {
        if(!((node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))||
             (node->left()->is_typed(cdk::TYPE_INT)  &&  node->right()->is_typed(cdk::TYPE_INT))))
            throw std::string("Operator has incompatible types.");
    }
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

std::shared_ptr<cdk::basic_type> l22::type_checker::typeOfPointer(std::shared_ptr<cdk::reference_type> leftPtr, std::shared_ptr<cdk::reference_type> rightPtr){
    std::shared_ptr<cdk::basic_type> left, right;
    left = leftPtr;
    right = rightPtr;
    while (left->name() == cdk::TYPE_POINTER && right->name() == cdk::TYPE_POINTER) {
        left = cdk::reference_type::cast(left)->referenced();
        right = cdk::reference_type::cast(right)->referenced();
    }
    if (left->name() == cdk::TYPE_POINTER || right->name() == cdk::TYPE_POINTER)
        throw std::string("Wrong pointer type.");
    if (left->name() == cdk::TYPE_INT && right->name() == cdk::TYPE_INT)
        return cdk::primitive_type::create(4, cdk::TYPE_INT);
    else if (left->name() == cdk::TYPE_DOUBLE && right->name() == cdk::TYPE_DOUBLE)
        return cdk::primitive_type::create(8, cdk::TYPE_DOUBLE);
    else if (left->name() == cdk::TYPE_STRING && right->name() == cdk::TYPE_STRING)
        return cdk::primitive_type::create(4, cdk::TYPE_STRING);
    else // FIXME AUTO
        throw std::string("Wrong pointer type.");
}