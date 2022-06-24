#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated

//---------------------------------------------------------------------------

void l22::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
    // EMPTY
}

void l22::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
    // EMPTY
}

void l22::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
    if (_inFunctionBody) {
        _pf.DOUBLE(node->value()); // load number to the stack
    } else {
        _pf.SDOUBLE(node->value());    // double is on the DATA segment
    }
}

void l22::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->argument()->accept(this, lvl); //lvl +2 ?
    _pf.INT(0);
    _pf.EQ();
}

void l22::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    int lbl = ++_lbl;
    node->left()->accept(this, lvl);
    _pf.DUP32();
    _pf.JZ(mklbl(lbl));
    node->right()->accept(this, lvl);
    _pf.AND();
    _pf.ALIGN();
    _pf.LABEL(mklbl(lbl));
}

void l22::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    int lbl = ++_lbl;
    node->left()->accept(this, lvl);
    _pf.DUP32();
    _pf.JNZ(mklbl(lbl));
    node->right()->accept(this, lvl);
    _pf.OR();
    _pf.ALIGN();
    _pf.LABEL(mklbl(lbl));
}

void l22::postfix_writer::do_address_of_node(l22::address_of_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS
    node->argument()->accept(this, lvl);
}

void l22::postfix_writer::do_sizeof_node(l22::sizeof_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS //copiei de cima pq parece a mma vibe
    node->argument()->accept(this, lvl);
}

void l22::postfix_writer::do_again_node(l22::again_node * const node, int lvl) {
    if(_next.empty()){
        throw std::string("next instruction not in a cycle");
    }
    else{
        _pf.JMP(_next.back());
    }
}

void l22::postfix_writer::do_return_node(l22::return_node * const node, int lvl) {

}

void l22::postfix_writer::do_stop_node(l22::stop_node * const node, int lvl) {
    if(_stop.empty()){
        throw std::string("stop instruction not in a cycle");
    }
    else{
        _pf.JMP(_stop.back());
    }
}

void l22::postfix_writer::do_block_node(l22::block_node * const node, int lvl) {
    //_symtab.push(); // for block-local vars SIM OU NAO?
    std::cout << node->instructions() << std::endl;
    if (node->declarations() == nullptr)
        std::cout << node->declarations() << std::endl;

    if (node->declarations())
        node->declarations()->accept(this, lvl + 2);
    if (node->instructions())
        node->instructions()->accept(this, lvl + 2);
    std::cout << "           saiu blocknode     " << std::endl;

    //_symtab.pop();
}

void l22::postfix_writer::do_function_call_node(l22::function_call_node * const node, int lvl) {
    // EMPTY
}

void l22::postfix_writer::do_function_definition_node(l22::function_definition_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;

    if (_inFunctionBody || _inFunctionArgs) {
        throw std::string("cannot define function in body or in arguments");
        return;
    }

    // remember symbol so that args and body know
    _function = new_symbol();
    _functions_to_declare.erase(_function->name());  // just in case
    reset_new_symbol();

    _currentReturnJumpLabel = mklbl(++_lbl);

    _offset = 8; // prepare for arguments (4: remember to account for return address)
    _symtab.push(); // scope of args

    if (node->arguments()->size() > 0) {
        _inFunctionArgs = true;
        for (size_t ix = 0; ix < node->arguments()->size(); ix++) {
            cdk::basic_node *arg = node->arguments()->node(ix);
            if (arg == nullptr) break; // this means an empty sequence of arguments
            arg->accept(this, 0); // the function symbol is at the top of the stack
        }
        _inFunctionArgs = false;
    }

    _pf.TEXT();
    _pf.ALIGN();
    /*if (node->qualifier() == tPUBLIC)*/ _pf.GLOBAL(_function->name(), _pf.FUNC());
    _pf.LABEL(_function->name());

    // compute stack size to be reserved for local variables
//    frame_size_calculator lsc(_compiler, _symtab, _function);
//    node->accept(&lsc, lvl);
//    _pf.ENTER(lsc.localsize()); // total stack size reserved for local variables

    _offset = 0; // prepare for local variable

    _newRetValSeen = false;
//    _currentFunctionName = node->identifier();
    // the following flag is a slight hack: it won't work with nested functions
    _inFunctionBody = true;
    node->block()->accept(this, lvl + 4); // body has its own scope
    std::shared_ptr<cdk::functional_type> node_type = cdk::functional_type::cast(node->type());

//    if(!_newRetValSeen) {
//        if (node->type()->name() != cdk::TYPE_VOID) {
//
//             set value
//            if(node->type()->name() == cdk::TYPE_INT) {
//                if(node_type->output())
//                    node_type->output(0)(this, lvl + 2);
//                else
//                    _pf.INT(0);
//            }
//            else if(node->type()->name() == cdk::TYPE_DOUBLE) {
//                if(node->retval()) {
//                    node->retval()->accept(this, lvl + 2);
//                    if(node->retval()->type()->name() == cdk::TYPE_INT)
//                        _pf.I2D();
//                }
//                else
//                    _pf.DOUBLE(0);
//            }

    // make it a return value
    if (node->type()->name() == cdk::TYPE_INT || node->type()->name() == cdk::TYPE_STRING || node->type()->name() == cdk::TYPE_POINTER)
        _pf.STFVAL32();
    else if (node->type()->name() == cdk::TYPE_DOUBLE)
        _pf.STFVAL64();
    else
        std::cerr << node->lineno() << ": should not happen: unknown return type" << std::endl;


    _inFunctionBody = false;
    _newRetValSeen = false;
    _currentFunctionName = "";

    _pf.LEAVE();
    _pf.RET();

    _symtab.pop(); // scope of arguments

//    if (node->identifier() == "fir") {
//         declare external functions
//        for (std::string s : _functions_to_declare)
//            _pf.EXTERN(s);
//    }
}

