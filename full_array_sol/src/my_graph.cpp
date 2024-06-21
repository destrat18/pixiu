#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <algorithm>
#include <cassert>
#include "my_graph.hpp"



MyGraph::MyGraph(const std::vector<std::pair<int, int>>& edges) {
        n_vert = 0;
        for (auto edge: edges) {
            n_vert = std::max(n_vert, std::max(edge.first, edge.second) + 1);
        }
        adg_set = std::vector<std::set<int>>(n_vert);

        for (auto edge: edges) {
            adg_set[edge.first].insert(edge.second);
            adg_set[edge.second].insert(edge.first);        
        }

        for (int i = 0; i < n_vert; i++) {
            alive_vertices.insert(i);
            if (!adg_set[i].empty()) {
                vertices_non_zero_degree.insert(i);
            }
        }
}

MyGraph::MyGraph(const std::vector<std::pair<int, int>>& edges, int n_vert) {
        this->n_vert = n_vert;
        adg_set = std::vector<std::set<int>>(n_vert);

        for (auto edge: edges) {
            adg_set[edge.first].insert(edge.second);
            adg_set[edge.second].insert(edge.first);        
        }

        for (int i = 0; i < n_vert; i++) {
            alive_vertices.insert(i);
            if (!adg_set[i].empty()) {
                vertices_non_zero_degree.insert(i);
            }
        }
}


bool MyGraph::is_connected (int from, int to) const {
    return (adg_set[from].find(to) != adg_set[from].end());
}

bool MyGraph::is_connected (int from, int to, char label) const {
    // if label is 'c' then we check if either (from, to, label) or (to, from, label) is in the graph 
    // if label is 'd' then we check if (from, to, label) is in the graph
    // otherwise we throw an exception
    
    // TODO: optimize if needed
    if (label == 'c') {
        for (auto l_edge: labeled_edges) {
            if ((std::get<0>(l_edge) == from && std::get<1>(l_edge) == to && std::get<2>(l_edge) == label) || (std::get<0>(l_edge) == to && std::get<1>(l_edge) == from && std::get<2>(l_edge) == label)) {
                return true;
            }
        }
        return false;
    } else if (label == 'd') {
        for (auto l_edge: labeled_edges) {
            if (std::get<0>(l_edge) == from && std::get<1>(l_edge) == to && std::get<2>(l_edge) == label) {
                return true;
            }
        }
        return false;
    } else {
        throw std::logic_error("label must be either 'c' or 'd'");
    }

}


const std::set<int>& MyGraph::get_neighbours(int from) const {
    return adg_set[from];
}


std::vector<std::pair<int, int>> MyGraph::edge_list() const {
    std::vector<std::pair<int, int>> edges;

    for (int i = 0; i < n_vert; i++) {
        for (auto it: adg_set[i]) {
            if (i < it)
                edges.push_back({i, it});
        }
    }
    return edges;
}

void MyGraph::delete_edge(int u, int v) {
    adg_set[u].erase(v);
    adg_set[v].erase(u);

    if (adg_set[u].empty()) {
        vertices_non_zero_degree.erase(u);
    }

    if (adg_set[v].empty()) {
        vertices_non_zero_degree.erase(v);
    }
}

void MyGraph::delete_vertex(int u) {
    for (auto v: adg_set[u]) {
        adg_set[v].erase(u);
        if (adg_set[v].empty()) {
            vertices_non_zero_degree.erase(v);
        }
    }
    adg_set[u].clear();
    alive_vertices.erase(u);
    if (vertices_non_zero_degree.find(u) != vertices_non_zero_degree.end()) {
        vertices_non_zero_degree.erase(u);
    }

}