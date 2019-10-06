#include <cstring>
#include "Block.h"

std::istream& operator>>(std::istream& is, Block& block) {
    if (is.read(block, Block::SIZE).gcount() != Block::SIZE)
        throw std::runtime_error("Could not read a full block");
    return is;
}

std::ostream& operator<<(std::ostream& os, const Block& block) { return os.write(block, Block::SIZE); }

INode::INode(const char* file_name) { std::strncpy(name, file_name, data.end() - name); }
