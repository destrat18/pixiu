#pragma once

#include <vector>
#include <set>
#include <boost/graph/adjacency_list.hpp>
#include <cstdlib>
#include <filesystem>

#include "comon_defs.hpp"
#include "parsetd.hpp"
#include "my_graph.hpp"

namespace wrap {

std::vector<TreeNode> decomposition(const std::filesystem::path& path_to_gr, float decomposition_time=0.02);
std::vector<TreeNode> decomposition(const std::vector<std::pair<int, int>>& edge_list, const std::string& method = "CUTSET", float decomposition_time=0.02);
std::vector<TreeNode> decomposition2(const std::vector<std::pair<int, int>>& edge_list, const std::string& method, float decomposition_time);
std::vector<TreeNode> decomposition3(const std::vector<std::pair<int, int>>& edge_list, const std::string& method, float decomposition_time);
std::vector<TreeNode> decomposition3(const std::vector<std::pair<int, int>>& edge_list, const MyGraph& g, const std::string& method, float decomposition_time);

// T exact_decomposition(const std::vector<std::pair<int, int>>& adgency_list, const std::string& method = "CUTSET");
std::ostream& operator<<(std::ostream& stream, const TreeNode& node);
// std::vector<TreeNode> getBottomUpOrder(T tree);

}
