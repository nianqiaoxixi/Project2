#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <map>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"
#include "parser.h"

using namespace Boost::Internal;

std::string index_pool[3] = {"h", "w", "t"};
class MyPrinter : public IRPrinter {
 public:
    int op_flag;
    bool index_flag;
    MyPrinter() : IRPrinter() {
        op_flag = -1;
        index_flag = false;
    }
    void visit(Ref<const Var> op) {
        if (global_flag) {
            if (op->type().is_float()) {
                oss << "float ";
            }
            else {
                oss << "int ";
            }
        }
        oss << op->name;
        if ((op->shape)[0] == 1) {
        }
        else {    
            for (size_t i = 0; i < op->args.size(); ++i) {
                oss << "[";
                index_flag = true;
                op->args[i].visit_expr(this);
                oss << "]";
            }
            index_flag = false;
        }
    }
    void visit(Ref<const Binary> op) {
        if (op->op_type == BinaryOpType::Add || op->op_type == BinaryOpType::Sub) {
            oss << "(";
        }
        (op->a).visit_expr(this);
        if (op->op_type == BinaryOpType::Add) {
            op_flag = 0;
            oss << " + ";
        } else if (op->op_type == BinaryOpType::Sub) {
            op_flag = 0;
            oss << " - ";
        } else if (op->op_type == BinaryOpType::Mul) {
            op_flag = 1;
            oss << " * ";
        } else if (op->op_type == BinaryOpType::Div) {
            op_flag = 1;
            oss << " / ";
        } else if (op->op_type == BinaryOpType::Mod) {
            op_flag = 1;
            oss << " % ";
        } else if (op->op_type == BinaryOpType::And) {
            op_flag = 2;
            oss << " && ";
        } else if (op->op_type == BinaryOpType::Or) {
            op_flag = 2;
            oss << " || ";
        }
        (op->b).visit_expr(this);
        if (op->op_type == BinaryOpType::Add || op->op_type == BinaryOpType::Sub) {
            oss << ")";
        }
    }
    void visit(Ref<const IntImm> op) {
        if ((op_flag == 0)&&(index_flag == false))
            oss << "0.0";
        else
            oss << op->value();
    }


    void visit(Ref<const UIntImm> op) {
        if ((op_flag == 0)&&(index_flag == false))
            oss << "0.0";
        else
            oss << op->value();
    }


    void visit(Ref<const FloatImm> op) {
        if ((op_flag == 0)&&(index_flag == false))
            oss << "0.0";
        else
            oss << op->value();
    }
};

class MyMutator : public IRMutator {
 public:
    Type data_type;
    Type index_type;
    Expr temp_expr;
    std::vector<Expr> src_expr;
    Expr dst_expr;
    std::string out;
    std::vector<std::string> grad;
    int grad_index;
    int mode;
    bool if_remove;
    std::map<std::string, Expr> input_var;

    int index_use;
    std::map<std::string, Expr> match;
    std::map<std::string, Expr> left_index;
    std::map<std::string, Expr> extra_index;
    
    Ref<const Index> index_temp;
    Expr global_index;
    std::vector<Expr> new_indexs;

    MyMutator(std::string _out, std::vector<std::string> _grad, Type _data_type, Type _index_type): IRMutator() {
        mode = 0;
        out = _out;
        for (auto g : _grad) {
            grad.push_back(g);
            src_expr.push_back(Expr(0));
        }
        grad_index = 0;
        index_use = 0;
        if_remove = false;
        data_type = _data_type;
        index_type = _index_type;
        Expr expr = Expr(0);
        temp_expr = Var::make(data_type, "temp", {expr}, {1});
    }

    void reset() {
        index_use = 0;
        match.erase(match.begin(), match.end());
        left_index.erase(left_index.begin(), left_index.end());
        extra_index.erase(extra_index.begin(), extra_index.end());
        new_indexs.clear();
    }

