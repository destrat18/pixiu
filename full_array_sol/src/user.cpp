// #include <iostream>
// #include <string>
// #include <vector>
// #include <chrono>
// #include <set>
// #include <map>
// #include <unordered_map>
// #include <iomanip>
// #include <sstream>
// #include <cmath>
// #include <future>
// #include <thread>

// #include "comon_defs.hpp"
// #include "config.hpp"
// #include "utils.hpp"
// #include "tdlib_wrapper.hpp"
// #include "nicify.hpp"
// #include "my_graph.hpp"
// #include "independent_set.hpp"
// #include "matching.hpp"
// #include "perfect_matching.hpp"
// #include "parse_gr.hpp"

// typedef double num_for_ind_sets;
// typedef double num_for_matchings;
// typedef double num_for_perfect_matchings;



// template <
//     class result_t   = std::chrono::nanoseconds,
//     class clock_t    = std::chrono::steady_clock,
//     class duration_t = std::chrono::nanoseconds
// >
// auto since(std::chrono::time_point<clock_t, duration_t> const& start)
// {
//     return std::chrono::duration_cast<result_t>(clock_t::now() - start);
// }


// std::string make_csv_string(
//     std::string cid, 
//     int num_of_vertices, 
//     int num_of_edges, 
//     int treewidth, 
//     num_for_ind_sets num_of_independent_sets, 
//     num_for_matchings num_of_matchings,
//     num_for_perfect_matchings num_of_perfect_matchings,
//     long long t_tree_decomposition,
//     long long t_num_of_independent_sets,
//     long long t_num_of_matchings,
//     long long t_num_of_perfect_matchings,
//     long long t_whole_iteration,
//     bool time_limit_exceeded,
//     std::string delimeter=csv_delimeter
//     ) {

//     std::stringstream stream;

//     stream << cid << csv_delimeter <<
//             num_of_vertices << csv_delimeter <<
//             num_of_edges << csv_delimeter <<
//             treewidth << csv_delimeter <<
//             num_of_independent_sets << csv_delimeter <<
//             num_of_matchings  << csv_delimeter <<
//             num_of_perfect_matchings << csv_delimeter <<
//             t_tree_decomposition << csv_delimeter <<
//             t_num_of_independent_sets << csv_delimeter <<
//             t_num_of_matchings << csv_delimeter <<
//             t_num_of_perfect_matchings << csv_delimeter <<
//             t_whole_iteration << csv_delimeter <<
//             time_limit_exceeded;
//     return stream.str();
// }

// std::string make_csv_string_header (std::string delimeter=csv_delimeter) {
//     std::stringstream stream;

//     stream << "cid" << csv_delimeter <<
//             "num_of_vertices" << csv_delimeter <<
//             "num_of_edges" << csv_delimeter <<
//             "treewidth" << csv_delimeter <<
//             "num_of_independent_sets" << csv_delimeter <<
//             "num_of_matchings"  << csv_delimeter <<
//             "num_of_perfect_matchings" << csv_delimeter <<
//             "t_tree_decomposition" << csv_delimeter <<
//             "t_num_of_independent_sets" << csv_delimeter <<
//             "t_num_of_matchings" << csv_delimeter <<
//             "t_num_of_perfect_matchings" << csv_delimeter <<
//             "t_whole_iteration" << csv_delimeter <<
//             "time_limit_exceeded";
//     return stream.str();
// }

// void compute_using_tw(std::vector<std::filesystem::path> graph_names, std::ostream& answer_output_stream) {
//     std::cerr << "the database has been red" << std::endl;

//     std::vector<AdjG> dataset;

    
//     answer_output_stream << make_csv_string_header() << std::endl;
//     int cnt = 0;
//     for (auto file_name: graph_names) {

//         // std::cerr << file_name << std::endl;

//         auto iteration_start = std::chrono::steady_clock::now(); 

//         cnt++;
//         if (cnt % 1000 == 0) {
//             std::cerr << cnt << "/" << graph_names.size() << std::endl;
//         }

//         std::ifstream in(file_name, std::ios_base::in);
//         dataset.push_back(AdjG());


//         /*time intense part begins*/

//         MyGraph g = parse_gr(in);

//         in.close();

//         auto g_edges = g.edge_list();


//         auto start = std::chrono::steady_clock::now(); 
//         auto decomposition = wrap::decomposition3(g_edges, "", std::pow(1.0 + std::max(g.n_vert, 50), 0.5)/1000.0);
//         auto decomposition_time = since(start).count();

