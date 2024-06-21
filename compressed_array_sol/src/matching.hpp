#pragma once

// #define MATCHING_PRINT_DEBUG

#include <vector>
#include <numeric>
#include <algorithm>

#include "my_graph.hpp"
#include "comon_defs.hpp"

using namespace wrap;




template <class Num>
inline Num count_matchings(const MyGraph& graph, const std::vector<wrap::TreeNode>& treedecomp) {

    int mem_off_bits[40] = {};

    int max_bag_size = 0;
    std::vector<std::vector<Num>> dpt;
    
    for (auto node: treedecomp) {
        dpt.push_back(std::vector<Num>());
        dpt.back().resize(((long long)1) << node.bag.size());
        if (node.bag.size() >= MAX_BAG_LIMIT) {
            throw std::logic_error("treewidth is too large");
        }
    }



    for (int i = 0; i < treedecomp.size(); i++) {

        const TreeNode& node = treedecomp[i];
        switch (node.node_type) {
            case wrap::LEAF: {
                dpt[i][0] = 1;
                break;
            }
            case wrap::INTRODUCE: {
                int child_idx = node.children_idx[0];
                int introduced_vertex = node.distinguished_vertex;
                int introduced_vertex_idx = -1;
                for (int j = 0; j < node.bag.size(); j++) {
                    if (node.bag[j] == introduced_vertex) {
                        introduced_vertex_idx = j;
                        break;
                    }
                }
                assert(introduced_vertex_idx >= 0);

                for (unsigned long long mask = 0; mask < ((unsigned long long)1 << node.bag.size()); mask++) {
                    if (mask & bit_extractor_ull(introduced_vertex_idx)) {
                        dpt[i][mask] = dpt[child_idx][cut_bit(introduced_vertex_idx, mask)];
                    } else {
                        dpt[i][mask] = 0;
                    }
                }


                break;
            }
            case wrap::FORGET: {
                int child_idx = node.children_idx[0];
                int forgetted_vertex = node.distinguished_vertex;
                int forgetted_vertex_idx = -1;
                for (int j = 0; j < treedecomp[child_idx].bag.size(); j++) {
                    if (treedecomp[child_idx].bag[j] == forgetted_vertex) {
                        forgetted_vertex_idx = j;
                    }
                }
                assert(forgetted_vertex_idx >= 0);

                for (unsigned long long mask = 0; mask < ((unsigned long long)1 << node.bag.size()); mask++) {
                    dpt[i][mask] = 0;
                    dpt[i][mask] += dpt[child_idx][insert_bit(forgetted_vertex_idx, 0, mask)];
                    unsigned long long mask_with_v = insert_bit(forgetted_vertex_idx, 1, mask);
                    dpt[i][mask] += dpt[child_idx][mask_with_v];

#ifdef MATCHING_PRINT_DEBUG
                    std::cout << "Match[c, M] = " << dpt[child_idx][insert_bit(forgetted_vertex_idx, 0, mask)] 
                              << " Match[c, M+v] = " << dpt[child_idx][mask_with_v];
#endif

                    for (int u_idx = 0; u_idx < treedecomp[child_idx].bag.size(); u_idx++) {
                        if (!(bit_extractor_ull(u_idx) & mask_with_v) // u does not belong to M
                        && u_idx != forgetted_vertex_idx
                        && graph.is_connected(treedecomp[child_idx].bag[u_idx], forgetted_vertex)) {
                            dpt[i][mask] += dpt[child_idx][mask_with_v | bit_extractor_ull(u_idx)];
#ifdef MATCHING_PRINT_DEBUG
                        std::cout << " Match[c, M+v+"<<treedecomp[child_idx].bag[u_idx]<<"] = " << dpt[child_idx][mask_with_v | bit_extractor_ull(u_idx)];  
#endif
                        }
                    }
#ifdef MATCHING_PRINT_DEBUG
                        std::cout << std::endl;
#endif
                }

                break; 
            }
            case wrap::MERGE: {
                int l_child_idx = node.children_idx[0];
                int r_child_idx = node.children_idx[1];
                

                for (unsigned long long mask = 0; mask < ((unsigned long long)1 << node.bag.size()); mask++) {
                    int off_bit_cnt = 0;
                    for (int bit_idx = 0; bit_idx < node.bag.size(); bit_idx++) {
                        if (!(mask & bit_extractor_ull(bit_idx))) {
                            mem_off_bits[off_bit_cnt++] = bit_idx;
                        }
                    }
#ifdef MATCHING_PRINT_DEBUG
                        std::cout << "MERGE case, mask = " << mask << std::endl;
#endif
                    for (unsigned long long h1_mask = 0; h1_mask < ((unsigned long long)1 << off_bit_cnt); h1_mask++) {
                        unsigned long long mask_cup_h1 = mask;
                        unsigned long long mask_cup_h2 = mask;
                        for (int h1_idx = 0; h1_idx < off_bit_cnt; h1_idx++) {
                            if (bit_extractor_ull(h1_idx) & h1_mask) {
                                mask_cup_h1 |= bit_extractor_ull(mem_off_bits[h1_idx]);
                            } else {
                                mask_cup_h2 |= bit_extractor_ull(mem_off_bits[h1_idx]);
                            }
                        }
                        dpt[i][mask] += dpt[l_child_idx][mask_cup_h1] * dpt[r_child_idx][mask_cup_h2];
#ifdef MATCHING_PRINT_DEBUG
                        std::cout << "Added: " << "dpt[" << l_child_idx << "][" << mask_cup_h1 << "] * dpt[" << r_child_idx << "][" << mask_cup_h2 << "]" << " = " << dpt[l_child_idx][mask_cup_h1] * dpt[r_child_idx][mask_cup_h2] << std::endl;
#endif
                    }
                }
                break;
            } 
            default:
                assert(false);
        }
    }


#ifdef MATCHING_PRINT_DEBUG
    std::cout << "DPT:\n";
    for (int i = 0; i < dpt.size(); i++) {
        std::cout << "-----------------\n" << treedecomp[i];
        for (auto it: dpt[i]) std::cout << it << " ";
        std::cout << "\n-----------------" << std::endl;
    }
#endif

    return dpt.back()[0];
}


