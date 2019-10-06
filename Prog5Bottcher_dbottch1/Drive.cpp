#include <set>
#include <string.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <iomanip>
#include <vector>
#include "Drive.h"
#include "Block.h"
#include "Segment.h"
#include "macros.h"
#include "IMap.h"

IMap Drive::imap{};
std::bitset<Drive::NUM_SEGMENTS> Drive::segment_dirtiness{};

Drive::Drive(const std::string& directory_path): directory_path(directory_path) {
    struct stat directory_info{};
    if (stat(directory_path.c_str(), &directory_info) == -1) {
        if (errno == ENOENT) {
            if (mkdir(directory_path.c_str(), 0770)) {
                perror("Issue creating directory");
                throw std::runtime_error("Failed to create directory");
            }
        } else {
            perror("Issue accessing directory");
            throw std::runtime_error("Directory exists but could not access");
        }
    }

    std::fstream backed_name_map = backed_ibfstream("NAME_MAP");
    if (backed_name_map) {
        std::pair<name_map_type::key_type, name_map_type::mapped_type> new_pair;
        for (int i = 0; i < IMap::NUM_FILES && !backed_name_map.eof(); i++) {
            backed_name_map >> new_pair.first >> new_pair.second;
            name_map.insert(new_pair);
        }
        if (!backed_name_map.eof()) throw std::runtime_error("File name map not entirely loaded");
        backed_name_map.close();
    } else backed_obfstream("NAME_MAP", std::ios::trunc).close();

    std::fstream checkpoint_region = backed_ibfstream("CHECKPOINT_REGION");
    if (checkpoint_region) {
        checkpoint_region.read(reinterpret_cast<char*>(&imap.leave_off), sizeof(constants::block_number_t));
        if (checkpoint_region.gcount() != sizeof(constants::block_number_t) || (imap.leave_off % Segment::SIZE) < 4) {
            throw std::runtime_error("Could not read/invalid leave off point.");
        }

        unsigned long dirtiness_dump;
        checkpoint_region.read(reinterpret_cast<char*>(&dirtiness_dump), sizeof(unsigned long));
        if (checkpoint_region.gcount() != sizeof(unsigned long)) {
            throw std::runtime_error("Could not read segment dirtiness");
        }
        segment_dirtiness = dirtiness_dump;

        checkpoint_region.read(imap.CHECKPOINT_REGION_AS_BYTES.data(), imap.CHECKPOINT_REGION_AS_BYTES.size());
        if (checkpoint_region.gcount() != imap.CHECKPOINT_REGION_AS_BYTES.size()) {
            throw std::runtime_error("Could not load CR");
        }

        std::set<constants::segmt_number_t> segments_to_load;


        std::vector<constants::block_number_t> imap_chunk_pointers;
        std::map<constants::block_number_t, Block> block_pointers_to_blocks;
        std::map<constants::segmt_number_t, std::vector<constants::block_number_t>> segmts_to_block_nums;


        for (constants::block_number_t i : imap.CHECKPOINT_REGION) {
            imap_chunk_pointers.push_back(i);
            if (i) segmts_to_block_nums[i / Segment::SIZE].push_back(i);
        }

        Block as_read;
        for (const auto& pair: segmts_to_block_nums) {
            std::fstream segment_file = memory_segment.backed_ibfstream(pair.first);
            for (const constants::block_number_t& blockNumber: pair.second) {
                segment_file.seekg((blockNumber % Segment::SIZE) * Block::SIZE).read(as_read, Block::SIZE);
                block_pointers_to_blocks[blockNumber] = as_read;
                as_read.clear();
            }
            segment_file.close();
        }

        for (int i = 0; i < imap.grouped.size(); i++) {
            if (imap_chunk_pointers[i])
                imap.grouped[i] = reinterpret_cast<IMap::subgroup_type&>(block_pointers_to_blocks[imap_chunk_pointers[i]]);
        }

        memory_segment = Segment(imap.leave_off, directory_path);
        imap.leave_off--;

        if (checkpoint_region.tellg() != imap.CHECKPOINT_REGION_AS_BYTES.size() + sizeof(imap.leave_off) + sizeof(unsigned long)) { // reading bytes does not read past eof
            std::cerr << "Checkpoint region not entirely loaded." << checkpoint_region.tellg() << std::endl;
        }
    } else {
        checkpoint_region = backed_obfstream("CHECKPOINT_REGION", std::ios::trunc);
        checkpoint_region.write(reinterpret_cast<const char*>(&imap.leave_off), sizeof(constants::block_number_t));
        checkpoint_region.write(imap.CHECKPOINT_REGION_AS_BYTES.data(), imap.CHECKPOINT_REGION_AS_BYTES.size());
        checkpoint_region.close();

        for (int i = 0; i/Segment::SIZE < NUM_SEGMENTS; i += Segment::SIZE)
            Segment(i, directory_path).flush();

    }
}

