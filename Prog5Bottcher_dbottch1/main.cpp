#include <iostream>
#include <iterator>
#include <sstream>
#include <string.h>
#include <vector>
#include "Drive.h"

int main(int argc, char* argv[]) {
    Drive drive;
    std::string input;

    std::cout << "Available Commands:" << std::endl
              << "import <filename> <lfs_filename>" << std::endl
              << "remove <lfs_filename>" << std::endl
              << "cat <lfs_filename> <howmany> <start>" << std::endl
              << "display <lfs_filename> <howmany> <start>" << std::endl
              << "overwrite <lfs_filename> <howmany> <start> <c>" << std::endl
              << "list" << std::endl
              << "shutdown" << std::endl
              << ">: ";
    while (std::getline(std::cin, input)) {
        std::istringstream iss(input);
        std::vector<std::string> words{std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};

        if (words.empty()) {
            std::cerr << "No command entered" << std::endl;
        } else if (words[0] == "import") {
            if (words.size() == 3) drive.import(words[1], words[2]); // TODO: WHAT IF NO OPEN
            else std::cerr << "`import` has exactly 2 arguments" << std::endl;
        } else if(words[0] == "import_n") {
            if (words.size() == 4) {
                for(int i = 0; i < std::stoi(words[3]); i++) {
                    drive.import(words[1], words[2] + std::to_string(i));
                }
            }
            else std::cerr << "`import` has exactly 3 arguments" << std::endl;
        }
        else if (words[0] == "remove") {
            if (words.size() == 2) drive.remove(words[1]);
            else std::cerr << "`remove` has exactly 1 argument" << std::endl;
        } else if (words[0] == "list") {
            if (words.size() == 1) drive.list();
            else std::cerr << "`list` has no arguments" << std::endl;
        } else if (words[0] == "shutdown") {
            if (words.size() == 1) break;
            else std::cerr << "`shutdown` has no arguments" << std::endl;
        } else if (words[0] == "cat") {
            if (words.size() == 2) drive.cat(words[1]); //have to add cat
            else std::cerr << "`cat` has exactly 1 argument" << std::endl;
        } else if (words[0] == "display") {
            if (words.size() == 4) drive.display(words[1], std::stoi(words[3]), std::stoi(words[2])); //have to add display
            else std::cerr << "`display` has exactly 3 arguments" << std::endl;
        } else if (words[0] == "overwrite") {
            if (words.size() == 5) {
                if (words[4].size() != 1) std::cerr << "Fourth argument to overwrite must be a single character" << std::endl;
                else drive.overwrite(words[1], std::stoi(words[3]), std::stoi(words[2]), *words[4].c_str());
            } else std::cerr << "`overwrite` has exactly 4 arguments" << std::endl;
        } else std::cerr << "Unknown Command" << std::endl;
        std::cout << ">: ";
    }

    return 0;
}
