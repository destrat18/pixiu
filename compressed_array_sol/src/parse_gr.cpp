#include <iostream>
#include <filesystem>
#include <iomanip>
#include <string>
#include <set>
#include <sstream>
#include <map>
#include <vector>


// #define PARSE_GR_PRINT_DEBUG


#include "parse_gr.hpp"
#include "my_graph.hpp"

struct CWVertex {
    int ind;
    int cost;
    int weight;
};

MyGraph parse_gr(std::istream& in) {
        

    std::vector<std::pair<int, int>> edges;

    std::set<int> have_red;
    std::map<int, int> remap;
    int free_label = 0;

    std::string current_string;
    int n = -1, m = -1;
    while (std::getline(in, current_string)) {

#ifdef PARSE_GR_PRINT_DEBUG
        std::cout << "current string:\n" << current_string << std::endl;
#endif
        if (current_string.size() <= 1) {
            continue;
        }
        if (current_string.size() >= 5 && current_string[0] == 'p' && current_string[1] == ' ' && current_string[2] == 't' && current_string[3] == 'w') {
            std::stringstream read_n_m(current_string);
            std::string tmp1;
            read_n_m >> tmp1 >> tmp1 >> n >> m;
            continue;
        }
        if (current_string[0] >= '0' && current_string[0] <= '9') {
            int l, r;
            std::stringstream read_l_r(current_string);
            read_l_r >> l >> r;

            if (have_red.find(l) == have_red.end()) {
                have_red.insert(l);
                remap[l] = free_label;
                free_label++;
            }

            if (have_red.find(r) == have_red.end()) {
                have_red.insert(r);
                remap[r] = free_label;
                free_label++;
            }
            edges.push_back({remap[l], remap[r]});

        }
    }

#ifdef PARSE_GR_PRINT_DEBUG
    std::cout << "edge list:" << std::endl;
    for (auto it: edges) {
        std::cout << it.first << " " << it.second << std::endl;
    }
#endif

    return MyGraph(edges, n);

}


MyGraph parse_kgr(std::istream& in) {
        

    std::vector<std::pair<int, int>> edges;
    std::vector<std::tuple<int, int, char>> labeled_edges;

    std::set<int> have_red;
    std::map<int, int> remap; // fileidx -> idx
    std::map<int, int> idx_to_file_idx;
    std::map<int, std::string> idx_to_hash;
    std::map<int, int> idx_to_cost;
    std::map<int, int> idx_to_volume;

    std::set<int> unused_file_idxs;

    int free_label = 0;

    std::string current_string;
    int n = -1, m = -1;

    in >> n >> m;

    for (int i = 1; i <= n; i++) {
        unused_file_idxs.insert(i);
    }

    std::vector<int> costs_one_based(n);
    std::vector<int> weights_one_based(n);
    for (int i = 0; i < n; i++) {
        in >> costs_one_based[i];
    }
    for (int i = 0; i < n; i++) {
        in >> weights_one_based[i];
    }
    for (int i = 0; i < m; i++) {
        int l, r;
        char type;
        in >> l >> r >> type;
        if (l == r) {
            throw 1;
        }

        unused_file_idxs.erase(l);
        unused_file_idxs.erase(r);

        if (have_red.find(l) == have_red.end()) {
            have_red.insert(l);
            remap[l] = free_label;
            idx_to_file_idx[free_label] = l;
            free_label++;
        }

        if (have_red.find(r) == have_red.end()) {
            have_red.insert(r);
            remap[r] = free_label;
            idx_to_file_idx[free_label] = r;
            free_label++;
        }



        edges.push_back({remap[l], remap[r]});
        labeled_edges.push_back({remap[l], remap[r], type});
    }

    // take care of unused vertices
    for (auto it: unused_file_idxs) {
        remap[it] = free_label;
        idx_to_file_idx[free_label] = it;
        free_label++;
    }

    // read hashes
    for (int i = 1; i <= n; i++) {
        std::string hash;
        in >> hash;
        idx_to_hash[remap[i]] = hash;
    }

    // assign costs
    for (int i = 1; i <= n; i++) {
        idx_to_cost[remap[i]] = costs_one_based[i - 1];
    }

    // assign volumes
    for (int i = 1; i <= n; i++) {
        idx_to_volume[remap[i]] = weights_one_based[i - 1];
    }

    auto g = MyGraph(edges, n);
    g.set_idx_to_fileidx(idx_to_file_idx);
    g.set_fileidx_to_idx(remap);
    g.set_idx_to_hash(idx_to_hash);
    g.set_idx_to_cost(idx_to_cost);
    g.set_idx_to_volume(idx_to_volume);
    g.set_labeled_edges(labeled_edges);
    

    int max_capacity=90112;
    if (in >> max_capacity) {
        // std::cout << "max capacity: " << max_capacity << std::endl;
    } else {
        // std::cout << "max capacity not specified, using default: " << max_capacity << std::endl;
    }

    g.max_capacity = max_capacity;


    return g; 

}



