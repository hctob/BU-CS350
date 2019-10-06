#include <fstream>
#include <string>
#include <utility>
#include <vector>
#include <set>
#include "macros.h"
#include "Segment.h"
#include "IMap.h"
#include "Drive.h"

Segment::Segment(std::string drive_name): backing_drive(std::move(drive_name)) {}

Segment::Segment(int index, const std::string& drive_name): Segment(drive_name) {
    std::fstream segment_file = backed_ibfstream(index / SIZE);

    if (segment_file) segment_file >> *this;

    segment_file.close();

    backing_segment = index / SIZE;
    curr_idx = index % SIZE;
}

std::ostream& operator<<(std::ostream& os, const Segment& segment) {
    for (const Segment::value_type& block : segment) os << block;
    return os;
}

std::istream& operator>>(std::istream& is, Segment& segment) {
    Segment::index_type i;
    for (i = 0; i < Segment::SIZE && !is.eof(); i++) is >> segment[i];
    if (i < Segment::SIZE) throw std::runtime_error("Could not read a full segment");
    return is;
}

void Segment::flush() {
    if (backing_segment >= 64) throw std::runtime_error("Disk full");

    std::fstream segment_file = backed_obfstream(backing_segment, std::ios::trunc);
    segment_file << *this;
    segment_file.close();
    blocks = consolidated_type{};
}

constants::block_number_t Segment::write_block(const Segment::value_type& block, constants::block_number_t& leave_off, bool should_clean) {
    if (backing_segment >= 64) throw std::runtime_error("SEGMENTS FULL");
    blocks[curr_idx] = block;
    Drive::segment_dirtiness[backing_segment] = true;

    return leave_off = backing_segment * SIZE + curr_idx++;
}

constants::block_number_t Segment::update_SSB(constants::block_number_t identifier, INode::index_type data_index, Segment::SummaryBlock::META_IDENTIFIER metaIdentifier, constants::block_number_t& leave_off) {
    constants::block_number_t relBlockNumber = leave_off % SIZE;
    constants::block_number_t SSBNumber = relBlockNumber / (Block::SIZE / sizeof(SummaryBlock::SSB_meta));
    constants::block_number_t SSBPair = relBlockNumber % (Block::SIZE / sizeof(SummaryBlock::SSB_meta));
    SummaryBlock as_summary_block = blocks[SSBNumber];
    as_summary_block[SSBPair] = {identifier, data_index, metaIdentifier};
    blocks[SSBNumber] = as_summary_block; // NOLINT(cppcoreguidelines-slicing)
    return leave_off;
}

constants::block_number_t Segment::write_data(
        const Segment::value_type& data,
        constants::inode_number_t corresponding_inode,
        constants::block_count_t corresponding_pointer,
        constants::block_number_t& leave_off,
        bool should_clean) {
    if (curr_idx == SIZE) {
        flush();
        curr_idx = NUM_SSBS;
        find_backing_segment(leave_off, should_clean);
    }
    write_block(data, leave_off, should_clean);
    return update_SSB(corresponding_inode, corresponding_pointer, SummaryBlock::META_IDENTIFIER::DATA, leave_off);
};

constants::block_number_t Segment::write_inode(
        const Segment::value_type& inode,
        constants::inode_number_t inodeNumber,
        constants::block_number_t& leave_off,
        bool should_clean) {
    if (curr_idx == SIZE) {
        flush();
        curr_idx = NUM_SSBS;
        find_backing_segment(leave_off, should_clean);
    }
    write_block(inode, leave_off, should_clean);
    return update_SSB(inodeNumber, 0, SummaryBlock::META_IDENTIFIER::INODE, leave_off);
}

constants::block_number_t Segment::write_imap_chunk(
        const Segment::value_type& imap_chunk,
        IMap::index_type chunk_number,
        constants::block_number_t& leave_off,
        bool should_clean) {
    if (curr_idx == SIZE) {
        flush();
        curr_idx = NUM_SSBS;
        find_backing_segment(leave_off, should_clean);
    }
    write_block(imap_chunk, leave_off, should_clean);
    return update_SSB(chunk_number, 0, SummaryBlock::META_IDENTIFIER::IMAP_CHUNK, leave_off);
}

constants::block_number_t Segment::write_file(
        std::fstream& file,
        const std::string& lfs_filename,
        constants::inode_number_t inodeNumber,
        constants::block_number_t& leave_off) {
    Data curr_data;
    INode node(lfs_filename);

    for (constants::block_count_t i = 0; i < INode::NUM_DATA_BLOCKS && !file.eof(); i++) {
        file.read(curr_data, Block::SIZE);
        node.incomplete_size = file.gcount() % Block::SIZE;
        if (!node.incomplete_size) node.complete_size++;
        node[i] = write_data(curr_data, inodeNumber, i ,leave_off);
        curr_data.clear();
    }

    return write_inode(node, inodeNumber, leave_off);
}

std::fstream Segment::backed_ibfstream(const constants::segmt_number_t& segmtNumber, std::ios_base::openmode mode) {
    return std::ibfstream(backing_drive + "/" + FILENAME_PREFIX + std::to_string(segmtNumber), mode);
}

