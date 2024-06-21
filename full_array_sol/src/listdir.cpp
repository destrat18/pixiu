#include <string>
#include <iostream>
#include <filesystem>
#include <boost/algorithm/string.hpp>

namespace fs = std::filesystem;

int main()
{
    
    
    std::string path = ".";
    for (const auto & entry : fs::directory_iterator(path)) {
        if (boost::algorithm::ends_with(std::string(entry.path()), ".gr")) {
            std::cout << std::string(entry.path()).substr(2, std::string::npos) << std::endl;
        }
        
    }
}