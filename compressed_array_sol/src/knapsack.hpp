#pragma once

// #define IND_SET_PRINT_DEBUG

#include <vector>

#include "my_graph.hpp"
#include "comon_defs.hpp"
#include "compressedarray.hpp"

using namespace wrap;


// #define PRINT_BAGS
// #define PRINT_DPT
#undef PRINT_BAGS

struct DptInfo {
    int opt_cost;
    int best_split;
    int best_split_l = 0;
    int best_split_r = 0;
    // unsigned long long opt_mask;
    char need_take = 0;
    bool valid = true;

    DptInfo(): opt_cost(0), best_split(0), need_take(0), valid(true) {}
    // DptInfo(int opt_cost, int best_split, int need_take, bool valid): opt_cost(opt_cost), best_split(best_split), need_take(need_take), valid(valid) {}

    void make_invalid() {
        valid = false;
    }

    bool operator<(const DptInfo& other) const {
        // invalid < valid 
        if (!valid && other.valid) {
            return true;
        }
        return opt_cost < other.opt_cost;
    }

    bool operator>(const DptInfo& other) const {
        // use < to implement >
        return other < *this;
    }

    bool operator==(const DptInfo& other) const {
        // invalid == invalid
        if (!valid && !other.valid) {
            return true;
        }
        return !(opt_cost < other.opt_cost) && !(other.opt_cost < opt_cost);
    }

    void print() {
        std::cout << "opt_cost = " << opt_cost << " best_split = " << best_split << " need_take = " << (int)need_take << " valid = " << valid << std::endl;
    }

};

inline DptInfo get_invalid_dpt_info() {
    DptInfo ans;
    ans.make_invalid();
    return ans;
}

inline std::set<int> get_subtree_bags(const std::vector<wrap::TreeNode>& treedecomp, int node_idx) {
    std::set<int> ans;
    for (auto it: treedecomp[node_idx].bag) {
        ans.insert(it);
    }
    for (auto it: treedecomp[node_idx].children_idx) {
        auto subtree_bags = get_subtree_bags(treedecomp, it);
        ans.insert(subtree_bags.begin(), subtree_bags.end());
    }
    return ans;
}


