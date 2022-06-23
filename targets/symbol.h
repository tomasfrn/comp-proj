#ifndef __L22_TARGETS_SYMBOL_H__
#define __L22_TARGETS_SYMBOL_H__

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>

namespace l22 {

    class symbol {
        int _qualifier;
        std::shared_ptr<cdk::basic_type> _type;
        std::string _name;
        bool _function;
        long _value; // hack!
        bool _forward = false;
        int _offset = 0;
        std::vector<std::shared_ptr<cdk::basic_type>> _arguments;

    public:
        symbol(int qualifier, std::shared_ptr<cdk::basic_type> type, const std::string &name, bool function,
               long value, bool forward = false) :
                _qualifier(qualifier),_type(type), _name(name), _function(function), _value(value), _forward(forward) {
        }

        virtual ~symbol() {
            // EMPTY
        }

        std::vector<std::shared_ptr<cdk::basic_type>> arguments() {
            return _arguments;
        }

        void setArguments(std::vector<std::shared_ptr<cdk::basic_type>> arguments)
        {
            _arguments = arguments;
        }
        int qualifier() {
            return _qualifier;
        }

        std::shared_ptr<cdk::basic_type> type() const {
            return _type;
        }
        bool is_typed(cdk::typename_type name) const {
            return _type->name() == name;
        }
        const std::string &name() const {
            return _name;
        }
        void set_type(std::shared_ptr<cdk::basic_type> t) {
            _type = t;
        }

        bool isFunction() const {
            return _function;
        }
        bool forward() const {
            return _forward;
        }

        long value() const {
            return _value;
        }

        long value(long v) {
            return _value = v;
        }

        int offset() const {
            return _offset;
        }

        void set_offset(int offset) {
            _offset = offset;
        }
    };

} // l22

#endif
