#pragma once

#include <vector>
#include <string>
#include <filesystem>


std::vector<std::filesystem::path> list_files_ends_with(const std::string& path, const std::string& suff);