void l22::postfix_writer::do_index_node(l22::index_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS; //será?? nao faco idea
    if (node->base()) {
        node->base()->accept(this, lvl);
    } else {
        if (_function) {
            _pf.LOCV(-_function->type()->size());
        } else {
            std::cerr << "FATAL: " << node->lineno() << ": trying to use return value outside function" << std::endl;
        }
    }
    node->index()->accept(this, lvl);
    _pf.INT(3);
    _pf.SHTL();
    _pf.ADD(); // add pointer and index
}

void l22::postfix_writer::do_null_ptr_node(l22::null_ptr_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS; //talvez nao seja pq diferente no enunciado
    if (_inFunctionBody) {
        _pf.INT(0);
    } else {
        _pf.SINT(0);
    }
}

void l22::postfix_writer::do_stack_alloc_node(l22::stack_alloc_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->argument()->accept(this, lvl);
    if(cdk::reference_type::cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE)
        _pf.INT(3);
    else
        _pf.INT(2);

    _pf.SHTL();
    _pf.ALLOC();
    _pf.SP();
}

void l22::postfix_writer::do_variable_declaration_node(l22::variable_declaration_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    std::cout << "            ENTROU  vardeclnode         " << std::endl;

    auto id = node->identifier();

    std::cout << "INITIAL OFFSET: " << _offset << std::endl;

    if (node->type() == nullptr)
        std::cout << "           aqui    " << std::endl;

    // type size?
    int offset = 0, typesize = node->type()->size(); // in bytes

    std::cout << "ARG: " << id << ", " << typesize << std::endl;
    if (_inFunctionBody) {
        std::cout << "IN BODY" << std::endl;
        _offset -= typesize;
        offset = _offset;
    } else if (_inFunctionArgs) {
        std::cout << "IN ARGS" << std::endl;
        offset = _offset;
        _offset += typesize;
    } else {
        std::cout << "GLOBAL!" << std::endl;
        offset = 0; // global variable
    }
    std::cout << "OFFSET: " << id << ", " << offset << std::endl;

    auto symbol = new_symbol();
    if (symbol) {
        symbol->set_offset(offset);
        reset_new_symbol();
    }

    if (_inFunctionBody) {
        // if we are dealing with local variables, then no action is needed
        // unless an initializer exists
        if (node->initializer()) {
            node->initializer()->accept(this, lvl);
            if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING) || node->is_typed(cdk::TYPE_POINTER)
                || node->is_typed(cdk::TYPE_FUNCTIONAL)) {
                _pf.LOCAL(symbol->offset());
                _pf.STINT();
            } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
                if (node->initializer()->is_typed(cdk::TYPE_INT))
                    _pf.I2D();
                _pf.LOCAL(symbol->offset());
                _pf.STDOUBLE();
            } else {
                std::cerr << "cannot initialize" << std::endl;
            }
        }
    } else {
        if (!_function) {
            if (node->initializer() == nullptr) {
                _pf.BSS();
                _pf.ALIGN();
                _pf.LABEL(id);
                _pf.SALLOC(typesize);
            } else {

                if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE) || node->is_typed(cdk::TYPE_POINTER)) {
                    if (node->constant()) {
                        _pf.RODATA();
                    } else {
                        _pf.DATA();
                    }
                    _pf.ALIGN();
                    _pf.LABEL(id);

                    if (node->is_typed(cdk::TYPE_INT)) {
                        node->initializer()->accept(this, lvl);
                    } else if (node->is_typed(cdk::TYPE_POINTER)) {
                        node->initializer()->accept(this, lvl);
                    } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
                        if (node->initializer()->is_typed(cdk::TYPE_DOUBLE)) {
                            // entra aqui mas da seg fault
                            std::cout << node->initializer() << std::endl;

//                            node->initializer()->accept(this, lvl);

                        } else if (node->initializer()->is_typed(cdk::TYPE_INT)) {
                            cdk::integer_node *dclini = dynamic_cast<cdk::integer_node*>(node->initializer());
                            cdk::double_node ddi(dclini->lineno(), dclini->value());
                            ddi.accept(this, lvl);
                        } else {
                            std::cerr << node->lineno() << ": '" << id << "' has bad initializer for real value\n";
                            _errors = true;
                        }
                    }
                } else if (node->is_typed(cdk::TYPE_STRING)) {
                    if (node->constant()) {
                        int litlbl;
                        // HACK!!! string literal initializers must be emitted before the string identifier
                        _pf.RODATA();
                        _pf.ALIGN();
                        _pf.LABEL(mklbl(litlbl = ++_lbl));
                        _pf.SSTRING(dynamic_cast<cdk::string_node*>(node->initializer())->value());
                        _pf.ALIGN();
                        _pf.LABEL(id);
                        _pf.SADDR(mklbl(litlbl));
                    } else {
                        _pf.DATA();
                        _pf.ALIGN();
                        _pf.LABEL(id);
                        node->initializer()->accept(this, lvl);
                    }
                } else {
                    std::cerr << node->lineno() << ": '" << id << "' has unexpected initializer\n";
                    _errors = true;
                }

            }

        }
    }
    std::cout << "            ENTROU 2          " << std::endl;

}

