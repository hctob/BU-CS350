#ifndef ASSN05_DRIVE_H
#define ASSN05_DRIVE_H

#include <bitset>
#include <map>
#include <string.h>
#include "Block.h"
#include "Segment.h"
#include "IMap.h"

class Drive {
    friend Segment;
    using index_type = unsigned short;
    using name_map_type = std::map<std::string, index_type>;
    const std::string& directory_path;
    Segment memory_segment;
    name_map_type name_map;
    static IMap imap;

    public:
        static constexpr constants::segmt_number_t NUM_SEGMENTS = 64;
        Drive(const std::string& directory_path = "DRIVE"); // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        void import(std::string& filename, const std::string& lfs_filename);
        void remove(std::string& lfs_filename);
        void list();
        void cat(std::string& lfs_filename);
        void display(std::string& lfs_filename, INode::file_size_type start, INode::file_size_type how_many);
        void display(INode node, INode::file_size_type start, INode::file_size_type how_many);
        void overwrite(std::string& lfs_filename, INode::file_size_type start, INode::file_size_type how_many, char c);
        void overwrite(INode node, constants::inode_number_t inodeNumber, INode::file_size_type start,
                       INode::file_size_type how_many, char c);

        std::fstream backed_ibfstream(const std::string& backed_name, std::ios_base::openmode mode = std::ios_base::in);
        std::fstream backed_obfstream(const std::string& backed_name, std::ios_base::openmode mode = std::ios_base::out);
        ~Drive();
    private:
        static std::bitset<NUM_SEGMENTS> segment_dirtiness;

};

#endif
