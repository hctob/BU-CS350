#ifndef ASSN05_SEGMENT_H
#define ASSN05_SEGMENT_H

#include <array>
#include <ostream>
#include <string>
#include "Block.h"

class Segment {
    class SummaryBlock : public Block {
        public :
            enum META_IDENTIFIER : char { SSB, DATA, INODE, IMAP_CHUNK };
            struct SSB_meta {
                union {
                    constants::inode_number_t inodeNumber;
                    constants::block_number_t chunkNumber;
                } identifier;
                INode::index_type data_index;
                META_IDENTIFIER type;
            };
            SummaryBlock(const Block& other): Block(other) {};
            SummaryBlock(const SummaryBlock& other): SummaryBlock(reinterpret_cast<const Block&>(other)) {};
#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
            SSB_meta& operator[](index_type idx) { return data_as_metadata[idx]; };
            const SSB_meta& operator[](index_type idx) const { return data_as_metadata[idx]; };
#pragma clang diagnostic pop
        private:
            using consolidated_meta_type = std::array<SSB_meta, Block::SIZE / sizeof(SSB_meta)>;
            consolidated_meta_type& data_as_metadata = reinterpret_cast<consolidated_meta_type&>(data);
    };

    using index_type = unsigned short;
    using size_t = index_type;

    using value_type = Block;

    public:
        static constexpr size_t SIZE = 1024;
    private:
        using consolidated_type = std::array<value_type, SIZE>;

        using iterator = consolidated_type::iterator;
        using const_iterator = consolidated_type::const_iterator;
        using reverse_iterator = consolidated_type::reverse_iterator;
        using const_reverse_iterator = consolidated_type::const_reverse_iterator;

        std::string backing_drive;
        constants::segmt_number_t backing_segment = 0;

        static constexpr size_t NUM_SSBS = SIZE / (Block::SIZE / sizeof(SummaryBlock::SSB_meta));

        index_type curr_idx = NUM_SSBS;
        consolidated_type blocks{};
    public:
        static constexpr const char* FILENAME_PREFIX = "SEGMENT";

        explicit Segment(std::string drive_name = "DRIVE");
        Segment(int index, const std::string& drive_name = "DRIVE"); // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        friend std::ostream& operator<<(std::ostream& os, const Segment& segment);
        friend std::istream& operator>>(std::istream& is, Segment& segment);
        value_type& operator[](index_type idx) { return blocks[idx]; };
        const value_type& operator[](index_type idx) const { return blocks[idx]; };

        void flush();
        constants::segmt_number_t get_backing_segment() { return backing_segment; };
        
        constants::block_number_t write_block(const value_type& block, constants::block_number_t& leave_off, bool should_clean = true);
        constants::block_number_t write_data(const value_type& data, constants::inode_number_t corresponding_inode,
                                             constants::block_count_t corresponding_pointer,
                                             constants::block_number_t& leave_off, bool should_clean = true);
        constants::block_number_t write_inode(const value_type& inode, constants::inode_number_t inodeNumber, constants::block_number_t& leave_off, bool should_clean = true);
        constants::block_number_t write_file(std::fstream& file,
                                             const std::string& lfs_filename,
                                             constants::inode_number_t inodeNumber,
                                             constants::block_number_t& leave_off);
        constants::block_number_t write_imap_chunk(const value_type& imap_chunk, index_type chunk_number, constants::block_number_t& leave_off, bool should_clean = true);

        std::fstream backed_ibfstream(const constants::segmt_number_t& segmtNumber, std::ios_base::openmode mode = std::ios_base::in);
        std::fstream backed_obfstream(const constants::segmt_number_t& segmtNumber, std::ios_base::openmode mode = std::ios_base::out);

        // Iterators.
        iterator begin() noexcept { return blocks.begin(); }
        const_iterator begin() const noexcept { return blocks.begin(); }

        iterator end() noexcept { return blocks.end(); }
        const_iterator end() const noexcept { return blocks.end(); }

        reverse_iterator rbegin() noexcept { return blocks.rbegin(); }
        const_reverse_iterator rbegin() const noexcept { return blocks.rbegin(); }

        reverse_iterator rend() noexcept { return blocks.rend(); }
        const_reverse_iterator rend() const noexcept { return blocks.rend(); }

        const_iterator cbegin() const noexcept { return blocks.cbegin(); }
        const_iterator cend() const noexcept { return blocks.cend(); }

        const_reverse_iterator crbegin() const noexcept { return blocks.crbegin(); }
        const_reverse_iterator crend() const noexcept { return blocks.crbegin(); }

        constants::block_number_t update_SSB(constants::block_number_t identifier, INode::index_type data_index,
                                             SummaryBlock::META_IDENTIFIER metaIdentifier,
                                             constants::block_number_t& leave_off);


        void find_backing_segment(constants::block_number_t& leave_off, bool should_clean);

        void clean(constants::segmt_number_t possible_orphans, constants::block_number_t& leave_off);
};

#endif