void l22::postfix_writer::do_identity_node(l22::identity_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->argument()->accept(this, lvl);
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
    for (size_t i = 0; i < node->size(); i++) {
        node->node(i)->accept(this, lvl);
    }
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
    _pf.INT(node->value()); // push an integer
}

void l22::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
    int lbl1;

//    generate the string
    _pf.RODATA(); // strings are DATA readonly
    _pf.ALIGN(); // make sure we are aligned
    _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
    _pf.SSTRING(node->value()); // output string characters

//    leave the address on the stack
    _pf.TEXT(); // return to the TEXT segment
    _pf.ADDR(mklbl(lbl1)); // the string to be printed
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->argument()->accept(this, lvl); // determine the value
    _pf.NEG(); // 2-complement
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.ADD();
}
void l22::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.SUB();
}
void l22::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.MUL();
}
void l22::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.DIV();
}
void l22::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.MOD();
}
void l22::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.LT();
}
void l22::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.LE();
}
void l22::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.GE();
}
void l22::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.GT();
}
void l22::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.NE();
}
void l22::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->left()->accept(this, lvl);
    node->right()->accept(this, lvl);
    _pf.EQ();
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    // simplified generation: all variables are global
    _pf.ADDR(node->name());
}

void l22::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->lvalue()->accept(this, lvl);
    _pf.LDINT(); // depends on type size
}

