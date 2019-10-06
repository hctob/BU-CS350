#include "IMap.h"

IMap::Chunk IMap::get_chunk(IMap::index_type chunk_num) {
    return reinterpret_cast<Block&>(grouped[chunk_num]);
}

IMap::index_type IMap::next_available_inode() {
    for (index_type i = 0; i < NUM_FILES; i++) if (!data[i]) return i;
    throw std::runtime_error("Disk full of INodes");
}
