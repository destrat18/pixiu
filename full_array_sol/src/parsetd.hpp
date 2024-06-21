#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "tdlib_wrapper.hpp"
#include "comon_defs.hpp"

namespace wrap {
    std::vector<TreeNode> parse_td(std::istream& fin);
    std::vector<TreeNode> parse_tdp_to_nice(std::istream& fin);
}