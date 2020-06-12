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
        oss << "(";
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
        oss << ")";
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

    void visit(Ref<const Kernel> op) {
        print_indent();
        std::string str = "d";
        oss << "void " << op->name << "(";
        print_arg = true;
        size_t num1 = 0;
        size_t num2 = 0;
        for (size_t i = 0; i < op->inputs.size(); ++i) {
            if (((op->inputs[i]).as<Var>()->name).at(0) != str.at(0)) {
                if (op->inputs[i].type().is_float()) {
                    oss << "float";
                }
                if (op->inputs[i].type().is_int()) {
                    oss << "int";
                }
                oss << " (&";
                oss << (op->inputs[i]).as<Var>()->name;
                oss << ")";
                if (((op->inputs[i]).as<Var>()->shape[0]) == 1) {

                }
                else {
                    for (size_t j = 0; j < (op->inputs[i]).as<Var>()->shape.size(); ++j) {
                        oss << "[" << (op->inputs[i]).as<Var>()->shape[j] << "]";
                    }
                } 
                num1++;
                oss << ", ";
            }
        }
        for (size_t i = 0; i < op->outputs.size(); ++i) {
            bool flag = false;
            for (size_t j = 0; j < op->inputs.size(); ++j) {
                if ((op->outputs[i]).as<Var>()->name == (op->inputs[j]).as<Var>()->name) {
                    flag = true;
                    break;
                }
            }
            if (flag) {
                continue;
            }
            if (op->outputs[i].type().is_float()) {
                oss << "float";
            }
            if (op->outputs[i].type().is_int()) {
                oss << "int";
            }
            oss << " (&";
            oss << (op->outputs[i]).as<Var>()->name;
            oss << ") ";
            if (((op->outputs[i]).as<Var>()->shape[0]) == 1) {

            }
            else {
                for (size_t j = 0; j < (op->outputs[i]).as<Var>()->shape.size(); ++j) {
                    oss << "[" << (op->outputs[i]).as<Var>()->shape[j] << "]";
                }
            }
            if (i < op->outputs.size() - 1) {
                oss << ", ";
            }
        }
        if (num1 == op->inputs.size()) {

        }
        else {
            oss << ", ";
        }
        for (size_t i = 0; i < op->inputs.size(); ++i) {
            if (((op->inputs[i]).as<Var>()->name).at(0) == str.at(0)) {
                num2++;
                if (op->inputs[i].type().is_float()) {
                    oss << "float";
                }
                if (op->inputs[i].type().is_int()) {
                    oss << "int";
                }
                oss << " (&";
                oss << (op->inputs[i]).as<Var>()->name;
                oss << ")";
                if (((op->inputs[i]).as<Var>()->shape[0]) == 1) {

                }
                else {
                    for (size_t j = 0; j < (op->inputs[i]).as<Var>()->shape.size(); ++j) {
                        oss << "[" << (op->inputs[i]).as<Var>()->shape[j] << "]";
                    }
                }
                if(num2 < (op->inputs.size() - num1)){
                    oss << ", ";
                }
            }
        }
        print_arg = false;
        oss << ") {\n";
        enter();
        for (auto stmt : op->stmt_list) {
            stmt.visit_stmt(this);
        }
        exit();
        oss << "}\n";
    }
};
class MyMutator : public IRMutator {
 public:
    Expr src_change;
    Expr dst_change;
    std::string out;
    //std::vector<std::string> grad;
    std::string grad;
    int index_use;
    int mode; 
    bool if_remove;
    std::map<std::string, Expr> match;
    Ref<const Index> index_temp;
    Expr global_index;
    std::vector<Expr> new_indexs;
    MyMutator(std::string _out, std::string _grad): IRMutator() {
        mode = 0;
        out = _out;
        grad = _grad;
        index_use = 0;
        if_remove = false;
    }
    Expr visit(Ref<const Var> op) override {
        if (op->name == grad) {
            if (mode == 1) {
                std::vector<Expr> new_args;
                for (auto arg : op->args) {
                    mode = 3;
                    new_args.push_back(mutate(arg));
                }
                mode = 1;
                src_change = Var::make(op->type(), ("d" + grad), new_args, op->shape);
            }
            else if (mode == 2){
                return dst_change;
            }
            else {
                return Var::make(op->type(), ("d" + grad), op->args, op->shape);
            }
        }
        else if (op->name == out) {
            if (mode == 1) {
                dst_change = Var::make(op->type(), ("d" + out), op->args, op->shape);
            }
            else if (mode == 2){
                return src_change;
            }
            else {
                return Var::make(op->type(), ("d" + out), op->args, op->shape);
            }
        }
        return IRMutator::visit(op);
    }


    Expr visit(Ref<const Index> op) override {
        if (mode == 3) {
            index_temp = op;
        }
        else if (mode == 4) {
            match[op->name] = global_index;
        }
        else if (mode == 5) {
            if (match.find(op->name) != match.end()) {
                if_remove = true;
                return IRMutator::visit(op);
            }
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
        return IRMutator::visit(op);
    }

    Stmt visit(Ref<const Move> op) override {
        mode = 1;
        mutate(op->src);
        mutate(op->dst);
        mode = 2;
        Expr new_src = mutate(op->src);
        Expr new_dst = mutate(op->dst);
        mode = 0;
        return Move::make(new_dst, new_src, op->move_type);
        //return IRMutator::visit(op);
    }

    Stmt visit(Ref<const LoopNest> op) override {
        std::vector<Expr> new_index_list;
        std::vector<Stmt> new_body_list;
        for (auto body : op->body_list) {
            new_body_list.push_back(mutate(body));
        }
        for (auto index : op->index_list) {
            mode = 5;
            Expr expr = mutate(index);
            if (!if_remove) {
                new_index_list.push_back(expr);
            }
            else {
                if_remove = false;
            }
        }
        for (auto index : new_indexs) {
            new_index_list.push_back(index);
        }
        return LoopNest::make(new_index_list, new_body_list);
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
        MyMutator mutator(outs[0], grad_to[0]);
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
        std::string path = casePath + std::to_string(i) + ".json";
        std::string outpath = kernelPath + std::to_string(i) + ".cc";
        solution(i, path, outpath);
    }
    return 0;
}