#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <set>
#include <map>
#include <unordered_map>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <future>
#include <thread>

#include "comon_defs.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "tdlib_wrapper.hpp"
#include "nicify.hpp"
#include "my_graph.hpp"
#include "independent_set.hpp"
#include "matching.hpp"
#include "perfect_matching.hpp"
#include "parse_gr.hpp"
#include "knapsack.hpp"

typedef double num_for_ind_sets;
typedef double num_for_matchings;
typedef double num_for_perfect_matchings;


#define USER_PRINT_DEBUG


template <
    class result_t   = std::chrono::nanoseconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::nanoseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}


std::string make_csv_string(
    std::string cid, 
    int num_of_vertices, 
    int num_of_edges, 
    int treewidth, 
    num_for_ind_sets num_of_independent_sets, 
    num_for_matchings num_of_matchings,
    num_for_perfect_matchings num_of_perfect_matchings,
    long long t_tree_decomposition,
    long long t_num_of_independent_sets,
    long long t_num_of_matchings,
    long long t_num_of_perfect_matchings,
    long long t_whole_iteration,
    bool time_limit_exceeded,
    std::string delimeter=csv_delimeter
    ) {

    std::stringstream stream;

    stream << cid << csv_delimeter <<
            num_of_vertices << csv_delimeter <<
            num_of_edges << csv_delimeter <<
            treewidth << csv_delimeter <<
            num_of_independent_sets << csv_delimeter <<
            num_of_matchings  << csv_delimeter <<
            num_of_perfect_matchings << csv_delimeter <<
            t_tree_decomposition << csv_delimeter <<
            t_num_of_independent_sets << csv_delimeter <<
            t_num_of_matchings << csv_delimeter <<
            t_num_of_perfect_matchings << csv_delimeter <<
            t_whole_iteration << csv_delimeter <<
            time_limit_exceeded;
    return stream.str();
}

std::string make_csv_string_header (std::string delimeter=csv_delimeter) {
    std::stringstream stream;

    stream << "cid" << csv_delimeter <<
            "num_of_vertices" << csv_delimeter <<
            "num_of_edges" << csv_delimeter <<
            "treewidth" << csv_delimeter <<
            "num_of_independent_sets" << csv_delimeter <<
            "num_of_matchings"  << csv_delimeter <<
            "num_of_perfect_matchings" << csv_delimeter <<
            "t_tree_decomposition" << csv_delimeter <<
            "t_num_of_independent_sets" << csv_delimeter <<
            "t_num_of_matchings" << csv_delimeter <<
            "t_num_of_perfect_matchings" << csv_delimeter <<
            "t_whole_iteration" << csv_delimeter <<
            "time_limit_exceeded";
    return stream.str();
}


void compute_knapsack_using_tw(std::vector<std::filesystem::path> graph_names, std::ostream& answer_output_stream) {}


void compute_knapsack_using_tw(std::string filename, std::string tdp_filename, std::ostream& answer_output_stream) {
    // std::cerr << "the database has been red" << std::endl;


    std::ifstream fin(filename);
        

    auto iteration_start = std::chrono::steady_clock::now(); 

 
    // std::ifstream in(file_name, std::ios_base::in);
    /*time intense part begins*/
    MyGraph g = parse_kgr2(fin);
    // g.print();
    auto g_edges = g.edge_list();


    // auto start = std::chrono::steady_clock::now(); 
    // auto decomposition = wrap::decomposition3(g_edges, g, "", 1.0);
    // auto decomposition_time = since(start).count();

    // open file
    std::ifstream tdp_file(tdp_filename);
    auto decomposition = wrap::parse_tdp_to_nice(tdp_file);

    // // print decomposition
    // for (auto node: decomposition) {
    //     std::string nodetype;
    //     if (node.node_type == NodeType::INTRODUCE) {
    //         nodetype = "INTRODUCE";
    //     } else if (node.node_type == NodeType::FORGET) {
    //         nodetype = "FORGET";
    //     } else if (node.node_type == NodeType::LEAF) {
    //         nodetype = "LEAF";
    //     } else {
    //         nodetype = "UNKNOWN";
    //     }
    //     std::cout << node.idx << " " << nodetype << " " << node.distinguished_vertex << " [";
    //     for (auto it: node.bag) {
    //         std::cout << it << " ";
    //     }
    //     std::cout << "] " << node.children_idx.size() << " [";
    //     for (auto it: node.children_idx) {
    //         std::cout << it << " ";
    //     }
    //     std::cout << "]" << std::endl;
    // }


    int max_bag_size = 0;
    for (auto node: decomposition) {
        if (node.bag.size() > max_bag_size) {
            max_bag_size = node.bag.size();
        }
    }


#ifdef USER_PRINT_DEBUG
        // std::cerr << "max bag: " << max_bag_size << std::endl;
#endif

    auto start = std::chrono::steady_clock::now();
    auto td_independent_set_count = solve_knapsack(g, decomposition);
    auto t_independent_set = since(start).count();

    auto subset = td_independent_set_count.first;


    answer_output_stream << filename << "\n";
    answer_output_stream << td_independent_set_count.second << "\n";
    answer_output_stream << subset.size() << "\n";
    for (auto it: subset) {
        answer_output_stream << it + 1 << " ";
    }
    answer_output_stream << "\n";
    for (auto it: subset) {
        answer_output_stream << g.idx_to_hash[it] << "\n";
    }


    // std::cout << "Subset (hash, volume, cost):";
    // for (auto it: td_independent_set_count.first) {
    //     // hash, volume, cost 
    //     std::cout << g.idx_to_hash[it] << " " << g.idx_to_volume[it] << " " << g.idx_to_cost[it] << std::endl;
    // }
    // std::cout << "Cost: " << td_independent_set_count.second << std::endl;

    
}

template<class T>
struct timeout_wrapper {
    static T call(const MyGraph& g, std::function<T(MyGraph)> fn) {
        std::mutex m;
        std::condition_variable cv;
        T ans;

        std::thread t(
            [&cv, &ans, &g, &fn](){
                ans = fn(g);
                cv.notify_one();
            }                    
        );
        t.detach();

        {
            std::unique_lock<std::mutex> l(m);
            if(cv.wait_for(l, TIMEOUT) == std::cv_status::timeout) 
                throw std::runtime_error("Timeout");
        }

        return ans;
    }    
};



void compute_knapsack_using_naive(std::vector<std::filesystem::path> graph_names, std::ostream& answer_output_stream) {}

void compute_knapsack_using_naive(std::string filename, std::ostream& answer_output_stream) {}




void unimplemented() {
    throw std::runtime_error("unimplemented");
}



int main(int argc, char *argv[]) {  

    // ./run filename res_name naive|tw

    namespace fs = std::filesystem;

    if (argc != 4) {
        std::cout << "usage: ./run input_kdr_file tdp_decomposition_file output_file" << std::endl; 
        return 0;
    }

    std::string kgr_file = std::string(argv[1]);
    std::string tdp_file = std::string(argv[2]);
    std::string result_file = std::string(argv[3]);
 

    std::ofstream result_file_stream(result_file);
    // std::cout << "folder given, method = " << method << ", result_file = " << result_file << std::endl;
    compute_knapsack_using_tw(kgr_file, tdp_file, result_file_stream);


}