#include <vector>
#include <string>
#include <iostream>
#include <filesystem>

#include <boost/algorithm/string.hpp>

#include "utils.hpp"

namespace fs = std::filesystem;

std::vector<std::filesystem::path> list_files_ends_with(const std::string& path, const std::string& suff) {
    std::vector<std::filesystem::path> ans;

    for (const auto & entry : fs::directory_iterator(path)) {
        if (boost::algorithm::ends_with(std::string(entry.path()), suff)) {
            ans.push_back(entry.path());
        }
    }
    return ans;
}