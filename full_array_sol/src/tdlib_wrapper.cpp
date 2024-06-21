#include "tdlib_wrapper.hpp"

#include <vector>
#include <set>
#include <map>
#include <boost/graph/adjacency_list.hpp>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include "config.hpp"
#include "comon_defs.hpp"
#include "parsetd.hpp"
#include "flow-cutter/pace.h"
#include "my_graph.hpp"

namespace wrap {

// interacts with flow-cutter 
std::vector<TreeNode> decomposition3(const std::vector<std::pair<int, int>>& edge_list, const std::string& method, float decomposition_time) {
    std::set<int> unique_vertices;
    for (auto it: edge_list) {
        unique_vertices.insert(it.first);
        unique_vertices.insert(it.second);
    }

    int n = unique_vertices.size();

	std::stringstream graph_file;
	graph_file << "p tw " << n << " " << edge_list.size() << std::endl;

	for (auto it: edge_list) {
		graph_file << it.first + 1 << " " << it.second + 1 << std::endl;
	} 

	// std::cout << "graph_file:\n" << graph_file.str() << std::endl;

	// std::string result = tw(graph_file, method, decomposition_time);

	std::string result = tw(graph_file);

	std::stringstream tree_file(result);
	auto ans = wrap::parse_td(tree_file);
	return ans;
}


std::vector<TreeNode> decomposition3(const std::vector<std::pair<int, int>>& edge_list, const MyGraph& g, const std::string& method, float decomposition_time) {
    std::set<int> unique_vertices;
    for (auto it: edge_list) {
        unique_vertices.insert(it.first);
        unique_vertices.insert(it.second);
    }

    int n = unique_vertices.size();

	std::stringstream graph_file;
	graph_file << "p tw " << g.n_vert << " " << edge_list.size() << std::endl;

	for (auto it: edge_list) {
		graph_file << it.first + 1 << " " << it.second + 1 << std::endl;
	} 

	// std::cout << "graph_file:\n" << graph_file.str() << std::endl;

	// std::string result = tw(graph_file, method, decomposition_time);

	std::string result = tw(graph_file);

	std::stringstream tree_file(result);
	auto ans = wrap::parse_td(tree_file);
	return ans;
}
}