Drive::~Drive() {
    std::cout << "Shutting down..." << std::endl;
    std::fstream backed_name_map = backed_obfstream("NAME_MAP", std::ios::trunc);
    std::fstream checkpoint_region = backed_obfstream("CHECKPOINT_REGION", std::ios::trunc);

    for (name_map_type::value_type& entry: name_map) backed_name_map << entry.first << " " << entry.second << std::endl;
    imap.leave_off++;
    unsigned long dirtiness_dump = segment_dirtiness.to_ulong();
    checkpoint_region.write(reinterpret_cast<const char*>(&imap.leave_off), sizeof(constants::block_number_t));
    checkpoint_region.write(reinterpret_cast<const char*>(&dirtiness_dump), sizeof(unsigned long));
    checkpoint_region.write(imap.CHECKPOINT_REGION_AS_BYTES.data(), imap.CHECKPOINT_REGION_AS_BYTES.size());

    memory_segment.flush();
}


void Drive::import(std::string& filename, const std::string& lfs_filename) {
    std::fstream file = std::ibfstream(filename);
    if (!file) {
        std::cerr << "Could not open file: " << std::quoted(filename) << std::endl;
        return;
    }
    IMap::index_type node_number = imap.next_available_inode();
    name_map[lfs_filename] = node_number; // CHECK AT SEGMENT 55
    IMap::index_type chunk_number = node_number / IMap::Chunk::AS_POINTERS_SIZE;
    imap[node_number] = memory_segment.write_file(file, lfs_filename, node_number, imap.leave_off);
    imap.CHECKPOINT_REGION[chunk_number] = memory_segment.write_imap_chunk(imap.get_chunk(chunk_number), chunk_number, imap.leave_off);
    std::cout << std::quoted(lfs_filename) << " imported from file " << std::quoted(filename) <<  std::endl;
}

void Drive::remove(std::string& lfs_filename) {
    if (name_map.find(lfs_filename) == name_map.end()) {
        std::cerr << "Cannot remove " << std::quoted(lfs_filename) << " as it does not exists" << std::endl;
        return;
    }
    IMap::index_type node_number = name_map[lfs_filename];
    IMap::index_type chunk_number = node_number / IMap::Chunk::AS_POINTERS_SIZE;
    name_map.erase(lfs_filename);
    imap[node_number] = 0;
    imap.CHECKPOINT_REGION[chunk_number] = memory_segment.write_imap_chunk(imap.get_chunk(chunk_number), chunk_number, imap.leave_off);
    std::cout << std::quoted(lfs_filename) << " removed" << std::endl;
}

void Drive::list() {
    constants::segmt_number_t memory_segment_num = memory_segment.get_backing_segment();

    std::vector<constants::block_number_t> inode_pointers;
    std::map<constants::block_number_t, Block> block_pointers_to_blocks;
    std::map<constants::segmt_number_t, std::vector<constants::block_number_t>> segmts_to_block_nums;
    std::map<name_map_type::key_type, INode::file_size_type> names_to_sizes;

    for (const constants::block_number_t& inodePointer : imap) {
        if (inodePointer) {
            inode_pointers.push_back(inodePointer);
            segmts_to_block_nums[inodePointer / Segment::SIZE].push_back(inodePointer);
        }
    }

    if (segmts_to_block_nums.find(memory_segment_num) != segmts_to_block_nums.end()) {
        for (const constants::block_number_t& blockNumber: segmts_to_block_nums[memory_segment_num])
            block_pointers_to_blocks[blockNumber] = memory_segment[blockNumber % Segment::SIZE];
        segmts_to_block_nums.erase(memory_segment_num);
    }

    Block as_read;
    for (const auto& pair: segmts_to_block_nums) {
        std::fstream segment_file = memory_segment.backed_ibfstream(pair.first);
        for (const constants::block_number_t& blockNumber: pair.second) {
            segment_file.seekg((blockNumber % Segment::SIZE) * Block::SIZE).read(as_read, Block::SIZE);
            block_pointers_to_blocks[blockNumber] = as_read;
            as_read.clear();
        }
        segment_file.close();
    }

    for (const auto& pair: block_pointers_to_blocks)
        names_to_sizes[INode(pair.second).name] = INode(pair.second).file_size();

    for (auto& pair : names_to_sizes)
        if (pair.second != 0) // TODO: <no visble file name> \t size = 0 bytes is displayed
            std::cout << pair.first << ":\t\t" << pair.second << " B" << std::endl;
}


