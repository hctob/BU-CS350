#include "macros.h"

std::fstream std::bfstream(const char* filename, ios_base::openmode mode) {
    return fstream(filename, ios_base::binary | mode);
}

std::fstream std::bfstream(const std::string& filename, std::ios_base::openmode mode) {
    return bfstream(filename.c_str(), mode);
}

std::fstream std::ibfstream(const std::string& filename, std::ios_base::openmode mode) {
    return bfstream(filename, ios_base::in | mode);
}

std::fstream std::obfstream(const std::string& filename, std::ios_base::openmode mode) {
    return bfstream(filename, ios_base::out | mode);
}
