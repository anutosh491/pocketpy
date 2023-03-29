#pragma once

#include "obj.h"
#include "error.h"

namespace pkpy{

enum NameScope {
    NAME_LOCAL = 0,
    NAME_GLOBAL,
    NAME_ATTR,
    NAME_SPECIAL,
};

enum Opcode {
    #define OPCODE(name) OP_##name,
    #include "opcodes.h"
    #undef OPCODE
};

inline const char* OP_NAMES[] = {
    #define OPCODE(name) #name,
    #include "opcodes.h"
    #undef OPCODE
};

struct Bytecode{
    uint8_t op;
    uint16_t block;
    int arg;
    int line;
};

enum CodeBlockType {
    NO_BLOCK,
    FOR_LOOP,
    WHILE_LOOP,
    CONTEXT_MANAGER,
    TRY_EXCEPT,
};

struct CodeBlock {
    CodeBlockType type;
    int parent;         // parent index in blocks
    int start;          // start index of this block in codes, inclusive
    int end;            // end index of this block in codes, exclusive
};

struct CodeObject {
    shared_ptr<SourceData> src;
    Str name;
    bool is_generator = false;

    CodeObject(shared_ptr<SourceData> src, Str name) {
        this->src = src;
        this->name = name;
    }

    std::vector<Bytecode> codes;
    List consts;
    std::vector<std::pair<StrName, NameScope>> names;
    std::map<StrName, int> global_names;
    std::vector<CodeBlock> blocks = { CodeBlock{NO_BLOCK, -1} };
    std::map<StrName, int> labels;

    uint32_t perfect_locals_capacity = 2;
    uint32_t perfect_hash_seed = 0;

    void optimize(VM* vm);

    bool add_label(StrName label){
        if(labels.count(label)) return false;
        labels[label] = codes.size();
        return true;
    }

    int add_name(StrName name, NameScope scope){
        if(scope == NAME_LOCAL && global_names.count(name)) scope = NAME_GLOBAL;
        auto p = std::make_pair(name, scope);
        for(int i=0; i<names.size(); i++){
            if(names[i] == p) return i;
        }
        names.push_back(p);
        return names.size() - 1;
    }

    int add_const(PyObject* v){
        consts.push_back(v);
        return consts.size() - 1;
    }

    void _mark() const {
        for(PyObject* v : consts) OBJ_MARK(v);
    }
};


} // namespace pkpy