std::fstream Segment::backed_obfstream(const constants::segmt_number_t& segmtNumber, std::ios_base::openmode mode) {
    return std::obfstream(backing_drive + "/" + FILENAME_PREFIX + std::to_string(segmtNumber), mode);
}

void Segment::find_backing_segment(constants::block_number_t& leave_off, bool should_clean) {
    constants::segmt_number_t possible_orphans = backing_segment;
    for (constants::segmt_number_t i = backing_segment; i < Drive::NUM_SEGMENTS; i++, i %= Drive::NUM_SEGMENTS) {
        if (!Drive::segment_dirtiness[i]) {
            backing_segment = i;
            if (should_clean && Drive::NUM_SEGMENTS - Drive::segment_dirtiness.count() < 9) {
                clean(possible_orphans, leave_off);
            }
            return;
        }
    }
    throw std::runtime_error("Drive full");
}

void Segment::clean(constants::segmt_number_t possible_orphans, constants::block_number_t& leave_off) {
    constants::segmt_number_t j = 0;
    for (constants::segmt_number_t i = 0; i < Drive::NUM_SEGMENTS && j < 32; j++, i++, i %= Drive::NUM_SEGMENTS) {
        if (Drive::segment_dirtiness[i] && backing_segment != i && possible_orphans != i) {
            Segment to_clean(i * SIZE, backing_drive);

            std::map<constants::inode_number_t, Block> inumbers_to_nodes;
            std::vector<std::pair<constants::block_number_t, SummaryBlock::SSB_meta>> valid_data_block_pairs;

            IMap::index_type valid_inodes = 0, valid_imap_chunks = 0;

            std::set<constants::block_number_t> chunks_to_rewrite;
            std::fstream check_file;
            constants::block_number_t inode_block_number;
            INode as_read("");
            for (constants::block_number_t relBlockNumber = 0; relBlockNumber < SIZE; relBlockNumber++) {
                constants::block_number_t blockNumber = to_clean.backing_segment * SIZE + relBlockNumber;
                constants::block_number_t SSBNumber = relBlockNumber / (Block::SIZE / sizeof(SummaryBlock::SSB_meta));
                constants::block_number_t SSBPair = relBlockNumber % (Block::SIZE / sizeof(SummaryBlock::SSB_meta));

                SummaryBlock::SSB_meta metadata = SummaryBlock(to_clean.blocks[SSBNumber])[SSBPair];

                switch (metadata.type) {
                    case SummaryBlock::SSB: break;
                    case SummaryBlock::DATA:
                        inode_block_number = Drive::imap[metadata.identifier.inodeNumber];
                        check_file = backed_ibfstream(inode_block_number / SIZE);
                        check_file.seekg((inode_block_number % Segment::SIZE) * Block::SIZE).read(as_read, Block::SIZE);
                        check_file.close();

                        if (as_read[metadata.data_index] == blockNumber) {
                            valid_data_block_pairs.emplace_back(relBlockNumber, metadata);
                            inumbers_to_nodes.emplace(metadata.identifier.inodeNumber, as_read);
                            chunks_to_rewrite.insert(metadata.identifier.inodeNumber / IMap::Chunk::AS_POINTERS_SIZE);
                        }
                        as_read.clear();
                        break; // CHECK AT EDGE CASE OF 55 SEGMENTS
                    case SummaryBlock::INODE:
                        if (Drive::imap[metadata.identifier.inodeNumber] == blockNumber) {
                            valid_inodes++;
                            inumbers_to_nodes.emplace(metadata.identifier.inodeNumber, to_clean[relBlockNumber]);
                            chunks_to_rewrite.insert(metadata.identifier.inodeNumber / IMap::Chunk::AS_POINTERS_SIZE);
                        }
                        break;
                    case SummaryBlock::IMAP_CHUNK:
                        if (Drive::imap.CHECKPOINT_REGION[metadata.identifier.chunkNumber] == blockNumber) {
                            valid_imap_chunks++;
                            chunks_to_rewrite.insert(metadata.identifier.chunkNumber);
                        }
                        break; // CHECK AT EDGE CASE 55 SEGMENTS
                }
            }
            if (valid_data_block_pairs.size() + valid_inodes + valid_imap_chunks == SIZE - NUM_SSBS)
                continue;


            for (const auto& pair : valid_data_block_pairs) {
                INode temp = inumbers_to_nodes[pair.second.identifier.inodeNumber];
                temp[pair.second.data_index] = write_data(to_clean[pair.first], pair.second.identifier.inodeNumber, pair.second.data_index, leave_off, false);
                inumbers_to_nodes[pair.second.identifier.inodeNumber] = temp;
            }
            for (const auto& pair : inumbers_to_nodes)
                Drive::imap[pair.first] = write_inode(pair.second, pair.first, leave_off, false);
            for (constants::block_number_t chunk_number : chunks_to_rewrite)
                Drive::imap.CHECKPOINT_REGION[chunk_number] = write_imap_chunk(Drive::imap.get_chunk(chunk_number), chunk_number, leave_off, false);

            Drive::segment_dirtiness[i] = false;
        }
    }
}

