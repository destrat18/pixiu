#pragma once

#include <set>
#include <vector>
#include <string>
#include <map>

struct MyGraph {
    std::vector<std::set<int>> adg_set;
    std::set<int> alive_vertices;
    std::set<int> vertices_non_zero_degree;

    std::map<int, int> idx_to_fileidx;
    std::map<int, int> fileidx_to_idx;
    std::map<int, std::string> idx_to_hash;
    std::map<int, int> idx_to_cost;
    std::map<int, int> idx_to_volume;
    std::vector<std::tuple<int, int, char>> labeled_edges;
    int n_vert;

    int max_capacity = 0;

    MyGraph(const std::vector<std::pair<int, int>>& edges);
    MyGraph(const std::vector<std::pair<int, int>>& edges, int n_vert);

    bool is_connected (int from, int to) const;

    bool is_connected (int from, int to, char label) const;

    const std::set<int>& get_neighbours(int from) const;

    std::vector<std::pair<int, int>> edge_list() const;

    void delete_edge(int u, int v);

    void delete_vertex(int v);

    void set_idx_to_fileidx(const std::map<int, int>& idx_to_fileidx) {
        this->idx_to_fileidx = idx_to_fileidx;
    }
    void set_fileidx_to_idx(const std::map<int, int>& fileidx_to_idx) {
        this->fileidx_to_idx = fileidx_to_idx;
    }
    void set_idx_to_hash(const std::map<int, std::string>& idx_to_hash) {
        this->idx_to_hash = idx_to_hash;
    }
    void set_idx_to_cost(const std::map<int, int>& idx_to_cost) {
        this->idx_to_cost = idx_to_cost;
    }
    void set_idx_to_volume(const std::map<int, int>& idx_to_volume) {
        this->idx_to_volume = idx_to_volume;
    }
    void set_labeled_edges(const std::vector<std::tuple<int, int, char>>& labeled_edges) {
        this->labeled_edges = labeled_edges;
    }

    void print() {
        std::cout << "idx fidx hash cost volume" << std::endl;
        for (int i = 0; i < n_vert; i++) {
            std::cout << i << " " << idx_to_fileidx[i] << " " << idx_to_hash[i] << " " << idx_to_cost[i] << " " << idx_to_volume[i] << std::endl;
        }
        std::cout << "edges:" << std::endl; 
        for (auto it: labeled_edges) {
            std::cout << std::get<0>(it) << " " << std::get<1>(it) << " " << std::get<2>(it) << std::endl;
        }
    }

};