//         int max_bag_size = 0;
//         for (auto node: decomposition) {
//             if (node.bag.size() > max_bag_size) {
//                 max_bag_size = node.bag.size();
//             }
//         }
//         if (max_bag_size == 0 || max_bag_size >= MAX_BAG_LIMIT) {
//             answer_output_stream << make_csv_string(
//                 file_name,
//                 g.n_vert,
//                 g_edges.size(),
//                 max_bag_size - 1,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 true
//             ) << std::endl;
//             continue; 
//         }

        
//         start = std::chrono::steady_clock::now();
//         decomposition = wrap::nicify(decomposition);
//         auto nicification_time = since(start).count();


// #ifdef USER_PRINT_DEBUG
//         answer_output_stream << "max bag: " << max_bag_size << std::endl;
// #endif

//         start = std::chrono::steady_clock::now();
//         auto td_independent_set_count = count_independet_set<num_for_ind_sets>(g, decomposition);
//         auto t_independent_set = since(start).count();

//         start = std::chrono::steady_clock::now();
//         auto td_matchings_count = count_matchings<num_for_matchings>(g, decomposition);
//         auto t_matching = since(start).count();

//         start = std::chrono::steady_clock::now();
//         auto td_perfect_matchings_count = count_perfect_matchings<num_for_perfect_matchings>(g, decomposition);
//         auto t_perfect_matching = since(start).count();

//         auto whole_time = since(iteration_start).count();

//         answer_output_stream << make_csv_string(
//             file_name,
//             g.n_vert,
//             g_edges.size(),
//             max_bag_size - 1,
//             td_independent_set_count,
//             td_matchings_count,
//             td_perfect_matchings_count,
//             decomposition_time + nicification_time,
//             t_independent_set,
//             t_matching,
//             t_perfect_matching,
//             whole_time,
//             false
//         ) << std::endl;        
//     }
// }

// void compute_knapsack_using_tw(std::vector<std::filesystem::path> graph_names, std::ostream& answer_output_stream) {}

// void compute_knapsack_using_tw(std::string filename, std::ostream& answer_output_stream) {
//     std::cerr << "the database has been red" << std::endl;

//     std::vector<AdjG> dataset;

//     std::ifstream fin(filename);
    
//     answer_output_stream << make_csv_string_header() << std::endl;
//     int cnt = 0;
//     while (true) {

//         // std::cerr << file_name << std::endl;
//         std::stringstream in;

//         bool break_flag = false;
        
//         std::string file_name, blah;
//         int n = 0, m = 0;

//         if (!(fin >> blah)) {
//             break;
//         }
//         while (true) {
//             fin >> file_name;

//             if (!(fin >> blah)) {
//                 break_flag = true;
//                 break;
//             }
//             if (blah == "cid") {
//                 continue;
//             } 
//             fin >> blah;
//             fin >> n >> m;
//             in << "p tw " << n << " " << m << "\n";
//             break;
//         }
//         if (break_flag) break;

//         for (int i = 0; i < m; i++) {
//             int l, r;
//             fin >> l >> r;
//             in << l << " " << r << std::endl;
//         }


//         auto iteration_start = std::chrono::steady_clock::now(); 

//         cnt++;
//         if (cnt % 1000 == 0) {
//             std::cerr << cnt << std::endl;
//         }

//         // std::ifstream in(file_name, std::ios_base::in);
//         dataset.push_back(AdjG());

//         /*time intense part begins*/
//         MyGraph g = parse_gr(in);
//         auto g_edges = g.edge_list();


//         auto start = std::chrono::steady_clock::now(); 
//         auto decomposition = wrap::decomposition3(g_edges, "", std::pow(1.0 + std::max(g.n_vert, 50), 0.5)/1000.0);
//         auto decomposition_time = since(start).count();

//         int max_bag_size = 0;
//         for (auto node: decomposition) {
//             if (node.bag.size() > max_bag_size) {
//                 max_bag_size = node.bag.size();
//             }
//         }
//         if (max_bag_size == 0 || max_bag_size >= MAX_BAG_LIMIT) {
//             answer_output_stream << make_csv_string(
//                 file_name,
//                 g.n_vert,
//                 g_edges.size(),
//                 max_bag_size - 1,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 true
//             ) << std::endl;
//             continue; 
//         }

        
//         start = std::chrono::steady_clock::now();
//         decomposition = wrap::nicify(decomposition);
//         auto nicification_time = since(start).count();


