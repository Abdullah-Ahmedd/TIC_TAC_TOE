#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "include/nlohmann/json.hpp"
//#include "json/json.hpp"  // Correct path where json.hpp is located


using json = nlohmann::json;

int main() {
    std::vector<std::string> names;
    std::string name;
    char choice;

    std::cout << "Enter names (type 'q' to quit):\n";
    while (true) {
        std::cout << "Enter name: ";
        std::getline(std::cin, name);
        if (name == "q") break;
        names.push_back(name);
    }

    // Create a JSON object
    json j;
    j["names"] = names;

    // Save to a JSON file
    std::ofstream file("names.json");
    if (file.is_open()) {
        file << j.dump(4); // Pretty print with 4 spaces
        file.close();
        std::cout << "Names saved to 'names.json'.\n";
    } else {
        std::cerr << "Error: Could not open file for writing.\n";
    }

    return 0;
}