    Expr visit(Ref<const Var> op) override {
        switch(mode) {
            //get the two Expr to be changed
            case 1:
                if (op->name == grad[grad_index]) {
                    std::vector<Expr> new_args;
                    for (auto arg : op->args) {
                        mode = 3;
                        new_args.push_back(mutate(arg));
                    }
                    for (auto arg : op->args) {
                        mode = 6;
                        mutate(arg);
                    }
                    mode = 1;
                    //get the grad Expr from src
                    src_expr[grad_index] = Var::make(op->type(), ("d" + grad[grad_index]), new_args, op->shape);
                }
                else if (op->name == out) {
                    //get the out Expr from dst
                    dst_expr = Var::make(op->type(), ("d" + out), op->args, op->shape);
                }
                else {
                    input_var[op->name] = op;
                }
                return IRMutator::visit(op);
            //change the two Expr
            case 2:
                if (op->name == grad[grad_index]) {
                    return dst_expr;
                }
                else if (op->name == out) {
                    for (auto arg : op->args) {
                        mutate(arg);
                    }
                    return temp_expr;
                }
                else {
                    for (auto arg : op->args) {
                        mutate(arg);
                    }
                    return IRMutator::visit(op);
                }
            case 7:
                if (input_var.find(op->name) == input_var.end()) {
                    if_remove = true;
                }
                return IRMutator::visit(op);
            default:
                if (op->name == grad[grad_index]) {
                    return Var::make(op->type(), ("d" + grad[grad_index]), op->args, op->shape);
                }
                else if (op->name == out) {
                    return Var::make(op->type(), ("d" + out), op->args, op->shape);
                }
                else {
                    return IRMutator::visit(op);
                }
            }     
    }


    Expr visit(Ref<const Index> op) override {
        switch (mode) {
            //get the indexs those appear on the right but not on the left
            case 2:
                if (left_index.find(op->name) == left_index.end()) {
                    extra_index[op->name] = op;
                }
                break;
            //when there is an operation in the left index, change it to a single one
            case 3:
                index_temp = op;
                break;
            //change the single index in the right index 
            case 4:
                match[op->name] = global_index;
                break;
            //generate the outmost loopnest, some indexs should be removed
            case 5:
                if (match.find(op->name) != match.end() ||
                    extra_index.find(op->name) != extra_index.end()) {
                    if_remove = true;
                    return IRMutator::visit(op);
                }
                break;
            case 6:
                left_index[op->name] = op;
                break;
        }
        if (match.find(op->name) != match.end()) {
            return match[op->name];
        }
        return IRMutator::visit(op);
    }

    Expr visit(Ref<const Binary> op) override {
        if (mode == 3) {
            if (op->op_type == BinaryOpType::Add) {
                mutate(op->a);
                Expr new_index = Index::make(index_temp->type(), index_pool[index_use++], 
                    index_temp->dom, index_temp->index_type);
                new_indexs.push_back(new_index);
                global_index = Binary::make(index_temp->type(), BinaryOpType::Sub, new_index, index_temp);
                mode = 4;
                mutate(op->b);
                mode = 1;
                return new_index;
            }
        }
        if (op->op_type == BinaryOpType::Mul) {
            if ((op->a).as<Var>() != nullptr && (op->b).as<Var>() != nullptr){
                if ((op->a).as<Var>()->name == (op->b).as<Var>()->name) {
                    mutate(op->a);
                    mutate(op->b);
                    input_var[(op->a).as<Var>()->name] = op->a;
                    Expr new_expr = Binary::make((op->a).type(), BinaryOpType::Add, op->a, op->b);
                    return Binary::make((op->a).type(), BinaryOpType::Mul, mutate(op->a), new_expr);
                }
            }
        }
        return IRMutator::visit(op);
    }

    Stmt visit(Ref<const Move> op) override {
        mode = 1;
        mutate(op->src);
        mutate(op->dst);
        mode = 2;
        Expr new_src = Binary::make(data_type, BinaryOpType::Add, temp_expr, mutate(op->src));
        Expr new_dst = mutate(op->dst);
        mode = 0;
        return Move::make(new_dst, new_src, op->move_type);
        //return IRMutator::visit(op);
    }

    Stmt visit(Ref<const LoopNest> op) override {
        std::vector<Expr> new_index_list;
        std::vector<Stmt> new_body_list;
        std::vector<Stmt> in_body_list;
        // temp = 0
        Stmt st = Move::make(temp_expr, Expr(0), MoveType::LocalToLocal);
        new_body_list.push_back(st);
        // temp = temp + ...
        for (auto body : op->body_list) {
            in_body_list.push_back(mutate(body));
        }

        //generate inner loopnest
        std::map<std::string, Expr>::iterator it;
        std::vector<Expr> index_vec;
        for (it = extra_index.begin(); it != extra_index.end(); it++) {
            index_vec.push_back(it->second);
        }
        st = LoopNest::make(index_vec, in_body_list);
        new_body_list.push_back(st);

        //... = temp
        st = Move::make(src_expr[grad_index], temp_expr, MoveType::MemToMem);
        new_body_list.push_back(st);

        for (auto index : op->index_list) {
            mode = 5;
            Expr expr = mutate(index);
            if (!if_remove) {
                new_index_list.push_back(expr);
            }
            else {
                if_remove = false;
            }
            mode = 0;
        }
        for (auto index : new_indexs) {
            new_index_list.push_back(index);
        }
        return LoopNest::make(new_index_list, new_body_list);
    }

