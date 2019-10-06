#ifndef ASSN05_BLOCK_H
#define ASSN05_BLOCK_H

#include <array>
#include <iostream>
#include "macros.h"

class Block {
    using value_type = constants::byte;

    public:
        using index_type = unsigned short;
        operator char*() { return data.data(); } // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        operator const char*() const { return data.data(); } // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)

        friend std::istream& operator>>(std::istream& is, Block& block);
        friend std::ostream& operator<<(std::ostream& os, const Block& block);

        using size_t = index_type;
        static constexpr size_t SIZE = 1024;
        static constexpr int AS_POINTERS_SIZE = SIZE / sizeof(constants::block_number_t); //AS_POINTERS_SIZE = 512, # entries in imap chunk

        void clear() { data = consolidated_type{}; }
    protected:
        using consolidated_block_nums_type = std::array<constants::block_number_t, AS_POINTERS_SIZE>;
    public:
        static constexpr size_t size() { return SIZE; }
        using consolidated_type = std::array<value_type, SIZE>;
        value_type& operator[](index_type idx) { return data[idx]; };
        const value_type& operator[](index_type idx) const { return data[idx]; };
    protected:
        consolidated_type data{};
};

class Data : public Block {
    public:
        Data() = default;
        Data(const Block& other): Block(other) {}; // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
};

class INode : public Block {
    using file_name_type = char*;
    using incomplete_block_count_type = unsigned short;

    // safe because internally data is contiguous.
    consolidated_block_nums_type& data_as_pointers = reinterpret_cast<consolidated_block_nums_type&>(data);
    static_assert(sizeof(data_as_pointers) == sizeof(data),
                  "Incompatible reinterpretation, std::array no longer contiguous");

    public:
        using index_type = constants::block_count_t;
        using file_size_type = unsigned int;
        static constexpr index_type NUM_DATA_BLOCKS = 128;
        static constexpr file_size_type MAX_FILE_SIZE = NUM_DATA_BLOCKS * Block::SIZE;

        explicit INode(const char* file_name);
        explicit INode(const std::string& file_name): INode(file_name.c_str()) {};
        INode(const Block& other): Block(other) {}; // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        INode(const INode& other): INode(reinterpret_cast<const Block&>(other)) {};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
        constants::block_number_t& operator[](index_type idx) const { return data_as_pointers[idx]; };
        constants::block_number_t& operator[](index_type idx) { return data_as_pointers[idx]; };
#pragma clang diagnostic pop

        incomplete_block_count_type& incomplete_size = data_as_pointers[NUM_DATA_BLOCKS];
        constants::block_count_t& complete_size = reinterpret_cast<constants::block_count_t&>(data[(NUM_DATA_BLOCKS + 1) * 2]);

        file_size_type file_size() const { return complete_size * Block::SIZE + incomplete_size; }
        file_name_type name = reinterpret_cast<file_name_type>(&complete_size + 1);
};

#endif
