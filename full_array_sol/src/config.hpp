#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
using namespace std::chrono_literals;

const std::filesystem::path path_to_treedecomposition_utility("/home/sergey/hkust/research/pace-lib/flow-cutter-pace17/flow_cutter_pace17");
// const std::filesystem::path path_to_treedecomposition_utility("/Users/harshit/My Drive/Projects/New life/HKUST/Projects/Chemical Graph Theory/Sergeie/flow-cutter-pace17");
// const std::filesystem::path path_to_folder_with_graphs("extreme_graph");
// const std::filesystem::path path_to_folder_with_graphs("/home/sergey/hkust/research/paramethrized-chemistry/experiments/graphs");
const std::string csv_delimeter = "\t";


const auto TIMEOUT = 600s;