    Group visit(Ref<const Kernel> op) override {
        std::vector<Expr> new_inputs;
        std::vector<Expr> new_outputs;
        std::vector<Stmt> new_stmt_list;
        while ((unsigned int)grad_index < grad.size()) {
            for (auto stmt : op->stmt_list) {
                new_stmt_list.push_back(mutate(stmt));
            }
            grad_index++;
            reset();
        }
        for (auto expr : op->inputs) {
            mode = 7;
            Expr ex = mutate(expr);
            if (!if_remove) {
                new_inputs.push_back(ex);
            }
            else {
                if_remove = false;
            }
            mode = 0;
        }
        
        for (auto expr : op->outputs) {
            new_inputs.push_back(mutate(expr));
        }
        
        for (auto expr : src_expr) {
            new_outputs.push_back(expr);
        }
        
        return Kernel::make(op->name, new_inputs, new_outputs, new_stmt_list, op->kernel_type);
    }
};

int solution(int i, std::string path, std::string outpath) {
    std::ifstream is(path, std::ios::binary);
    if(is) {
        if(!is.is_open()) {
            std::cout << "Open json file failed!" <<std::endl;
            return -1;
        }
        std::string str;
        std::string name;
        std::vector<std::string> ins;
        std::vector<std::string> outs;
        std::string type;
        std::string kernel;
        std::vector<std::string> grad_to;
        while (std::getline(is, str)) {
            int index0 = str.find("\"");
            int index1 = str.find(":");
            if (index1 == -1) {
                continue;
            }
            std::string domain = str.substr(index0 + 1);
            std::string content = str.substr(index1);
            index1 = domain.find("\"");
            domain = domain.substr(0, index1);
            
            if (domain == "name" || domain == "data_type") {
                index0 = content.find("\"");
                index1 = content.rfind("\"");
                if (domain == "name") {
                    name = content.substr(index0 + 1, index1 - index0 - 1);
                }
                else {
                    type = content.substr(index0 + 1, index1 - index0 - 1);
                }
            }
            else if (domain == "ins" || domain == "outs" || domain == "grad_to") {
                index0 = content.find("[");
                index1 = content.rfind("]");
                std::string temp = content.substr(index0 + 1, index1 - index0 - 1);
                index0 = temp.find("\"");
                while (index0 != -1) {
                    temp = temp.substr(index0 + 1);
                    index1 = temp.find("\"");
                    if (domain == "ins") {
                        ins.push_back(temp.substr(0,index1));
                    }
                    else if (domain == "outs") {
                        outs.push_back(temp.substr(0, index1));
                    }
                    else {
                        grad_to.push_back(temp.substr(0, index1));
                    }
                    temp = temp.substr(index1 + 1);
                    index0 = temp.find("\"");
                }
            }
            else {
                index0 = content.find("\"");
                index1 = content.rfind("\"");
                kernel = content.substr(index0 + 1, index1 - index0 - 1);
            }
        }
        
        Type index_type = Type::int_scalar(32);
        Type data_type;
        if (type == "int") {
            data_type = Type::int_scalar(32);
        }
        else {
            data_type = Type::float_scalar(32);
        }

        is.close();

        Group main_code = parser(kernel, index_type, data_type, ins, outs, name);
        
        // visitor
        IRVisitor visitor;
        main_code.visit_group(&visitor);

        // mutator
        MyMutator mutator(outs[0], grad_to, data_type, index_type);
        main_code = mutator.mutate(main_code);

        // printer
        MyPrinter printer;
        std::string code = printer.print(main_code);

        std::ofstream ofile(outpath, std::ios::out);
        ofile << "#include \"../run2.h\"\n";
        ofile << "\n";
        ofile << code;
        ofile << "\n";
        ofile.close();
        std::cout << "Success!\n";
    }
    return 1;
}

int main() {
    std::string casePath = "./cases/case";
    std::string kernelPath = "./kernels/grad_case";
    for(int i = 1; i <= 10; i ++) {
        if (i == 6 || i == 8 || i == 10)
            continue;
        std::string path = casePath + std::to_string(i) + ".json";
        std::string outpath = kernelPath + std::to_string(i) + ".cc";
        solution(i, path, outpath);
    }
    return 0;
}