template <class Num> 
inline Num count_matchings_brute_force(const MyGraph& graph) {

    int n = graph.n_vert;

    if (n > 10) {
        throw std::logic_error("too large to bruteforce");
    }

    std::vector<int> all_vert(n);
    std::iota(all_vert.begin(), all_vert.end(), 0);
    std::vector<std::vector<int>> all_perm;
    do {
        all_perm.push_back(all_vert);
    } while(std::next_permutation(all_vert.begin(), all_vert.end()));

    auto fac = [](int x) -> long long {
        if (x == 0) {
            return 1;
        }
        long long ans = 1;
        for (long long i = 1; i <= x; i++) ans *= i;
        return ans;
    };

    Num ans = 1;
    for (int matching_size = 2; matching_size <= n; matching_size += 2) {
        Num to_add = 0;
        for (auto perm: all_perm) {
            bool is_matching = true;
            for (int tmp = 0; tmp < matching_size; tmp += 2) {
                if (!graph.is_connected(perm[tmp], perm[tmp + 1])) {
                    is_matching = false;
                    break;
                }
            }
            if (is_matching) to_add++;
        }
        
        assert(to_add % fac(n - matching_size) == 0);
        ans += to_add / fac(n - matching_size) / fac(matching_size / 2) / ((unsigned long long)1 << (matching_size / 2));

    }
    return ans;

}


template <class Num> 
inline Num count_matchings_branching(MyGraph graph) {
    if (graph.vertices_non_zero_degree.empty()) {
        return 1;
    }

    auto g1 = graph;
    auto g2 = graph;

    // (u, v) edge to branch
    auto u = *graph.vertices_non_zero_degree.begin();
    auto v = *graph.adg_set[u].begin();

    g1.delete_edge(u, v);
    Num uv_not_in_matching = count_matchings_branching<Num>(g1);

    g2.delete_vertex(u);
    g2.delete_vertex(v);
    Num uv_in_matching = count_matchings_branching<Num>(g2);

    return uv_not_in_matching + uv_in_matching;
}