inline std::pair<std::vector<int>, int> solve_knapsack(const MyGraph& graph, const std::vector<wrap::TreeNode>& treedecomp) {
    int INF = 100000000;
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

        std::cout << "subtree_bags = [";
        auto subtree_bags = get_subtree_bags(treedecomp, i);
        for (auto it: subtree_bags) {
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
    int max_volume = graph.max_capacity + 1;


    // for each node for each capacity from 0 to max_volume we compute the maximum cost

    std::vector<std::vector<CompressedArray<DptInfo>>> dpt;
    
    for (auto node: treedecomp) {
        std::vector<CompressedArray<DptInfo>> dpt_node;

        dpt_node.resize(((long long)1) << node.bag.size());
        // std::cout << "node.bag.size() = " << node.bag.size() << std::endl;
        // std::cout << "allocating " << (((long long)1) << node.bag.size()) * max_volume << " space:" << std::endl;
        for (int i = 0; i < ((long long)1) << node.bag.size(); i++) {
            dpt_node[i] = CompressedArray<DptInfo>(max_volume); //.resize(max_volume);
        }
        dpt.push_back(dpt_node);

        // if ((node.bag.size()) >= MAX_BAG_LIMIT) {
        //     throw std::logic_error("treewidth is too large");
        // }
    }

    // std::cout << "allocation is done" << std::endl;

    // now we fill dpt[node][mask][capacity]

    DptInfo invalid_dpt_info = get_invalid_dpt_info();

    for (int i = 0; i < treedecomp.size(); i++) {
        // std::cout << "step: " << i << std::endl;

        const TreeNode& node = treedecomp[i];
        switch (node.node_type) {
            case wrap::LEAF: {
                // std::cout << "In LEAF" << std::endl;
                // dpt[i][0] = 1;
                // set all capacities to 0
                for (int j = 0; j < ((long long)1 << node.bag.size()); j++) {
                    dpt[i][j].set(0, DptInfo());
                }
                break;
            }
            case wrap::INTRODUCE: {
                // std::cout << "In INTRODUCE" << std::endl;
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
                
                #pragma omp parallel for
                for (unsigned long long mask = 0; mask < ((unsigned long long)1 << node.bag.size()); mask++) {
                    
                    // std::cout << "Current MASK = " << mask << "/" << ((unsigned long long)1 << node.bag.size()) << std::endl;
                    
                    unsigned long long mask_complement = ((unsigned long long)1 << node.bag.size()) - 1 - mask;

                    // std::cout << "block1" << std::endl;


                    bool introduced_vertex_has_dependency_in_M = false;
                    for (int bag_idx = 0; bag_idx < node.bag.size(); bag_idx++) {
                        if (bag_idx == introduced_vertex_idx) {continue;}
                        if (bit_extractor_ull(bag_idx) & mask) {
                            if (graph.is_connected(node.bag[introduced_vertex_idx], node.bag[bag_idx], 'd')) {
                                introduced_vertex_has_dependency_in_M = true;
                                break;
                            }
                        }
                    }

                    // std::cout << "block2" << std::endl;

                    bool introduced_vertex_has_dependency_in_M_complement = false;
                    for (int bag_idx = 0; bag_idx < node.bag.size(); bag_idx++) {
                        if (bag_idx == introduced_vertex_idx) {continue;}
                        if (bit_extractor_ull(bag_idx) & mask_complement) {
                            if (graph.is_connected(node.bag[introduced_vertex_idx], node.bag[bag_idx], 'd')) {
                                introduced_vertex_has_dependency_in_M_complement = true;
                                break;
                            }
                        }
                    }

                    // std::cout << "block3" << std::endl;

                    bool M_depends_on_introduced_vertex = false;
                    for (int bag_idx = 0; bag_idx < node.bag.size(); bag_idx++) {
                        if (bag_idx == introduced_vertex_idx) {continue;}
                        if (bit_extractor_ull(bag_idx) & mask) {
                            if (graph.is_connected(node.bag[bag_idx], node.bag[introduced_vertex_idx], 'd')) {
                                M_depends_on_introduced_vertex = true;
                                break;
                            }
                        }
                    }

                    // std::cout << "block4" << std::endl;


                    bool M_complement_depends_on_introduced_vertex = false;
                    for (int bag_idx = 0; bag_idx < node.bag.size(); bag_idx++) {
                        if (bag_idx == introduced_vertex_idx) {continue;}
                        if (bit_extractor_ull(bag_idx) & mask_complement) {
                            if (graph.is_connected(node.bag[bag_idx], node.bag[introduced_vertex_idx], 'd')) {
                                M_complement_depends_on_introduced_vertex = true;
                                break;
                            }
                        }
                    }


                    // std::cout << "block5" << std::endl;

                    bool introduced_vertex_has_conflict_in_M = false;
                    for (int bag_idx = 0; bag_idx < node.bag.size(); bag_idx++) {
                        if (bag_idx == introduced_vertex_idx) {continue;}
                        if (bit_extractor_ull(bag_idx) & mask) {
                            if (graph.is_connected(node.bag[introduced_vertex_idx], node.bag[bag_idx], 'c')) {
                                introduced_vertex_has_conflict_in_M = true;
                                break;
                            }
                        }
                    }

                    // std::cout << "block6" << std::endl;

                    int mask_volume = 0;

                    for (int bag_idx = 0; bag_idx < node.bag.size(); bag_idx++) {
                        if (bit_extractor_ull(bag_idx) & mask) {
                            mask_volume += graph.idx_to_volume.at(node.bag[bag_idx]);
                        }
                    }


                    for (int capacity = 0; capacity < max_volume; capacity++) {
                        if (capacity < mask_volume) {
                            // make invalid dptinfo 
                            dpt[i][mask].set(capacity, invalid_dpt_info);
                            // dpt[i][mask][capacity].make_invalid();
                            continue;
                        }
                        // std::cout << "Current CAPACITY: " << capacity << "/" << max_volume << std::endl;
                        if (!(bit_extractor_ull(introduced_vertex_idx) & mask)) { // introduced vertex not in the chosen set 
                            if (M_depends_on_introduced_vertex) {
                                
                                // std::cout << "ina" << std::endl;
                                dpt[i][mask].set(capacity, invalid_dpt_info);
                                // dpt[i][mask][capacity].make_invalid();
                                // std::cout << "outa" << std::endl;

                                continue;
                            }

                            // std::cout << "inb" << std::endl;
                            unsigned long long mask_without_introduced_vertex = cut_bit(introduced_vertex_idx, mask);
                            
                            // check if the mask is valid
                            // LASTFIX
                            auto tpd_info_to_check = dpt[child_idx][mask_without_introduced_vertex].get(capacity);
                            if (!tpd_info_to_check.valid) {
                                // dpt[i][mask][capacity].make_invalid();
                                dpt[i][mask].set(capacity, invalid_dpt_info);
                            } else {
                                DptInfo to_insert = DptInfo();
                                to_insert.opt_cost = tpd_info_to_check.opt_cost;
                                dpt[i][mask].set(capacity, to_insert);                                
                            }

                            // std::cout << "outb" << std::endl;

                        } else { // introduced vertex in the chosen set
                            if (introduced_vertex_has_dependency_in_M_complement) {

                                // std::cout << "inc" << std::endl;
                                dpt[i][mask].set(capacity, invalid_dpt_info);
                                // dpt[i][mask][capacity].make_invalid();
                                // std::cout << "outc" << std::endl;
                                continue;
                            }
                            if (introduced_vertex_has_conflict_in_M) {

                                // std::cout << "ind" << std::endl;
                                // dpt[i][mask][capacity].make_invalid();
                                dpt[i][mask].set(capacity, invalid_dpt_info);
                                // std::cout << "outd" << std::endl;

                                continue;
                            }

                            // std::cout << "ine" << std::endl;
                            int vertex_cost = graph.idx_to_cost.at(node.bag[introduced_vertex_idx]);
                            int vertex_volume = graph.idx_to_volume.at(node.bag[introduced_vertex_idx]);
                            // std::cout << "oute" << std::endl;


                            if (capacity < vertex_volume) {
                                // std::cout << "inf" << std::endl;
                                dpt[i][mask].set(capacity, invalid_dpt_info);
                                // dpt[i][mask][capacity].make_invalid();
                                // std::cout << "outf" << std::endl;
                                continue;
                            }
                            unsigned long long mask_without_introduced_vertex = cut_bit(introduced_vertex_idx, mask);

                            
                            // LASTFIX
                            auto tpd_info_to_check = dpt[child_idx][mask_without_introduced_vertex].get(capacity - vertex_volume);
                            if (!tpd_info_to_check.valid) {
                                // dpt[i][mask][capacity].make_invalid();
                                dpt[i][mask].set(capacity, invalid_dpt_info);
                            } else {
                                DptInfo to_insert = DptInfo();
                                to_insert.opt_cost = tpd_info_to_check.opt_cost + vertex_cost;
                                dpt[i][mask].set(capacity, to_insert);
                            }
                            
                        }
                    }

                    // std::cout << "survived" << std::endl;
                }

                break;
            }
            case wrap::FORGET: {
                // std::cout << "In FORGET" << std::endl;
                int child_idx = node.children_idx[0];
                int forgetted_vertex = node.distinguished_vertex;
                int forgetted_vertex_idx = -1;
                for (int j = 0; j < treedecomp[child_idx].bag.size(); j++) {
                    if (treedecomp[child_idx].bag[j] == forgetted_vertex) {
                        forgetted_vertex_idx = j;
                    }
                }
                assert(forgetted_vertex_idx >= 0);


                #pragma omp parallel for
                for (unsigned long long mask = 0; mask < ((unsigned long long)1 << node.bag.size()); mask++) {
                    unsigned long long child_mask_with_forgotten_vertex = insert_bit(forgetted_vertex_idx, 1, mask);
                    unsigned long long child_mask_without_forgotten_vertex = insert_bit(forgetted_vertex_idx, 0, mask);

                    for (int capacity = 0; capacity < max_volume; capacity++) {
                        auto tpd_info_with_forgotten_vertex = dpt[child_idx][child_mask_with_forgotten_vertex].get(capacity);
                        auto tpd_info_without_forgotten_vertex = dpt[child_idx][child_mask_without_forgotten_vertex].get(capacity);
                        if (tpd_info_with_forgotten_vertex.valid && tpd_info_without_forgotten_vertex.valid) {
                            int opt_if_take = tpd_info_with_forgotten_vertex.opt_cost;
                            int opt_if_not_take = tpd_info_without_forgotten_vertex.opt_cost;
                            
                            auto dpt_info_to_insert = DptInfo();
                            dpt_info_to_insert.opt_cost = std::max(opt_if_take, opt_if_not_take);
                            // dpt[i][mask][capacity].opt_cost = std::max(opt_if_take, opt_if_not_take);
                            
                            if (opt_if_take > opt_if_not_take) {
                                dpt_info_to_insert.need_take = 1;
                                // dpt[i][mask][capacity].need_take = 1;
                            } else {
                                dpt_info_to_insert.need_take = 0;
                                // dpt[i][mask][capacity].need_take = 0;
                            }
                            dpt[i][mask].set(capacity, dpt_info_to_insert);
                        } else if (tpd_info_with_forgotten_vertex.valid) {
                            auto dpt_info_to_insert = DptInfo();
                            dpt_info_to_insert.opt_cost = tpd_info_with_forgotten_vertex.opt_cost;
                            dpt_info_to_insert.need_take = 1;
                            dpt[i][mask].set(capacity, dpt_info_to_insert);
                            // dpt[i][mask][capacity].opt_cost = tpd_info_with_forgotten_vertex.opt_cost;
                            // dpt[i][mask][capacity].need_take = 1;
                        } else if (tpd_info_without_forgotten_vertex.valid) {
                            auto dpt_info_to_insert = DptInfo();
                            dpt_info_to_insert.opt_cost = tpd_info_without_forgotten_vertex.opt_cost;
                            dpt_info_to_insert.need_take = 0;
                            dpt[i][mask].set(capacity, dpt_info_to_insert);
                            // dpt[i][mask][capacity].opt_cost = dpt[child_idx][child_mask_without_forgotten_vertex][capacity].opt_cost;
                            // dpt[i][mask][capacity].need_take = 0;
                        } else {
                            dpt[i][mask].set(capacity, invalid_dpt_info);
                            // dpt[i][mask][capacity].make_invalid();
                        }
                    }
                }

                break; 
            }

            case wrap::MERGE: {
                #if 0
                // std::cout << "In MERGE" << std::endl;
                int l_child_idx = node.children_idx[0];
                int r_child_idx = node.children_idx[1];

                // std::cout << "MERGE complexity: " << ((long long)1 << node.bag.size()) * max_volume  * max_volume << std::endl;


                // estimate potential merge complexity:
                long long potential_merge_complexity = 0;
                for (unsigned long long mask = 0; mask < ((unsigned long long)1 << node.bag.size()); mask++) {
                    std::set<int> l_child_costs;
                    std::set<int> r_child_costs;

                    for (int capacity = 0; capacity < max_volume; capacity++) {
                        if (dpt[l_child_idx][mask][capacity].valid) {
                            l_child_costs.insert(dpt[l_child_idx][mask][capacity].opt_cost);
                        }
                        if (dpt[r_child_idx][mask][capacity].valid) {
                            r_child_costs.insert(dpt[r_child_idx][mask][capacity].opt_cost);
                        }
                    }

                    potential_merge_complexity += l_child_costs.size() * r_child_costs.size() * max_volume;
                }
                // std::cout << "POTENTIAL MERGE complexity = " << potential_merge_complexity << std::endl;


                for (unsigned long long mask = 0; mask < ((unsigned long long)1 << node.bag.size()); mask++) {


                    // early exit if mask is not valid
                    if (!dpt[l_child_idx][mask][max_volume - 1].valid || !dpt[r_child_idx][mask][max_volume - 1].valid) {
                        // std::cout << "made invalid" << std::endl;
                        for (int capacity = 0; capacity < max_volume; capacity++) {
                            dpt[i][mask][capacity].make_invalid();
                        }
                        continue;
                    }

                    // compute common cost:
                    int common_cost = 0;
                    for (int bag_idx = 0; bag_idx < node.bag.size(); bag_idx++) {
                        if (bit_extractor_ull(bag_idx) & mask) {
                            common_cost += graph.idx_to_cost.at(node.bag[bag_idx]);
                        }
                    }

                    int common_volume = 0;
                    for (int bag_idx = 0; bag_idx < node.bag.size(); bag_idx++) {
                        if (bit_extractor_ull(bag_idx) & mask) {
                            common_volume += graph.idx_to_volume.at(node.bag[bag_idx]);
                        }
                    }

                    // EXPERIMENTAL PART
                    // for each capacity we want to find a best split
                    // in other words, we have two vectors:
                    // dpt[l_child_idx][mask][0..max_volume] and dpt[r_child_idx][mask][0..max_volume]
                    // we want to compute a (max, +) convolution of these two vector:
                    // convolution of two vectors is a vector of size 2 * max_volume - 1 that maps capacity to best 
                    struct entry{
                        int sum_cost = -1;
                        int sum_vol = -1;
                        int l_idx = -1;
                        int r_idx = -1;
                    };

                    std::vector<entry> possible_sums(2 * max_volume - 1);
                    std::set<std::pair<int, int>> left_bag_cost_index;
                    std::set<std::pair<int, int>> right_bag_cost_index;

                    for (int capacity = 0; capacity < max_volume; capacity++) {
                        if (dpt[l_child_idx][mask][capacity].valid && (capacity == 0 || !dpt[l_child_idx][mask][capacity - 1].valid || dpt[l_child_idx][mask][capacity - 1].opt_cost < dpt[l_child_idx][mask][capacity].opt_cost)) {
                            left_bag_cost_index.insert({dpt[l_child_idx][mask][capacity].opt_cost, capacity});
                        }
                        if (dpt[r_child_idx][mask][capacity].valid && (capacity == 0 || !dpt[r_child_idx][mask][capacity - 1].valid || dpt[r_child_idx][mask][capacity - 1].opt_cost < dpt[r_child_idx][mask][capacity].opt_cost)) {
                            right_bag_cost_index.insert({dpt[r_child_idx][mask][capacity].opt_cost, capacity});
                        }
                    }

                    for (auto it_l: left_bag_cost_index) {
                        for (auto it_r: right_bag_cost_index) {
                            int sum_cost = it_l.first + it_r.first;
                            int sum_vol = it_l.second + it_r.second;
                            // if (sum_vol >= max_volume) {
                            //     continue;
                            // }
                            if (possible_sums[sum_vol].sum_cost == -1) {
                                possible_sums[sum_vol].sum_cost = sum_cost;
                                possible_sums[sum_vol].sum_vol = sum_vol;
                                possible_sums[sum_vol].l_idx = it_l.second;
                                possible_sums[sum_vol].r_idx = it_r.second;
                                continue;
                            }

                            if (possible_sums[sum_vol].sum_cost < sum_cost) {
                                possible_sums[sum_vol].sum_cost = sum_cost;
                                possible_sums[sum_vol].sum_vol = sum_vol;
                                possible_sums[sum_vol].l_idx = it_l.second;
                                possible_sums[sum_vol].r_idx = it_r.second;
                            }
                        }
                    }

                    // take closure to the right
                    entry curr_best = {-1, -1, -1, -1};
                    for (int i = 0; i < possible_sums.size(); i++) {
                        if (possible_sums[i].sum_cost == -1 || curr_best.sum_cost > possible_sums[i].sum_cost) {
                            possible_sums[i] = curr_best;
                            continue;
                        }

                        if (curr_best.sum_cost < possible_sums[i].sum_cost) {
                            curr_best = possible_sums[i];
                        }

                        possible_sums[i] = curr_best;
                    }

                    // now we can use possible_sums to compute the best split

                    for (int capacity = 0; capacity < max_volume; capacity++) {
                        int required_capacity = capacity + common_volume;

                        if (possible_sums[required_capacity].sum_cost == -1) {
                            dpt[i][mask][capacity].make_invalid();
                            continue;
                        }

                        int opt_cost = possible_sums[required_capacity].sum_cost - common_cost;
                        int best_split = possible_sums[required_capacity].l_idx;
                        int best_split_l = possible_sums[required_capacity].l_idx;
                        int best_split_r = possible_sums[required_capacity].r_idx;

                        dpt[i][mask][capacity].opt_cost = opt_cost;
                        dpt[i][mask][capacity].best_split = best_split;
                        dpt[i][mask][capacity].best_split_l = best_split_l;
                        dpt[i][mask][capacity].best_split_r = best_split_r;

                    }


                    // for (int capacity = 0; capacity < max_volume; capacity++) {
                    //     // compute the best capacity split
                    //     int best_split = -1;
                    //     int best_split_l = -1;
                    //     int best_split_r = -1;
                    //     int opt_cost = -1;

                        

                    //     for (int left_split = 0; left_split <= capacity - common_volume; left_split++) {
                    //         int left_capacity = left_split + common_volume;
                    //         int right_capacity = capacity - left_split;

                    //         if (dpt[l_child_idx][mask][left_capacity].valid && dpt[r_child_idx][mask][right_capacity].valid) {
                    //             int cur_opt_cost = dpt[l_child_idx][mask][left_capacity].opt_cost + dpt[r_child_idx][mask][right_capacity].opt_cost - common_cost; // we count common vertices twice
                    //             if (cur_opt_cost > opt_cost) {
                    //                 opt_cost = cur_opt_cost;
                    //                 best_split = left_split;
                    //                 best_split_l = left_capacity;
                    //                 best_split_r = right_capacity;
                    //             }
                    //         }

                    //     }

                    //     if (best_split == -1) {
                    //         dpt[i][mask][capacity].make_invalid();
                    //     } else {
                    //         dpt[i][mask][capacity].opt_cost = opt_cost;
                    //         dpt[i][mask][capacity].best_split = best_split;
                    //         dpt[i][mask][capacity].best_split_l = best_split_l;
                    //         dpt[i][mask][capacity].best_split_r = best_split_r;
                    //     }

                        // for (int split = 0; split <= capacity; split++) {
                        //     int left_capacity = split;
                        //     int right_capacity = capacity - split + common_cost;
                        //     if (left_capacity >= max_volume || right_capacity >= max_volume || left_capacity < 0 || right_capacity < 0) {
                        //         continue;
                        //     }

                        //     if (dpt[l_child_idx][mask][left_capacity].valid && dpt[r_child_idx][mask][capacity - split].valid) {
                        //         int cur_opt_cost = dpt[l_child_idx][mask][left_capacity].opt_cost + dpt[r_child_idx][mask][capacity - split].opt_cost - common_cost; // we count common vertices twice
                        //         if (cur_opt_cost > opt_cost) {
                        //             opt_cost = cur_opt_cost;
                        //             best_split = split;
                        //         }
                        //     }
                        // }

                        // if (best_split == -1) {
                        //     dpt[i][mask][capacity].make_invalid();
                        // } else {
                        //     dpt[i][mask][capacity].opt_cost = opt_cost;
                        //     dpt[i][mask][capacity].best_split = best_split;
                        // }
                        
                    // }
                }
                break;
                #endif
            } 
            default:
                assert(false);
        }
    }

#ifdef PRINT_DPT
    // print dpt:
    auto mask_printer = [](unsigned long long mask, int n) {
        std::string ans = "";
        for (int i = 0; i < n; i++) {
            if (bit_extractor_ull(i) & mask) {
                ans += "1";
            } else {
                ans += "0";
            }
        }
        return ans;
    };
    
    for (int i = 0; i < treedecomp.size(); i++) {
        std::cout << "------------------------------\n";
        std::cout << "bag_idx = " << i << std::endl;
        for (int j = 0; j < ((long long)1) << treedecomp[i].bag.size(); j++) {
            // print mask in binary
            std::cout << "mask=" << mask_printer(j, treedecomp[i].bag.size()) << "\n";
            for (int k = 0; k < max_volume; k++) {
                // print vol=k, opt_cost=opt_cost, best_split=best_split, need_take=need_take
                std::cout << "vol=" << k << " opt_cost=" << dpt[i][j][k].opt_cost << " best_split=" << dpt[i][j][k].best_split << " need_take=" << (dpt[i][j][k].need_take == 1) << "\n";

            }
            std::cout << std::endl;
        }
    }
    std::cout << "------------------------------\n";

#endif
    // opt in the last node

    // print dpt:
    auto mask_printer = [](unsigned long long mask, int n) {
        std::string ans = "";
        for (int i = 0; i < n; i++) {
            if (bit_extractor_ull(i) & mask) {
                ans += "1";
            } else {
                ans += "0";
            }
        }
        return ans;
    };

    // int opt = dpt.back()[0][graph.max_capacity].opt_cost;
    int opt = dpt.back()[0].get(graph.max_capacity).opt_cost;

    std::set<int> taken;
    unsigned long long current_mask = 0;
    int curr_node_idx = treedecomp.size() - 1;
    int curr_capacity = graph.max_capacity;

    // only have forget and introduce nodes
    // going backwards; If we have a forget node, we check if we need to take the vertex
    // if need, the next mask will be with 1 on the vertex
    // if not, the next mask will be with 0 on the vertex 
    // if we in introduce node, and introduced vertex is taken, need to reduce capacity by volume of the vertex


    // attention: this works only for path decomposition
    for (int node_idx = curr_node_idx; node_idx >= 1; node_idx--) {
        //print mask 
        // std::cout << "mask=" << mask_printer(current_mask, treedecomp[node_idx].bag.size()) << "\n";

        const TreeNode& node = treedecomp[node_idx];
        const TreeNode& prev_node = treedecomp[node_idx - 1];
        switch (node.node_type) {
            case wrap::LEAF: {
                break;
            }
            case wrap::INTRODUCE: {
                int introduced_vertex = node.distinguished_vertex;
                int introduced_vertex_idx = -1;
                for (int j = 0; j < node.bag.size(); j++) {
                    if (node.bag[j] == introduced_vertex) {
                        introduced_vertex_idx = j;
                        break;
                    }
                }

                assert(introduced_vertex_idx >= 0);

                

                if (bit_extractor_ull(introduced_vertex_idx) & current_mask) {    
                    curr_capacity -= graph.idx_to_volume.at(introduced_vertex);
                } 

                current_mask = cut_bit(introduced_vertex_idx, current_mask);

                break;
            }
            case wrap::FORGET: {
                int forgetted_vertex = node.distinguished_vertex;
                int forgetted_vertex_idx = -1;
                for (int j = 0; j < treedecomp[node_idx - 1].bag.size(); j++) {
                    if (treedecomp[node_idx - 1].bag[j] == forgetted_vertex) {
                        forgetted_vertex_idx = j;
                    }
                }
                assert(forgetted_vertex_idx >= 0);

                if (dpt[node_idx][current_mask].get(curr_capacity).need_take == 1) {
                    taken.insert(forgetted_vertex);
                    current_mask = insert_bit(forgetted_vertex_idx, 1, current_mask);
                } else {
                    current_mask = insert_bit(forgetted_vertex_idx, 0, current_mask);
                }

                break;
            }

            throw std::logic_error("not implemented");
        }
    }

    return {std::vector<int>(taken.begin(), taken.end()), opt};
}




