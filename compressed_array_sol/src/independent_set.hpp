#pragma once

// #define IND_SET_PRINT_DEBUG

#include <vector>

#include "my_graph.hpp"
#include "comon_defs.hpp"

using namespace wrap;


#define PRINT_BAGS


template <class Num>
inline Num count_independet_set(const MyGraph& graph, const std::vector<wrap::TreeNode>& treedecomp) {
    // int cnt = 0;
    // for (auto it: treedecomp) {
    //     std::cout << cnt++ << std::endl;
    //     std::cout << it << std::endl;
    // }


    #ifdef PRINT_BAGS
    std::cout << "bags:\n";

    for (int i = 0; i < treedecomp.size(); i++) {
        std::cout << "i = " << i << " " << "idx = " << treedecomp[i].idx << 
        " children_idx = [";
        for (auto it: treedecomp[i].children_idx) {
            std::cout << it << " ";
        }
        std::cout << "] " << "bag = [";
        for (auto it: treedecomp[i].bag) {
            std::cout << it << " ";
        }
        std::cout << "] ";
        std::cout << "dist_vertex = " << treedecomp[i].distinguished_vertex << " ";
        switch (treedecomp[i].node_type)
        {
        case wrap::INTRODUCE:
            std::cout << "INTRODUCE";
            break;
        case wrap::FORGET:
            std::cout << "FORGET";
            break;
        case wrap::MERGE:
            std::cout << "MERGE";
            break;
        case wrap::LEAF:
            std::cout << "LEAF";
            break;
        default:
            assert(false);
        }
        std::cout << std::endl;


    }

    // for (auto it: treedecomp) {
    //     for (auto it2: it.bag) {
    //         std::cout << it2 << " ";
    //     }
    //     std::cout << std::endl;
    // }
    #endif


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
        // std::cout << "step: " << i << std::endl;

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

                    bool introduced_vertex_has_neighbour_in_M = false;
                    for (int bag_idx = 0; bag_idx < node.bag.size(); bag_idx++) {
                        if (bag_idx == introduced_vertex_idx) {continue;}
                        if (bit_extractor_ull(bag_idx) & mask) {
                            if (graph.is_connected(node.bag[bag_idx], node.bag[introduced_vertex_idx])) {
                                introduced_vertex_has_neighbour_in_M = true;
                                break;
                            }
                        }
                    }

                    if (!(bit_extractor_ull(introduced_vertex_idx) & mask)) {
#ifdef IND_SET_PRINT_DEBUG
    std::cout << "I ";
#endif
                        dpt[i][mask] = dpt[child_idx][cut_bit(introduced_vertex_idx, mask)];
                    } else {
                        if (!introduced_vertex_has_neighbour_in_M) {
#ifdef IND_SET_PRINT_DEBUG
    std::cout << "II ";
#endif
                            dpt[i][mask] = dpt[child_idx][cut_bit(introduced_vertex_idx, mask)];
                        } else {
#ifdef IND_SET_PRINT_DEBUG
    std::cout << "III ";
#endif
                            dpt[i][mask] = 0;
                        }
                        // dpt[i][mask] = dpf
                    }
                }
#ifdef IND_SET_PRINT_DEBUG
    std::cout << "\n";
#endif


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
                    dpt[i][mask] = dpt[child_idx][insert_bit(forgetted_vertex_idx, 0, mask)] +
                        dpt[child_idx][insert_bit(forgetted_vertex_idx, 1, mask)];
                }
                break; 
            }
            case wrap::MERGE: {
                int l_child_idx = node.children_idx[0];
                int r_child_idx = node.children_idx[1];
                for (unsigned long long j = 0; j < dpt[i].size(); j++) {
                    dpt[i][j] = dpt[l_child_idx][j] * dpt[r_child_idx][j];
                }
                break;
            } 
            default:
                assert(false);
        }
    }

#ifdef IND_SET_PRINT_DEBUG
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
inline Num count_independet_set_brute_force(const MyGraph& graph) {
    const int MAX_VERT = 1000;
    int num_vert = graph.n_vert;

    if (num_vert > MAX_VERT) {
        throw std::logic_error("unfeasible");
    }
    Num ans = 0;
    for (unsigned long long mask = 0; mask < (((unsigned long long)1) << num_vert); mask++) {
        std::vector<int> in_mask;
        for (int i = 0; i < MAX_VERT; i++) {
            if (bit_extractor_ull(i) & mask) {
                in_mask.push_back(i);
            }
        }
        // std::cout << "current set:\n";
        // for (auto it: in_mask) {
        //     std::cout << it << " ";
        // }
        // std::cout << std::endl;

        bool has_edge = false;
        for (int l = 0; l < in_mask.size(); l++) {
            for (int r = l + 1; r < in_mask.size(); r++) {
                if (graph.is_connected(in_mask[l], in_mask[r])) {
                    has_edge = true;
                    break;
                }
            }
        }
        if (!has_edge) {
            ans += 1;
        }
    }

    return ans;

}



template <class Num> 
inline Num count_independet_set_branching(MyGraph graph) {
    
    if (graph.alive_vertices.empty()) {
        return 1;
    }

    int v = *graph.alive_vertices.begin();

    for (int i = 0; i < graph.adg_set.size(); i++) {
        if (graph.adg_set[i].size() > graph.adg_set[v].size()) {
            v = i;
        }
    }

    MyGraph g1 = graph;
    MyGraph g2 = graph;

    g1.delete_vertex(v);
    Num count_ind_sets_without_v = count_independet_set_branching<Num>(g1);

    auto v_neighborhood = g2.get_neighbours(v);

    for (auto neig: v_neighborhood) {
        g2.delete_vertex(neig);
    }
    g2.delete_vertex(v);

    Num count_ind_sets_with_v = count_independet_set_branching<Num>(g2);

    return count_ind_sets_with_v + count_ind_sets_without_v;
}