// #ifdef USER_PRINT_DEBUG
//         std::cerr << "max bag: " << max_bag_size << std::endl;
// #endif

//         start = std::chrono::steady_clock::now();
//         auto td_independent_set_count = count_independet_set<num_for_ind_sets>(g, decomposition);
//         auto t_independent_set = since(start).count();

//         start = std::chrono::steady_clock::now();
//         auto td_matchings_count = count_matchings<num_for_matchings>(g, decomposition);
//         auto t_matching = since(start).count();

//         start = std::chrono::steady_clock::now();
//         auto td_perfect_matchings_count = count_perfect_matchings<num_for_perfect_matchings>(g, decomposition);
//         auto t_perfect_matching = since(start).count();
        
//         auto whole_time = since(iteration_start).count();

//         answer_output_stream << make_csv_string(
//             file_name,
//             g.n_vert,
//             g_edges.size(),
//             max_bag_size - 1,
//             td_independent_set_count,
//             td_matchings_count,
//             td_perfect_matchings_count,
//             decomposition_time + nicification_time,
//             t_independent_set,
//             t_matching,
//             t_perfect_matching,
//             whole_time,
//             false
//         ) << std::endl;        
//     }

// }


// template<class T>
// struct timeout_wrapper {
//     static T call(const MyGraph& g, std::function<T(MyGraph)> fn) {
//         std::mutex m;
//         std::condition_variable cv;
//         T ans;

//         std::thread t(
//             [&cv, &ans, &g, &fn](){
//                 ans = fn(g);
//                 cv.notify_one();
//             }                    
//         );
//         t.detach();

//         {
//             std::unique_lock<std::mutex> l(m);
//             if(cv.wait_for(l, TIMEOUT) == std::cv_status::timeout) 
//                 throw std::runtime_error("Timeout");
//         }

//         return ans;
//     }    
// };


// void compute_using_tw(std::string filename, std::ostream& answer_output_stream) {
//     std::cerr << "the database has been red" << std::endl;

//     std::vector<AdjG> dataset;

//     std::ifstream fin(filename);
    
//     answer_output_stream << make_csv_string_header() << std::endl;
//     int cnt = 0;
//     while (true) {

//         // std::cerr << file_name << std::endl;
//         std::stringstream in;

//         bool break_flag = false;
        
//         std::string file_name, blah;
//         int n = 0, m = 0;

//         if (!(fin >> blah)) {
//             break;
//         }
//         while (true) {
//             fin >> file_name;

//             if (!(fin >> blah)) {
//                 break_flag = true;
//                 break;
//             }
//             if (blah == "cid") {
//                 continue;
//             } 
//             fin >> blah;
//             fin >> n >> m;
//             in << "p tw " << n << " " << m << "\n";
//             break;
//         }
//         if (break_flag) break;

//         for (int i = 0; i < m; i++) {
//             int l, r;
//             fin >> l >> r;
//             in << l << " " << r << std::endl;
//         }


//         auto iteration_start = std::chrono::steady_clock::now(); 

//         cnt++;
//         if (cnt % 1000 == 0) {
//             std::cerr << cnt << std::endl;
//         }

//         // std::ifstream in(file_name, std::ios_base::in);
//         dataset.push_back(AdjG());

//         /*time intense part begins*/
//         MyGraph g = parse_gr(in);
//         auto g_edges = g.edge_list();


//         auto start = std::chrono::steady_clock::now(); 
//         auto decomposition = wrap::decomposition3(g_edges, "", std::pow(1.0 + std::max(g.n_vert, 50), 0.5)/1000.0);
//         auto decomposition_time = since(start).count();

//         int max_bag_size = 0;
//         for (auto node: decomposition) {
//             if (node.bag.size() > max_bag_size) {
//                 max_bag_size = node.bag.size();
//             }
//         }
//         if (max_bag_size == 0 || max_bag_size >= MAX_BAG_LIMIT) {
//             answer_output_stream << make_csv_string(
//                 file_name,
//                 g.n_vert,
//                 g_edges.size(),
//                 max_bag_size - 1,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 0,
//                 true
//             ) << std::endl;
//             continue; 
//         }

        
//         start = std::chrono::steady_clock::now();
//         decomposition = wrap::nicify(decomposition);
//         auto nicification_time = since(start).count();


// #ifdef USER_PRINT_DEBUG
//         std::cerr << "max bag: " << max_bag_size << std::endl;
// #endif