void Drive::cat(std::string& lfs_filename) { display(lfs_filename, 0, IMap::NUM_FILES * Block::SIZE); }

void Drive::display(INode node, INode::file_size_type start, INode::file_size_type how_many) {
    if (start > node.file_size()) return;

    constants::block_count_t firstDataBlock = start / Block::SIZE;
    Block::size_t relStart = start % Block::SIZE;
    constants::segmt_number_t memory_segment_num = memory_segment.get_backing_segment();

    how_many = std::min(how_many, node.file_size() - start);
    constants::block_count_t lastDataBlock = (how_many + start) / Block::SIZE;

    std::vector<constants::block_number_t> data_block_pointers;
    std::map<constants::block_number_t, Data> block_pointers_to_blocks;
    std::map<constants::segmt_number_t, std::vector<constants::block_number_t>> segmts_to_block_nums;

    for (constants::block_count_t i = firstDataBlock; i <= lastDataBlock; i++) { // TODO: implement range for loop
        data_block_pointers.push_back(node[i]);
        segmts_to_block_nums[node[i] / Segment::SIZE].push_back(node[i]);
    }

    if (segmts_to_block_nums.find(memory_segment_num) != segmts_to_block_nums.end()) {
        for (const constants::block_number_t& blockNumber: segmts_to_block_nums[memory_segment_num])
            block_pointers_to_blocks[blockNumber] = memory_segment[blockNumber % Segment::SIZE];
        segmts_to_block_nums.erase(memory_segment_num);
    }

    Block as_read;
    for (const auto& pair: segmts_to_block_nums) {
        std::fstream segment_file = memory_segment.backed_ibfstream(pair.first);
        for (const constants::block_number_t& blockNumber: pair.second) {
            segment_file.seekg((blockNumber % Segment::SIZE) * Block::SIZE).read(as_read, Block::SIZE);
            block_pointers_to_blocks[blockNumber] = as_read;
            as_read.clear();
        }
        segment_file.close();
    }

    for (std::size_t i = 0; i < data_block_pointers.size() - 1; i++)
        std::cout << block_pointers_to_blocks[data_block_pointers[i]];

    std::cout.write(block_pointers_to_blocks[data_block_pointers.back()], how_many % Block::SIZE);
    std::cout << std::endl;
}

void Drive::display(std::string& lfs_filename, unsigned int start, unsigned int how_many) { // TODO: new type
    if (name_map.find(lfs_filename) == name_map.end()) {
        std::cerr << std::quoted(lfs_filename) << " does not exist" << std::endl;
        return;
    }

    if (!how_many || start / Block::SIZE >= INode::NUM_DATA_BLOCKS) return;

    constants::inode_number_t inodeNumber = name_map[lfs_filename];
    constants::block_number_t blockNumber = imap[inodeNumber];
    constants::segmt_number_t segmtNumber = blockNumber / Segment::SIZE;
    constants::block_number_t relBlockNumber = blockNumber % Segment::SIZE;

    if (segmtNumber == memory_segment.get_backing_segment())
        return display(memory_segment[relBlockNumber], start, how_many);
    else {
        Block as_read;
        std::fstream segment_file = memory_segment.backed_ibfstream(segmtNumber);
        segment_file.seekg(relBlockNumber * Block::SIZE).read(as_read, Block::SIZE);
        segment_file.close();
        return display(as_read, start, how_many);
    }
}

