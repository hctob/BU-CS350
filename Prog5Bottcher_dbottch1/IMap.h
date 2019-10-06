#ifndef ASSN05_IMAP_H
#define ASSN05_IMAP_H

#include <array>
#include "Block.h"

class IMap {
    public:
        using index_type = constants::inode_number_t;
        static constexpr index_type NUM_FILES = 10 * 1024;
        using value_type = constants::block_number_t;
        using consolidated_type = std::array<value_type, NUM_FILES>;

        using iterator = consolidated_type::iterator;
        using const_iterator = consolidated_type::const_iterator;
        using reverse_iterator = consolidated_type::reverse_iterator;
        using const_reverse_iterator = consolidated_type::const_reverse_iterator;

        value_type leave_off = 4;
        index_type next_available_inode();

        value_type& operator[](index_type idx) { return data[idx]; };
        const value_type& operator[](index_type idx) const { return data[idx]; };


        class Chunk : public Block {
            public:
                Chunk(const Block& other): Block(other) {}; // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        };

        //chunk containers
        static constexpr index_type NUM_CHUNKS = NUM_FILES / Chunk::AS_POINTERS_SIZE;
        using subgroup_type = std::array<value_type, Chunk::AS_POINTERS_SIZE>;
        using grouped_type = std::array<subgroup_type, NUM_CHUNKS>;

        std::array<value_type, NUM_CHUNKS> CHECKPOINT_REGION{};
        Chunk get_chunk(index_type chunk_num);

        //representation of the checkpoint region in bytes
        using CR_AS_BYTES_TYPE = std::array<char, NUM_CHUNKS * sizeof(index_type) / sizeof(char)>;
        CR_AS_BYTES_TYPE& CHECKPOINT_REGION_AS_BYTES = reinterpret_cast<CR_AS_BYTES_TYPE&>(CHECKPOINT_REGION);

        consolidated_type data{};
        grouped_type& grouped = reinterpret_cast<grouped_type&>(data);
        static_assert(sizeof(grouped) == sizeof(data),
                      "Reinterpret cast failed, std::array not contiguous");

        // Iterators.
        iterator begin() noexcept { return data.begin(); }
        const_iterator begin() const noexcept { return data.begin(); }
    
        iterator end() noexcept { return data.end(); }
        const_iterator end() const noexcept { return data.end(); }
    
        reverse_iterator rbegin() noexcept { return data.rbegin(); }
        const_reverse_iterator rbegin() const noexcept { return data.rbegin(); }
    
        reverse_iterator rend() noexcept { return data.rend(); }
        const_reverse_iterator rend() const noexcept { return data.rend(); }
    
        const_iterator cbegin() const noexcept { return data.cbegin(); }
        const_iterator cend() const noexcept { return data.cend(); }
    
        const_reverse_iterator crbegin() const noexcept { return data.crbegin(); }
        const_reverse_iterator crend() const noexcept { return data.crbegin(); }

};

#endif