//         start = std::chrono::steady_clock::now();
//         auto td_independent_set_count = count_independet_set<num_for_ind_sets>(g, decomposition);
//         auto t_independent_set = since(start).count();

//         start = std::chrono::steady_clock::now();
//         auto td_matchings_count = count_matchings<num_for_matchings>(g, decomposition);
//         auto t_matching = since(start).count();

//         start = std::chrono::steady_clock::now();
//         auto td_perfect_matchings_count = count_perfect_matchings<num_for_perfect_matchings>(g, decomposition);
//         auto t_perfect_matching = since(start).count();
        
//         auto whole_time = since(iteration_start).count();

//         answer_output_stream << make_csv_string(
//             file_name,
//             g.n_vert,
//             g_edges.size(),
//             max_bag_size - 1,
//             td_independent_set_count,
//             td_matchings_count,
//             td_perfect_matchings_count,
//             decomposition_time + nicification_time,
//             t_independent_set,
//             t_matching,
//             t_perfect_matching,
//             whole_time,
//             false
//         ) << std::endl;        
//     }
// }


// void compute_using_naive(std::vector<std::filesystem::path> graph_names, std::ostream& answer_output_stream) {
//     std::cerr << "the database has been red" << std::endl;

//     std::vector<AdjG> dataset;

    
//     answer_output_stream << make_csv_string_header() << std::endl;
//     int cnt = 0;
//     for (auto file_name: graph_names) {

//         auto iteration_start = std::chrono::steady_clock::now(); 

//         cnt++;
//         if (cnt % 1000 == 0) {
//             std::cerr << cnt << "/" << graph_names.size() << std::endl;
//         }

//         std::ifstream in(file_name, std::ios_base::in);
//         dataset.push_back(AdjG());


//         /*time intense part begins*/

//         MyGraph g = parse_gr(in);

//         auto g_edges = g.edge_list();

//         int64_t t_independent_set = -1;
//         num_for_ind_sets td_independent_set_count = -1;
//         try {
//             auto start = std::chrono::steady_clock::now();
//             td_independent_set_count = timeout_wrapper<num_for_ind_sets>::call(g, count_independet_set_branching<num_for_ind_sets>);
//             t_independent_set = since(start).count();
//         } catch (std::runtime_error& e) {
//             t_independent_set = -1;
//             td_independent_set_count = -1;
//         }

//         int64_t t_matching = -1;
//         num_for_matchings td_matchings_count = -1;
//         try {
//             auto start = std::chrono::steady_clock::now();
//             td_matchings_count = timeout_wrapper<num_for_matchings>::call(g, count_matchings_branching<num_for_matchings>);
//             t_matching = since(start).count();
//         } catch (std::runtime_error& e) {
//             t_matching = -1;
//             td_matchings_count = -1;
//         }

//         int64_t t_perfect_matching = -1;
//         num_for_perfect_matchings td_perfect_matchings_count = -1;
//         try {
//             auto start = std::chrono::steady_clock::now();
//             td_perfect_matchings_count = timeout_wrapper<num_for_perfect_matchings>::call(g, count_perfect_matchings_branching<num_for_perfect_matchings>);
//             t_perfect_matching = since(start).count();
//         } catch (std::runtime_error& e) {
//             t_perfect_matching = -1;
//             td_perfect_matchings_count = -1;
//         }

//         auto whole_time = since(iteration_start).count();

//         answer_output_stream << make_csv_string(
//             file_name,
//             g.n_vert,
//             g_edges.size(),
//             -1,
//             td_independent_set_count,
//             td_matchings_count,
//             td_perfect_matchings_count,
//             -1,
//             t_independent_set,
//             t_matching,
//             t_perfect_matching,
//             whole_time,
//             false
//         ) << std::endl;        
//     }
// }

// void compute_knapsack_using_naive(std::vector<std::filesystem::path> graph_names, std::ostream& answer_output_stream) {}

// void compute_knapsack_using_naive(std::string filename, std::ostream& answer_output_stream) {}


// void compute_using_naive(std::string filename, std::ostream& answer_output_stream) {
//     std::cerr << "the database has been red" << std::endl;

//     std::vector<AdjG> dataset;

//     std::ifstream fin(filename);
    
//     answer_output_stream << make_csv_string_header() << std::endl;
//     int cnt = 0;
//     while (true) {

//         // std::cerr << file_name << std::endl;
//         std::stringstream in;

//         bool break_flag = false;
        
//         std::string file_name, blah;
//         int n = 0, m = 0;

