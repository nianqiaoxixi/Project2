#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"
#include "parser.h"

using namespace Boost::Internal;

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
            else if (domain == "ins" || domain == "outs") {
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
                    else {
                        outs.push_back(temp.substr(0, index1));
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
        IRMutator mutator;
        main_code = mutator.mutate(main_code);

        // printer
        IRPrinter printer;
        std::string code = printer.print(main_code);

        std::ofstream ofile(outpath, std::ios::out);
        ofile << "#include \"../run.h\"\n";
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
    std::string kernelPath = "./kernels/kernel_case";
    for(int i = 1; i <= 10; i ++) {
        std::string path = casePath + std::to_string(i) + ".json";
        std::string outpath = kernelPath + std::to_string(i) + ".cc";
        solution(i, path, outpath);
        solution(-1, "./cases/example.json", "./kernels/kernel_example.cc");
    }
    return 0;
}