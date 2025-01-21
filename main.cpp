#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <boost/crc.hpp> 

std::uint32_t crc32Hash(const std::string& input) {
    boost::crc_32_type crc32;
    crc32.process_bytes(input.data(), input.size());
    return crc32.checksum();
}

namespace po = boost::program_options;
namespace fs = std::filesystem;

void writeLineToFile(const std::string& line) {
    std::ofstream file("directory.txt", std::ios::app);
    file << line << std::endl;
    file.close();
}

void readLinesFromFile(std::vector<std::string>& directory_list) {
    std::ifstream file("directory.txt");
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) { 
            directory_list.push_back(line);
        }
    }
    file.close();
}

void readLinesFromDFile(std::vector<int>& data){
    std::ifstream file("data.txt");
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) { 
            data.push_back(std::stoi(line));
        }
    }
    file.close();
}

void removeLineFromFile(const std::string& element) {
    std::ifstream file("directory.txt");
    std::ofstream tempFile("temp.txt");
    std::string line;
    while (std::getline(file, line)) {
        if (line != element) {
            tempFile << line << std::endl; 
        } 
    }
    file.close();
    tempFile.close();
    std::remove("directory.txt");
    std::rename("temp.txt", "directory.txt");
}

void replaceLineInFile(const int& oldLine, const int& newLine) {
    std::ifstream inputFile("data.txt");

    
    std::vector<std::string> lines;
    std::string line;

    int index = 0;
    while (std::getline(inputFile, line)) {
        if (index == oldLine) {
            lines.push_back(std::to_string(newLine));  
        } else {
            lines.push_back(line);      
        }
        index++;
    }
    
    inputFile.close();  

    
    std::ofstream outputFile("data.txt");
    for (const auto& l : lines) {
        outputFile << l << std::endl;  
    }

    outputFile.close();  
}

std::vector<std::uint32_t> HeshString(std::string word, int size_block){
    if(word.size() % size_block != 0){
        std::string s = std::string(size_block - (word.size() % size_block), ' ');
        word += s;
    }
    std::vector<std::uint32_t> hesh_word;
    for(int i = 0; i < word.size(); i += size_block){
        std::uint32_t hesh_block = crc32Hash(word.substr(i, size_block));
        hesh_word.push_back(hesh_block);
        //std::cout << word.substr(i, size_block) << "---" << hesh_block << std::endl;
    }

    return hesh_word;
}

bool FindInFile(const std::string& path, std::vector<std::uint32_t>& word, int size_block){
    std::ifstream file(path);
    std::string line, text;
    
    while (std::getline(file, line)) {
        if (!line.empty()) { 
            text += line;
        }
    }
    std::vector<std::uint32_t> hesh_text = HeshString(text, size_block);

    // std::cout << text << std::endl;
    // for(int i = 0; i < hesh_text.size(); i++){
    //     std::cout << hesh_text[i] << "  ";
    // }
    // std::cout << std::endl;
    // for(std::uint32_t i : word){
    //     std::cout << i << "  ";
    // }
    // std::cout << std::endl;
    file.close();
    bool t;
    if(word.size() <= hesh_text.size()){
        for(int i = 0; i < (hesh_text.size() - word.size() + 1); i++){
            t = 1;
            for(int ii = 0; ii < word.size(); ii++){
                if(word[ii] != hesh_text[ii + i]){
                    t = 0;
                }
            }
            if(t){
                return t;
            }
        }
    }
    return 0;
}

void listFiles(const std::string& directory, int includeSubdirs, size_t minSize, const std::string& mask, const std::string& word, int size_block) {
    fs::path dirPath(directory);
    
    std::vector<std::uint32_t> hesh_word = HeshString(word, size_block);
    std::vector<std::string> all_fils;
    std::vector<std::string> fils;

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (fs::is_regular_file(entry.status())) {
            auto fileSize = fs::file_size(entry.path());
            auto fileName = entry.path().filename().string();

            
            if (fileSize >= minSize && (mask.empty() || fileName.find(mask) != std::string::npos)) {
                all_fils.push_back(entry.path().string());
            }
        }
    }

    
    if (includeSubdirs) {
        for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
            if (fs::is_regular_file(entry.status())) {
                auto fileSize = fs::file_size(entry.path());
                auto fileName = entry.path().filename().string();

                
                if (fileSize >= minSize && (mask.empty() || fileName.find(mask) != std::string::npos)) {
                    bool t = 1;
                    for(int i = 0; i < all_fils.size(); i++){
                        if(all_fils[i] == entry.path().string()){
                            t = 0;
                        }
                    }
                    if(t){
                        all_fils.push_back(entry.path().string());
                    }
                }
            }
        }
    }

    for(int i = 0; i < all_fils.size() ; i++){
        if(FindInFile(all_fils[i], hesh_word, size_block)){
            fils.push_back(all_fils[i]);
            //std::cout << "1" << std::endl;
        }
        //std::cout << std::endl;
    }

    
    for(int i = 0; i < fils.size() ; i++){
        std::cout << fils[i] << std::endl;
    }
}


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "Russian");
    po::options_description desc("Options");

    desc.add_options()
        ("help,h", "help")
        ("directory_list,l", "display a list of directories to scan")
        ("add_directory,a", "add a directory to scan")
        ("del_directory,d", "remove a directory from the list for scanning") 
        ("get_data,g", "get technical information")
        ("set_min_size,m", "set the minimum size for files")
        ("set_scan_level,e", "set the scan level")
        ("set_block_size,b", "install the block frame")      
        ("scanning,s", "scanning directories");   


    std::vector<std::string> directory_list;
    std::vector<int> data;
    
    po::variables_map vm;

    
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

   
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    
    if (vm.count("directory_list")) {
        std::cout << "directory_list: " << std::endl;
        readLinesFromFile(directory_list);
        for(int i = 0; i < directory_list.size(); i++){
            std::cout << "  " << directory_list[i] << std::endl;
        }
    }

    if (vm.count("add_directory")) {
        std::string answer;
        std::cout << "Enter the path: ";
        std::cin >> answer;
        writeLineToFile(answer);
    }

    if (vm.count("del_directory")) {
        std::string answer;
        std::cout << "Enter the path: ";
        std::cin >> answer;
        removeLineFromFile(answer);
    }

    if (vm.count("get_data")) {
        std::cout << "data: " << std::endl;
        readLinesFromDFile(data);
        for(int i = 0; i < data.size(); i++){
            std::cout << "  " << data[i] << std::endl;
        }
    }

    if (vm.count("set_min_size")) {
        int answer;
        std::cout << "set the minimum size: ";
        std::cin >> answer;
        replaceLineInFile(1, answer);
    }

    if (vm.count("set_scan_level")) {
        int answer;
        std::cout << "set the scan level: "; 
        std::cin >> answer;
        replaceLineInFile(0, answer);
    }

    if (vm.count("set_block_size")) {
        int answer;
        std::cout << "set the block size: "; 
        std::cin >> answer;
        replaceLineInFile(2, answer);
    }

    if (vm.count("scanning")) {
        std::string answer, mack, word;
        std::cout << "Do you need to adjust the mask?[y/n] ";
        std::cin >> answer;
        if(answer == "y"){
            std::cout << "install the mask: ";
            std::cin >> mack;
        }
        std::cout << std::endl;
        std::cout << "keep the search element: ";
        std::cin >> word;
        readLinesFromFile(directory_list);
        readLinesFromDFile(data);
        for(int i = 0; i < directory_list.size(); i++){
            listFiles(directory_list[i], data[0], data[1], mack, word, data[2]);
        }
        
    }

    

    return 0;
}