void l22::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->rvalue()->accept(this, lvl); // determine the new value
    _pf.DUP32();
    if (new_symbol() == nullptr) {
        node->lvalue()->accept(this, lvl); // where to store the value
    } else {
        _pf.DATA(); // variables are all global and live in DATA
        _pf.ALIGN(); // make sure we are aligned
        _pf.LABEL(new_symbol()->name()); // name variable location
        reset_new_symbol();
        _pf.SINT(0); // initialize it to 0 (zero)
        _pf.TEXT(); // return to the TEXT segment
        node->lvalue()->accept(this, lvl);  //DAVID: bah!
    }
    _pf.STINT(); // store the value at address
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_program_node(l22::program_node * const node, int lvl) {
    // Note that Simple doesn't have functions. Thus, it doesn't need
    // a function node. However, it must start in the main function.
    // The ProgramNode (representing the whole program) doubles as a
    // main function node.

    // generate the main function (RTS mandates that its name be "_main")
    _pf.TEXT();
    _pf.ALIGN();
    _pf.GLOBAL("_main", _pf.FUNC());
    _pf.LABEL("_main");
    _pf.ENTER(0);  // Simple doesn't implement local variables

    node->statements()->accept(this, lvl);

    // end the main function
    _pf.INT(0);
    _pf.STFVAL32();
    _pf.LEAVE();
    _pf.RET();

    // these are just a few library function imports
    _pf.EXTERN("readi");
    _pf.EXTERN("printi");
    _pf.EXTERN("prints");
    _pf.EXTERN("println");
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_evaluation_node(l22::evaluation_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    node->argument()->accept(this, lvl); // determine the value
    if (node->argument()->is_typed(cdk::TYPE_INT)) {
        _pf.TRASH(4); // delete the evaluated value
    } else if (node->argument()->is_typed(cdk::TYPE_STRING)) {
        _pf.TRASH(4); // delete the evaluated value's address
    } else {
        std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
        exit(1);
    }
}

void l22::postfix_writer::do_write_node(l22::write_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;

    std::cout << "            ENTROU    write       " << std::endl;

    for (size_t ix = 0; ix < node->arguments()->size(); ix++) {
        cdk::expression_node *child = dynamic_cast<cdk::expression_node*>(node->arguments()->node(ix));
        std::cout << child->type()<< std::endl;


        std::shared_ptr<cdk::basic_type> etype = child->type();
        std::cout << "            ENTORU foriculo      2     " << std::endl;

        child->accept(this, lvl); // expression to print
        std::cout << "            ENTORU foriculo      3     " << std::endl;
        if (child->type() == 0) //ta null
            std::cout << "        yau ta nullzord       " << std::endl;

        if (etype->name() == cdk::TYPE_INT) {
            std::cout << "entrou inteiro" << std::endl;
            _functions_to_declare.insert("printi");
            _pf.CALL("printi");
            _pf.TRASH(4); // trash int
        } else if (etype->name() == cdk::TYPE_DOUBLE) {
            std::cout << "entrou double" << std::endl;
            _functions_to_declare.insert("printd");
            _pf.CALL("printd");
            _pf.TRASH(8); // trash double
        } else if (etype->name() == cdk::TYPE_STRING) {
            std::cout << "entrou string" << std::endl;
            _functions_to_declare.insert("prints");
            _pf.CALL("prints");
            _pf.TRASH(4); // trash char pointer
        } else {
            std::cout << "            ENTROU else         " << std::endl;
            std::cerr << "cannot print expression of unknown type" << std::endl;
            return;
        }

    }

    if (node->newline()) {
        _functions_to_declare.insert("println");
        _pf.CALL("println");
    }
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_input_node(l22::input_node * const node, int lvl) {
//  ASSERT_SAFE_EXPRESSIONS;
//  _pf.CALL("readi");
//  _pf.LDFVAL32();
//  node->argument()->accept(this, lvl);
//  _pf.STINT();
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_while_node(l22::while_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;

    std::string cond = mklbl(++_lbl);
    _next.push_back(cond);
    std::string end = mklbl(++_lbl);
    _stop.push_back(end);

    _pf.LABEL(cond);
    node->condition()->accept(this, lvl);
    _pf.JZ(end);
    node->block()->accept(this, lvl + 2);
    _pf.JMP(cond);
    _pf.LABEL(end);

    _next.pop_back();
    _stop.pop_back();
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_if_node(l22::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void l22::postfix_writer::do_if_else_node(l22::if_else_node * const node, int lvl) {
    ASSERT_SAFE_EXPRESSIONS;
    int lbl1, lbl2;
    node->condition()->accept(this, lvl);
    _pf.JZ(mklbl(lbl1 = ++_lbl));
    node->thenblock()->accept(this, lvl + 2);
    _pf.JMP(mklbl(lbl2 = ++_lbl));
    _pf.LABEL(mklbl(lbl1));
    node->elseblock()->accept(this, lvl + 2);
    _pf.LABEL(mklbl(lbl1 = lbl2));
}