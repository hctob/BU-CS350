#ifndef ASSN05_MACROS_H
#define ASSN05_MACROS_H

#include <fstream>

namespace std { // NOLINT(cert-dcl58-cpp)
    fstream bfstream(const char* filename, ios_base::openmode mode = ios_base::in | ios_base::out);;
    fstream bfstream(const string& filename, ios_base::openmode mode = ios_base::in | ios_base::out);;
    fstream ibfstream(const string& filename, ios_base::openmode mode = ios_base::in);;
    fstream obfstream(const string& filename, ios_base::openmode mode = ios_base::out);;
}

namespace constants {
    using byte = char;
    using block_number_t = unsigned short;
    using inode_number_t = unsigned short;
    using block_count_t = unsigned char;
    using segmt_number_t = unsigned char;
}

#endif
