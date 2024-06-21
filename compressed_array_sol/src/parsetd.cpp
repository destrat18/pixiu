#include "parsetd.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "tdlib_wrapper.hpp"
#include "comon_defs.hpp"



namespace wrap {


void dfs(int v, const std::vector<std::vector<int>>& tree_decomp_adgency_list, std::vector<char>& visited, std::vector<TreeNode>& ordering) {
    visited[v] = 1;
    std::vector<int> down_children;
    for (auto it: tree_decomp_adgency_list[v]) {
        if (!visited[it]) {
            down_children.push_back(it);
            dfs(it, tree_decomp_adgency_list, visited, ordering);
        }
    }
    ordering.push_back(TreeNode());
    ordering.back().v = v;
    ordering.back().children.resize(down_children.size());
    for (int i = 0; i < down_children.size(); i++) {
        ordering.back().children[i] = down_children[i];
    }

    
}

std::vector<TreeNode> convert(int number_of_bags, const std::vector<std::vector<int>>& tree_decomp_adgency_list, const std::vector<std::vector<int>>& bags) {
    
    assert(number_of_bags > 0);
    for (auto it: tree_decomp_adgency_list) {
        for (auto to: it) {
            assert(to >= 0);
            assert(to < number_of_bags);
        }
    }

    
    
    std::vector<char> visited(number_of_bags, 0);
    std::vector<TreeNode> ans;

    dfs(0, tree_decomp_adgency_list, visited, ans);

    for (int i = 0; i < number_of_bags; i++) {
        ans[i].bag = bags[ans[i].v];
    }

    return ans;

}

std::vector<TreeNode> parse_td(std::istream& fin) {
    std::string curr;    
    int number_of_bags, max_width, graph_vert_number;
    std::vector<std::vector<int>> tree_decomp_edge_list;
    std::vector<std::vector<int>> bags; 

    int bag_index = 0;
    while(std::getline(fin, curr)) {
        if (curr.size() > 0 && (curr[0] == 's' || curr[0] == 'b' || (curr[0] >= '0' && curr[0] <= '9'))) {
            std::vector<std::string> strs;
            boost::split(strs, curr, boost::is_any_of("\t "));
            if (curr[0] == 's' && curr[1] == ' ' && curr[2] == 't' && curr[3] == 'd' && curr[4] == ' ') {
                number_of_bags = std::atoi(strs[2].c_str());
                tree_decomp_edge_list.resize(number_of_bags);
                max_width = std::atoi(strs[3].c_str());
                graph_vert_number = std::atoi(strs[4].c_str());

                // std::cout << "here: " << number_of_bags << " " << max_width << " " << graph_vert_number << std::endl;
            } else if (curr[0] == 'b') {
                bags.push_back(std::vector<int>());
                for (int i = 1; i < strs.size(); i++) {
                    if (i == 1) {
                        assert(std::atoi(strs[i].c_str()) - 1 == bag_index++);
                    } else {
                        bags.back().push_back(std::atoi(strs[i].c_str()) - 1);
                    }
                }
            } else if (curr[0] >= '0' && curr[0] <= '9'){
                int l, r;
                l = std::atoi(strs[0].c_str()) - 1;
                r = std::atoi(strs[1].c_str()) - 1;
                tree_decomp_edge_list[l].push_back(r);
                tree_decomp_edge_list[r].push_back(l);

            }
        }
    }

    if (number_of_bags == 0) {
        return std::vector<TreeNode>();
    }
    return convert(number_of_bags, tree_decomp_edge_list, bags);
}


void dfs_tree_decomp(int v, 
const std::vector<std::vector<int>>& 
tree_decomp_adgency_list, std::vector<char>& visited, 
std::vector<int>& stack,
std::vector<TreeNode>& bags
) {
    visited[v] = 1;
    stack.push_back(v);

    bags.push_back(TreeNode());
    bags.back().idx = bags.size() - 1;
    bags.back().distinguished_vertex = v;
    bags.back().bag = stack;
    bags.back().node_type = NodeType::INTRODUCE;
    bags.back().children_idx.resize(1);
    bags.back().children_idx[0] = bags.size() - 2;



    for (auto it: tree_decomp_adgency_list[v]) {
        if (!visited[it]) {
            dfs_tree_decomp(it, tree_decomp_adgency_list, visited, stack, bags);
        }
    }

    stack.pop_back();

    bags.push_back(TreeNode());
    bags.back().idx = bags.size() - 1;
    bags.back().distinguished_vertex = v;
    bags.back().bag = stack;
    bags.back().node_type = NodeType::FORGET;
    bags.back().children_idx.resize(1);
    bags.back().children_idx[0] = bags.size() - 2;
    
}

std::vector<TreeNode> parse_tdp_to_nice(std::istream& fin) {
    int treedepth;
    fin >> treedepth;
    std::vector<int> parent;
    int v;

    while (fin >> v) {
        v--;
        parent.push_back(v);
    }   

    int n = parent.size();

    // std::cout << "n: " << n << std::endl;
    // std::cout << "parent: ";
    // for (auto it: parent) {
    //     std::cout << it << " ";
    // }
    // std::cout << std::endl;


    std::vector<std::vector<int>> tree(n);
    std::vector<int> ref_cnt(n, 0); // stores how many children has each vertex
    
    for (int i = 0; i < n; i++) {
        if (parent[i] != -1) {
            tree[i].push_back(parent[i]);
            tree[parent[i]].push_back(i);
            ref_cnt[parent[i]]++;
        }
    }

    std::vector<TreeNode> bags;
    bags.push_back(TreeNode()); // leaf bag
    bags[0].node_type = NodeType::LEAF;
    std::vector<int> dfs_stack;
    std::vector<char> visited(n, 0);

    for (int i = 0; i < n; i++) {
        if (parent[i] == -1) {
            dfs_tree_decomp(i, tree, visited, dfs_stack, bags);
        }
    }

    return bags;

}


}