#include <vector>
#include <unordered_map>
// #include <unordered_unordered_map>
#include <algorithm>

#include "comon_defs.hpp"
#include "nicify.hpp"



namespace wrap{
std::vector<TreeNode> nicify(const std::vector<TreeNode>& not_nice) {
    std::vector<TreeNode> deg_lg_2_leaves_root_empty;
    std::unordered_map<Vert, Vert> relabel;

    int free_label = 0;

    for (int i = 0; i < not_nice.size(); i++) {
        if (not_nice[i].children.size() == 0 && not_nice[i].bag.size() == 0) {
            deg_lg_2_leaves_root_empty.push_back(TreeNode());
            deg_lg_2_leaves_root_empty.back().v = free_label++; 
            relabel[not_nice[i].v] = (Vert)(free_label - 1);
        } if (not_nice[i].children.size() == 0 && not_nice[i].bag.size() > 0) { //empty 
            // empty leaf
            deg_lg_2_leaves_root_empty.push_back(TreeNode());
            deg_lg_2_leaves_root_empty.back().v = free_label++; 
            int leaf_label = free_label - 1;
            
            // copy old vertex with link to empty leaf
            deg_lg_2_leaves_root_empty.push_back(TreeNode());
            deg_lg_2_leaves_root_empty.back().v = free_label++;
            deg_lg_2_leaves_root_empty.back().bag = not_nice[i].bag;
            deg_lg_2_leaves_root_empty.back().children.push_back(leaf_label);
            relabel[not_nice[i].v] = (Vert)(free_label - 1);
        } else if (not_nice[i].children.size() == 1) {
            deg_lg_2_leaves_root_empty.push_back(TreeNode());
            deg_lg_2_leaves_root_empty.back().v = free_label++;
            deg_lg_2_leaves_root_empty.back().bag = not_nice[i].bag;
            deg_lg_2_leaves_root_empty.back().children = {relabel[not_nice[i].children[0]]};
            relabel[not_nice[i].v] = (Vert)(free_label - 1);
        } else if (not_nice[i].children.size() == 2) {
            deg_lg_2_leaves_root_empty.push_back(TreeNode());
            deg_lg_2_leaves_root_empty.back().v = free_label++;
            deg_lg_2_leaves_root_empty.back().bag = not_nice[i].bag;
            deg_lg_2_leaves_root_empty.back().children = {relabel[not_nice[i].children[0]], relabel[not_nice[i].children[1]]};
            relabel[not_nice[i].v] = (Vert)(free_label - 1);
        } else if (not_nice[i].children.size() > 2) {
            // duplicating node 
            for (int copied_node_ind = 0; copied_node_ind < not_nice[i].children.size() - 1; copied_node_ind++) {
                if (copied_node_ind == 0) {
                    deg_lg_2_leaves_root_empty.push_back(TreeNode());
                    deg_lg_2_leaves_root_empty.back().v = free_label++;
                    deg_lg_2_leaves_root_empty.back().bag = not_nice[i].bag;
                    deg_lg_2_leaves_root_empty.back().children = {relabel[not_nice[i].children[0]], relabel[not_nice[i].children[1]]};
                } else {
                    deg_lg_2_leaves_root_empty.push_back(TreeNode());
                    int prev_label = free_label - 1;
                    deg_lg_2_leaves_root_empty.back().v = free_label++;
                    deg_lg_2_leaves_root_empty.back().bag = not_nice[i].bag;
                    deg_lg_2_leaves_root_empty.back().children = {(Vert)prev_label, relabel[not_nice[i].children[copied_node_ind + 1]]};
                }
            }
            relabel[not_nice[i].v] = (Vert)(free_label - 1);
        }        
    }
    if (deg_lg_2_leaves_root_empty.back().bag.size() > 0) {
        deg_lg_2_leaves_root_empty.push_back(TreeNode());
        deg_lg_2_leaves_root_empty.back().v = free_label++;
        deg_lg_2_leaves_root_empty.back().children = {deg_lg_2_leaves_root_empty[deg_lg_2_leaves_root_empty.size() - 2].v};
    }


    for (int i = 0; i < deg_lg_2_leaves_root_empty.size(); i++) {
        std::sort(deg_lg_2_leaves_root_empty[i].bag.begin(), deg_lg_2_leaves_root_empty[i].bag.end());
        deg_lg_2_leaves_root_empty[i].bag.erase(std::unique(deg_lg_2_leaves_root_empty[i].bag.begin(), deg_lg_2_leaves_root_empty[i].bag.end()), deg_lg_2_leaves_root_empty[i].bag.end());
    }

    auto intersect = [](const std::vector<int>& a, const std::vector<int>& b) {
        std::set<int> b_set;
        for (auto it: b) b_set.insert(it);
        std::set<int> ans;

        for (auto it: a) {
            if (b_set.find(it) != b_set.end()) {
                ans.insert(it);
            }
        }
        return ans;        
    };

    auto unite = [](const std::vector<int>& a, const std::vector<int>& b) {
        std::set<int> ans_set;
        for (auto it: a) ans_set.insert(it);
        for (auto it: b) ans_set.insert(it);

        std::set<int> ans;
        for (auto it: ans_set) ans.insert(it);
        return ans;
    };

    auto setminus = [](const std::vector<int>& a, const std::vector<int>& b) { 
        std::set<int> b_set;
        for (auto it: b) b_set.insert(it);
        std::set<int> ans;

        for (auto it: a) {
            if (b_set.find(it) == b_set.end()) {
                ans.insert(it);
            }
        }
        return ans;
    };

    // nicify joins by duplicating 
    std::vector<TreeNode> nice_joins;
    std::unordered_map<Vert, Vert> relabel_again_again;

    free_label = 0;
    for (auto curr_v: deg_lg_2_leaves_root_empty) {
        if (curr_v.children.size() == 2) {
            nice_joins.push_back(TreeNode());
            nice_joins.push_back(TreeNode());
            nice_joins.push_back(TreeNode());
            int tmp_size = nice_joins.size();
            TreeNode& new_l_child = nice_joins[tmp_size - 3];
            TreeNode& new_r_child = nice_joins[tmp_size - 2];
            TreeNode& parent = nice_joins[tmp_size - 1];
            new_l_child.v = free_label++;
            new_r_child.v = free_label++;
            parent.v = free_label++;
            relabel_again_again[curr_v.v] = (Vert)(free_label - 1);

            for (auto it: curr_v.bag) {
                new_l_child.bag.push_back(it);
                new_r_child.bag.push_back(it);
                parent.bag.push_back(it);
            }

            new_l_child.children = {relabel_again_again[curr_v.children[0]]};
            new_r_child.children = {relabel_again_again[curr_v.children[1]]};
            parent.children = {new_l_child.v, new_r_child.v};
        } else {
            nice_joins.push_back(TreeNode());
            relabel_again_again[curr_v.v] = free_label;
            nice_joins.back().v = free_label++;
            for (auto it: curr_v.children) {
                nice_joins.back().children.push_back(relabel_again_again[it]);
            }
            nice_joins.back().bag.assign(curr_v.bag.begin(), curr_v.bag.end());
        }
    }

    std::unordered_map<Vert, int> vert_to_idx;

    for (int i = 0; i < nice_joins.size(); i++) {
        vert_to_idx[nice_joins[i].v] = i;
    }

    std::vector<TreeNode> completely_nice;
    std::unordered_map<Vert, Vert> relabel_again;

    free_label = 0;
    for (auto curr_v: nice_joins) {
        if (curr_v.children.size() == 0) {
            completely_nice.push_back(TreeNode());
            completely_nice.back().bag = curr_v.bag;
            relabel_again[curr_v.v] = (Vert)free_label;
            completely_nice.back().v = free_label++;
            completely_nice.back().node_type = LEAF;
            continue;
        }
        TreeNode l_child = nice_joins[vert_to_idx[curr_v.children[0]]];
        TreeNode r_child = (curr_v.children.size() == 2) ? nice_joins[vert_to_idx[curr_v.children[1]]] : TreeNode();
        if (curr_v.children.size() == 1) {
            auto curr_v_minus_child = setminus(curr_v.bag, l_child.bag);
            auto child_minus_curr_v = setminus(l_child.bag, curr_v.bag);
            auto child_intersect_curr_v = intersect(curr_v.bag, l_child.bag);

            if (curr_v_minus_child.size() == 0 && child_minus_curr_v.size() == 0) {
                completely_nice.push_back(TreeNode());
                completely_nice.back().bag = curr_v.bag;
                relabel_again[curr_v.v] = (Vert)free_label;
                completely_nice.back().v = free_label++;
                completely_nice.back().arb_flag1 = true;
                completely_nice.back().children = {relabel_again[curr_v.children[0]]};
                continue;
            }


            TreeNode curr_node;
            curr_node.bag.assign(l_child.bag.begin(), l_child.bag.end());

            Vert prev_vert = relabel_again[l_child.v];
            std::set<int> elems_to_exclude(child_minus_curr_v);
            while(!elems_to_exclude.empty()) {
                auto top_elem = elems_to_exclude.begin();

                completely_nice.push_back(TreeNode());
                completely_nice.back().children = {prev_vert};
                prev_vert = free_label;
                completely_nice.back().v = free_label++;
                completely_nice.back().node_type = FORGET;
                completely_nice.back().distinguished_vertex = *top_elem;
                elems_to_exclude.erase(top_elem);
                completely_nice.back().bag.assign(child_intersect_curr_v.begin(), child_intersect_curr_v.end());
                for (auto not_excluded_yet: elems_to_exclude) {
                    completely_nice.back().bag.push_back(not_excluded_yet);
                }
            }

            std::vector<int> elems_to_include(curr_v_minus_child.begin(), curr_v_minus_child.end());
            for (int elem_to_include_idx = 0; elem_to_include_idx < elems_to_include.size(); elem_to_include_idx++) {
                completely_nice.push_back(TreeNode());
                completely_nice.back().children = {prev_vert};
                prev_vert = free_label;
                completely_nice.back().v = free_label++;
                completely_nice.back().bag.assign(child_intersect_curr_v.begin(), child_intersect_curr_v.end());
                for (int i = 0; i < elem_to_include_idx + 1; i++) {
                    completely_nice.back().bag.push_back(elems_to_include[i]);
                }
                completely_nice.back().node_type = INTRODUCE;
                completely_nice.back().distinguished_vertex = elems_to_include[elem_to_include_idx];

            }

            relabel_again[curr_v.v] = (Vert)(free_label - 1);

        } else if (curr_v.children.size() == 2) {
            completely_nice.push_back(TreeNode());
            for (auto it: curr_v.bag) {
                completely_nice.back().bag.push_back(it);
            }
            relabel_again[curr_v.v] = free_label;
            completely_nice.back().v = free_label++;
            completely_nice.back().children = {relabel_again[l_child.v], relabel_again[r_child.v]};
            completely_nice.back().node_type = MERGE;
        } else {
            assert(false);
        }
    }


    std::set<Vert> dead_v;
    std::unordered_map<Vert, Vert> dead_reunordered_map;

    for (auto curr_v: completely_nice) {
        if (curr_v.arb_flag1) {
            dead_v.insert(curr_v.v);
            if (dead_v.find(curr_v.children[0]) == dead_v.end()) {
                dead_reunordered_map[curr_v.v] = curr_v.children[0];
            } else {
                dead_reunordered_map[curr_v.v] = dead_reunordered_map[curr_v.children[0]];
            }
        }
    }


    std::vector<TreeNode> completely_nice_no_dead;

    for (auto curr_v: completely_nice) {
        if (dead_v.find(curr_v.v) != dead_v.end()) {
            continue;
        }

        completely_nice_no_dead.push_back(TreeNode());
        completely_nice_no_dead.back().v = curr_v.v;
        for (auto child: curr_v.children) {
            if (dead_v.find(child) != dead_v.end()) {
                completely_nice_no_dead.back().children.push_back(dead_reunordered_map[child]);
            } else {
                completely_nice_no_dead.back().children.push_back(child);
            }
        }
        completely_nice_no_dead.back().node_type = curr_v.node_type;
        completely_nice_no_dead.back().bag.assign(curr_v.bag.begin(), curr_v.bag.end());
        completely_nice_no_dead.back().distinguished_vertex = curr_v.distinguished_vertex;
    }


    std::map<Vert, int> pos_of_vert_in_arr;
    for (int curr_pos = 0; curr_pos < completely_nice_no_dead.size(); curr_pos++){
        TreeNode& curr_v = completely_nice_no_dead[curr_pos];
        std::sort(curr_v.bag.begin(), curr_v.bag.end());
        pos_of_vert_in_arr[curr_v.v] = curr_pos;
        curr_v.idx = curr_pos;
        for (auto child: curr_v.children) {
            curr_v.children_idx.push_back(pos_of_vert_in_arr[child]);
        }
    }



    return completely_nice_no_dead;
}

}