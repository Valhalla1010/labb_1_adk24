#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>

const std::string index_path= "/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/rawindex.txt";
const std::string korpus_path= "/afs/kth.se/misc/info/kurser/DD2350/adk24/labb1/korpus";
const std::string byte_path= "/home/d/m/dmarzban/Desktop/Labb_1/aa.txt";
const std::string word_path= "/home/d/m/dmarzban/Desktop/Labb_1/b.txt";

// Size of the hash list
const size_t HASH_LIST_SIZE =27000;
std::vector<long> hash_list(HASH_LIST_SIZE,-1);

// Hash function for three-letter words
size_t hash_word(const std::string&word) {
    size_t hash = 0;
    for (char c : word) {
        hash = hash * 31+ c;
    }
    return hash % HASH_LIST_SIZE;
}

// Create byte position file
void byte_position() {
    std::ifstream raw_file(index_path);
    std::ofstream byte_file(byte_path);
    if (!raw_file.is_open() ||
        !byte_file.is_open()) {
        std::cerr << "fel att öppna fil för byte position skapad"<<std::endl;
        return;
    }

    std::string current_word;
    std::vector<std::string>positions;
    std::string word;
    std::string position;

    while (raw_file >> word >>position) {
        if (current_word.empty()) {
            current_word = word;
        }

        if (word == current_word) {
            positions.push_back(position);
        } else {
            if (!positions.empty()) {
                byte_file << current_word <<" ";
                for (const auto& pos: positions) {
                    byte_file << pos << " ";
                }
                byte_file << "\n";
            }
            current_word = word;
            positions = {position};

        }

    }

    if (!positions.empty()) {
        byte_file << current_word <<" ";
        for (const auto& pos: positions) {
            byte_file << pos << " ";
        }

        byte_file << "\n";

    }

}




// Create word position file
void word_posision() {
    std::ifstream byte_file(byte_path);
    std::ofstream pos_file(word_path);
    if (!byte_file.is_open() ||!pos_file.is_open()) {
        std::cerr << "fel att öppna fil för ord position skapad"<< std::endl;
        return;

    }
    std::string line;
    std::string current_word;
    long current_byte_pos = 0;
    while (std::getline(byte_file,line)) {
        current_byte_pos = byte_file.tellg();
        std::string word = line.substr(0,line.find(' '));
        std::string shortened_word = word.substr(0, 3);
        if (shortened_word != current_word)
        {
            pos_file << shortened_word << " " << (current_byte_pos - line.length()) << "\n";
            current_word = shortened_word;
        }
    }
}


// Create hash list
void create_hash_list() {
    std::ifstream file(word_path);
    if (!file.is_open()) {
        std::cerr << "fel att öppna ord position fil"<< std::endl;
        return;
    }

    std::string word;
    long byte_pos;
    while (file >> word >>byte_pos) {
        size_t hash_value = hash_word(word);
        hash_list[hash_value] = byte_pos;
    }
}

// Get hash position
std::vector<long> hash_position(const std::string& word) {
    std::vector<long> ret;
    std::string shortened_word = word.substr(0, 3);
    size_t hash_value = hash_word(shortened_word);
    size_t next_hash = hash_value + 1;
    if (hash_list[hash_value] != -1) {
        ret.push_back(hash_list[hash_value]);
    } else {
        return {-1};
    }

    while (next_hash < HASH_LIST_SIZE && hash_list[next_hash] == -1) {
        ++next_hash;
    }
    if (next_hash < HASH_LIST_SIZE)
    {
        ret.push_back(hash_list[next_hash]);
    } else {
        ret.push_back(-1);
    }
    return ret;
}

std::vector<std::string> binary_search(long byte_pos_first, long byte_pos_last, const std::string& target_word) {
    std::ifstream file(byte_path);
    std::vector<std::string>result;
    if (!file.is_open()) {
        std::cerr << "fel att öppna byte position fil för binärsökning" << std::endl;
        return result;
    }
    file.seekg(byte_pos_first);
    long lower = byte_pos_first;
    long higher = byte_pos_last;
    while (higher - lower > 0) {
        long mid = (lower + higher) / 2;
        file.seekg(mid);
        std::string line;
        std::getline(file, line);
        // Read line at mid
        std::string word = line.substr(0, line.find(' '));
        if (word < target_word) {
            lower = mid + 1;
        } else if (word > target_word) {
            higher = mid;
        } else {
            result.push_back(line.substr(line.find(' ') + 1));
            return result; // Found target
        }
    }
    return {}; // Not found
}

// Search function
std::string search(const std::string& target_word) {
    auto byte_list = hash_position(target_word);
    if (byte_list[0] == -1) {
        return "ordet kan inte hitta!";
    }
    std::ifstream file(word_path);
    std::ifstream korpus_f(korpus_path);
    std::string result;
    if (!file.is_open() || !korpus_f.is_open()) {
        std::cerr << "fel att öppna fil för sökning"<< std::endl;
        return " ";
    }
    file.seekg(byte_list[0]);
    std::string line;
    std::getline(file, line);
    long byte_pos = byte_list[0];
    long next_byte_pos = byte_list[1];
    auto korpus_pos = binary_search(byte_pos, next_byte_pos, target_word);
    if (korpus_pos.empty()) {
        return "ordet kan inte hitta!";
    }
    long position = std::stol(korpus_pos[0]);
    char buffer[31];
    korpus_f.seekg(position - 30);
    korpus_f.read(buffer, 30);
    buffer[30] = '\0';
    result += buffer;
    korpus_f.seekg(position);
    korpus_f.read(buffer, target_word.length());
    buffer[target_word.length()] = '\0';
    result += buffer;
    korpus_f.seekg(position + 30);
    korpus_f.read(buffer, 30);
    buffer[30] = '\0';
    result += buffer;
    return result;
}

int main() {
    int val;
    std::cout << "välja en operation: \n";
    std::cout << "1- skapa byte position \n";
    std::cout << "2- skapa ord position \n";
    std::cout << "3- skapa hash-lista \n";
    std::cout << "4- sök ord \n";
    std::cin>> val;

    switch (val){
        case 1:
            std::cout << "skapa byte position...\n";
            byte_position();
            break;
        case 2:
            std::cout << "skapa ord position... \n";
            word_posision();
            break;
        case 3:
            std::cout << "skapa hash_list.... \n";
            create_hash_list();
            break;
        case 4: {
            std::string target;
            std::cout << "agne ord att söka ";
            std::cin>> target;
            std::string result = search(target);
            std::cout << "sökresultat "<< result << std::endl;
            break;
        }
        default:
            std::cout << "ogiltigt val. Avslutar: \n";
            break;
    }
    return 0;
}