MyGraph parse_kgr2(std::istream& in) { // no relabeling
        

    std::vector<std::pair<int, int>> edges;
    std::vector<std::tuple<int, int, char>> labeled_edges;

    std::set<int> have_red;
    std::map<int, std::string> idx_to_hash;
    std::map<int, int> idx_to_cost;
    std::map<int, int> idx_to_volume;

    std::set<int> unused_file_idxs;


    std::string current_string;
    int n = -1, m = -1;

    in >> n >> m;

    for (int i = 0; i < n; i++) {
        unused_file_idxs.insert(i);
    }

    std::vector<int> costs(n);
    std::vector<int> weights(n);
    for (int i = 0; i < n; i++) {
        in >> costs[i];
    }
    for (int i = 0; i < n; i++) {
        in >> weights[i];
    }
    for (int i = 0; i < m; i++) {
        int l, r;
        char type;
        in >> l >> r >> type;
        if (l == r) {
            throw 1;
        }
        l--;
        r--;

        unused_file_idxs.erase(l);
        unused_file_idxs.erase(r);

        if (have_red.find(l) == have_red.end()) {
            have_red.insert(l);
        }

        if (have_red.find(r) == have_red.end()) {
            have_red.insert(r);
        }



        edges.push_back({l, r});
        labeled_edges.push_back({l, r, type});
    }

    // // take care of unused vertices
    // for (auto it: unused_file_idxs) {
    //     remap[it] = free_label;
    //     idx_to_file_idx[free_label] = it;
    //     free_label++;
    // }

    // read hashes
    for (int i = 0; i < n; i++) {
        std::string hash;
        in >> hash;
        idx_to_hash[i] = hash;
    }

    // assign costs
    for (int i = 0; i < n; i++) {
        idx_to_cost[i] = costs[i];
    }

    // assign volumes
    for (int i = 0; i < n; i++) {
        idx_to_volume[i] = weights[i];
    }

    std::map<int, int> idx_to_file_idx;
    for (int i = 0; i < n; i++) {
        idx_to_file_idx[i] = i;
    }

    std::map<int, int> remap;
    for (int i = 0; i < n; i++) {
        remap[i] = i;
    }

    auto g = MyGraph(edges, n);
    g.set_idx_to_fileidx(idx_to_file_idx);
    g.set_fileidx_to_idx(remap);
    g.set_idx_to_hash(idx_to_hash);
    g.set_idx_to_cost(idx_to_cost);
    g.set_idx_to_volume(idx_to_volume);
    g.set_labeled_edges(labeled_edges);
    

    int max_capacity=90112;
    if (in >> max_capacity) {
        // std::cout << "max capacity: " << max_capacity << std::endl;
    } else {
        // std::cout << "max capacity not specified, using default: " << max_capacity << std::endl;
    }

    g.max_capacity = max_capacity;

    

    return g; 

}