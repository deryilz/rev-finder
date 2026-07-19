#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "../utils/utils.hpp"

// single threaded for the moment
class Modifier {
    std::vector<std::string> lines {};

public:
    virtual void printHeader() = 0;
    virtual void checkParts(std::vector<std::string> parts) = 0;

    void print(std::string str, bool err = false) {
        (err ? std::cerr : std::cout) << str << std::endl;
    }

    bool openFile(std::string fileName) {
        std::ifstream file(fileName);

        if (!file.is_open()) {
            return false;
        }

        std::string line;
        std::getline(file, line); // ignore header
        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        return true;
    }

    bool startModifying() {
        if (lines.empty()) return false;

        printHeader();

        for (int i=0; i<lines.size(); i++) {
            std::vector<std::string> parts = str::split(lines[i]);

            for (auto &s : parts) {
                if (s.front() == '"') s.erase(0, 1);
                if (s.back() == '"') s.pop_back();
            }

            checkParts(parts);
        }

        return true;
    }
};