void Drive::overwrite(INode node, constants::inode_number_t inodeNumber, INode::file_size_type start, INode::file_size_type how_many, char c) {
    if (start >= node.file_size()) {
        std::cerr << "File " << std::quoted(node.name) << " does not have a valid last byte at " << start;
        return;
    }

    if (!(how_many = std::min(how_many, INode::MAX_FILE_SIZE - start))) return;

    constants::segmt_number_t memory_segment_num = memory_segment.get_backing_segment();
    constants::block_count_t currDataBlock = start / Block::SIZE;
    Block::size_t relStart = start % Block::SIZE;

    INode::file_size_type relative_how_many = std::min<INode::file_size_type>(how_many, Block::SIZE - relStart);


    INode copy = node;
    // should this be detected and modified when writing?
    if (start + how_many > node.file_size()) {
        copy.complete_size = (start + how_many) / Block::SIZE;
        copy.incomplete_size = (start + how_many) % Block::SIZE;
    }
    how_many -= relative_how_many;
    Block::size_t partial = how_many % Block::SIZE;

    Data to_fill;

    if (relative_how_many) {
        constants::block_number_t firstDataBlockNumber = node[currDataBlock];
        constants::segmt_number_t firstDataBlockSegmt = firstDataBlockNumber / Segment::SIZE;
        constants::block_number_t relFirstDataBlockNumber = firstDataBlockNumber % Segment::SIZE;
        if (memory_segment_num == firstDataBlockSegmt) to_fill = memory_segment[relFirstDataBlockNumber];
        else {
            std::fstream segment_file = memory_segment.backed_ibfstream(firstDataBlockSegmt);
            segment_file.seekg((relFirstDataBlockNumber % Segment::SIZE) * Block::SIZE).read(to_fill, Block::SIZE);
            segment_file.close();
        }
        std::fill_n(to_fill + relStart, relative_how_many, c);
        copy[currDataBlock] = memory_segment.write_data(to_fill, inodeNumber, currDataBlock, imap.leave_off);
        currDataBlock++;
    }

    if (how_many / Block::SIZE) std::fill_n<char*>(to_fill, Block::SIZE, c);

    for (int i = 0; i < how_many / Block::SIZE; i++, currDataBlock++)
        copy[currDataBlock] = memory_segment.write_data(to_fill, inodeNumber, currDataBlock, imap.leave_off);

    if (partial) {
        constants::block_number_t lastDataBlockNumber = node[currDataBlock];
        constants::segmt_number_t lastDataBlockSegmt = lastDataBlockNumber / Segment::SIZE;
        constants::block_number_t relLastDataBlockNumber = lastDataBlockNumber % Segment::SIZE;
        if (memory_segment_num == lastDataBlockSegmt) to_fill = memory_segment[relLastDataBlockNumber];
        else {
            std::fstream segment_file = memory_segment.backed_ibfstream(lastDataBlockSegmt);
            segment_file.seekg((relLastDataBlockNumber % Segment::SIZE) * Block::SIZE).read(to_fill, Block::SIZE);
            segment_file.close();
        }
        std::fill_n<char*>(to_fill, partial, c);
        copy[currDataBlock] = memory_segment.write_data(to_fill, inodeNumber, currDataBlock, imap.leave_off);
    }

    IMap::index_type node_number = name_map[node.name];
    IMap::index_type chunk_number = node_number / IMap::Chunk::AS_POINTERS_SIZE;

    imap[node_number] = memory_segment.write_inode(copy, inodeNumber, imap.leave_off);
    imap.CHECKPOINT_REGION[chunk_number] = memory_segment.write_imap_chunk(imap.get_chunk(chunk_number), chunk_number, imap.leave_off);
}

void Drive::overwrite(std::string& lfs_filename, INode::file_size_type start, INode::file_size_type how_many, char c) {
    if (name_map.find(lfs_filename) == name_map.end()) {
        std::cerr << std::quoted(lfs_filename) << " does not exist" << std::endl;
        return;
    }
    if (start / Block::SIZE >= INode::NUM_DATA_BLOCKS) {
        std::cerr << "File " << std::quoted(lfs_filename) << " does not have a valid last byte at " << start;
        return;
    }

    constants::inode_number_t inodeNumber = name_map[lfs_filename];
    constants::block_number_t blockNumber = imap[inodeNumber];
    constants::segmt_number_t segmtNumber = blockNumber / Segment::SIZE;
    constants::block_number_t relBlockNumber = blockNumber % Segment::SIZE;

    if (segmtNumber == memory_segment.get_backing_segment())
        return overwrite(memory_segment[relBlockNumber], inodeNumber, start, how_many, c);
    else {
        Block as_read;
        std::fstream segment_file = memory_segment.backed_ibfstream(segmtNumber);
        segment_file.seekg(relBlockNumber * Block::SIZE).read(as_read, Block::SIZE);
        segment_file.close();
        return overwrite(as_read, inodeNumber, start, how_many, c);
    }
}

std::fstream Drive::backed_ibfstream(const std::string& backed_name, std::ios_base::openmode mode) {
    return std::ibfstream(directory_path + "/" + backed_name, mode);
}

std::fstream Drive::backed_obfstream(const std::string& backed_name, std::ios_base::openmode mode) {
    return std::obfstream(directory_path + "/" + backed_name, mode);
}
