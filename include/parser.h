#include <iostream>
#include <fstream>
#include <string>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"

using namespace Boost::Internal;
Group parser(std::string str, Type index_type, Type data_type, 
	std::vector<std::string> in_str, std::vector<std::string> out_str, std::string kernel_name);