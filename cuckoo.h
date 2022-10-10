#ifndef _CUCKOO_H_
#define _CUCKOO_H_

#include <cstdint>
#include <optional>
#include <vector>

#include "mmh3.h"

uint32_t reconstruct_item(uint32_t item_left_and_index,
                          uint32_t current_location, uint32_t seed);

class Cuckoo {
 private:
  std::vector<uint32_t> hash_seeds_;
  std::vector<std::optional<uint32_t>> data_structure_;
  uint32_t insert_index_;
  uint32_t depth_ = 0;

 public:
  Cuckoo(std::vector<uint32_t> hash_seeds) : hash_seeds_(hash_seeds) {
    data_structure_.reserve(number_of_bins_);
    for (int i = 0; i < number_of_bins_; i++) data_structure_[i] = {};
    insert_index_ = rand() % 3;
  };

  static const uint32_t output_bits_ = 13;
  static const uint32_t log_no_hashes_ = 2;
  static const uint32_t number_of_bins_ = 1 << output_bits_;
  static const uint32_t recursion_depth_ = 8 * output_bits_;

  void Insert(uint32_t item);
  std::optional<uint32_t> GetItemAt(uint32_t index) {
    return data_structure_[index];
  }
  std::vector<uint32_t> const& GetHashSeeds() { return hash_seeds_; }
};

#endif  // _CUCKOO_H_