//         if (!(fin >> blah)) {
//             break;
//         }
//         while (true) {
//             fin >> file_name;

//             if (!(fin >> blah)) {
//                 break_flag = true;
//                 break;
//             }
//             if (blah == "cid") {
//                 continue;
//             } 
//             fin >> blah;
//             fin >> n >> m;
//             in << "p tw " << n << " " << m << "\n";
//             break;
//         }
//         if (break_flag) break;

//         for (int i = 0; i < m; i++) {
//             int l, r;
//             fin >> l >> r;
//             in << l << " " << r << std::endl;
//         }


//         auto iteration_start = std::chrono::steady_clock::now(); 

//         cnt++;
//         if (cnt % 1000 == 0) {
//             std::cerr << cnt << std::endl;
//         }

//         dataset.push_back(AdjG());

//         /*time intense part begins*/

//         MyGraph g = parse_gr(in);

//         auto g_edges = g.edge_list();

//         int64_t t_independent_set = -1;
//         num_for_ind_sets td_independent_set_count = -1;
//         try {
//             auto start = std::chrono::steady_clock::now();
//             td_independent_set_count = timeout_wrapper<num_for_ind_sets>::call(g, count_independet_set_branching<num_for_ind_sets>);
//             t_independent_set = since(start).count();
//         } catch (std::runtime_error& e) {
//             t_independent_set = -1;
//             td_independent_set_count = -1;
//         }

//         int64_t t_matching = -1;
//         num_for_matchings td_matchings_count = -1;
//         try {
//             auto start = std::chrono::steady_clock::now();
//             td_matchings_count = timeout_wrapper<num_for_matchings>::call(g, count_matchings_branching<num_for_matchings>);
//             t_matching = since(start).count();
//         } catch (std::runtime_error& e) {
//             t_matching = -1;
//             td_matchings_count = -1;
//         }

//         int64_t t_perfect_matching = -1;
//         num_for_perfect_matchings td_perfect_matchings_count = -1;
//         try {
//             auto start = std::chrono::steady_clock::now();
//             td_perfect_matchings_count = timeout_wrapper<num_for_perfect_matchings>::call(g, count_perfect_matchings_branching<num_for_perfect_matchings>);
//             t_perfect_matching = since(start).count();
//         } catch (std::runtime_error& e) {
//             t_perfect_matching = -1;
//             td_perfect_matchings_count = -1;
//         }

//         auto whole_time = since(iteration_start).count();

//         answer_output_stream << make_csv_string(
//             file_name,
//             g.n_vert,
//             g_edges.size(),
//             -1,
//             td_independent_set_count,
//             td_matchings_count,
//             td_perfect_matchings_count,
//             -1,
//             t_independent_set,
//             t_matching,
//             t_perfect_matching,
//             whole_time,
//             false
//         ) << std::endl;        
//     }


// }


// void unimplemented() {
//     throw std::runtime_error("unimplemented");
// }



// int main(int argc, char *argv[]) {   

//     // ./run filename res_name naive|tw

//     namespace fs = std::filesystem;

//     if (argc != 4) {
//         std::cout << "usage: ./run input_file_or_folder output_file_name naive|tw" << std::endl; 
//         return 0;
//     }

//     std::string folder_or_file = std::string(argv[1]);
//     std::string result_file = std::string(argv[2]);
//     std::string method = std::string(argv[3]);

//     if (method != "naive" && method != "tw") {
//         std::cout << "method should be tw or naive, " << method << " given" << std::endl;
//         return 0;
//     }

//     std::ofstream result_file_stream(result_file);

//     if (fs::is_directory(folder_or_file)) {
//         std::cout << "folder given, method = " << method << ", result_file = " << result_file << std::endl;
//         if (method == "naive") {
//             unimplemented();

//             // auto graph_names = list_files_ends_with(folder_or_file, ".kgr");
//             // compute_knapsack_using_naive(graph_names, result_file_stream);
//         } else if (method == "tw") {
//             unimplemented();

//             // auto graph_names = list_files_ends_with(folder_or_file, ".kgr");
//             // compute_knapsack_using_tw(graph_names, result_file_stream);
//         }
//     } else {
//         std::cout << "file given, method = " << method << ", result_file = " << result_file <<  std::endl;
//         if (method == "naive") {
//             unimplemented();

//             // compute_knapsack_using_naive(folder_or_file, result_file_stream);
//         } else if (method == "tw") {
//             compute_knapsack_using_tw(folder_or_file, result_file_stream);
//         }
//     }

// }