#include "cuckoo.h"

#include <sstream>

/*
def location(seed, item):
  '''
  :param seed: a seed of a Murmur hash function
  :param item: an integer
  :return: Murmur_hash(item_left) xor item_right, where item = item_left ||
item_right
  '''
  item_left = item >> output_bits
  item_right = item & mask_of_power_of_2
  hash_item_left = mmh3.hash(str(item_left), seed, signed=False) >> (32 -
output_bits)
  return hash_item_left ^ item_right
*/
uint32_t location(uint32_t seed, uint32_t item) {
  auto item_left = item >> Cuckoo::output_bits_;
  auto item_right = item & ((1 << Cuckoo::output_bits_) - 1);

  std::stringstream ss;
  ss << item_left;
  std::string str = ss.str();
  uint32_t hash_item_left;
  MurmurHash3_x86_32(str.c_str(), str.length(), seed, &hash_item_left);

  return (hash_item_left >> (32 - Cuckoo::output_bits_)) ^ item_right;
}

/*
def left_and_index(item, index):
  '''
  :param item: an integer
  :param index: a log_no_hashes bits integer
  :return: an integer represented as item_left || index
  '''
  return ((item >> (output_bits)) << (log_no_hashes)) + index
*/
uint32_t left_and_index(uint32_t item, uint32_t index) {
  return ((item >> Cuckoo::output_bits_) << Cuckoo::log_no_hashes_) + index;
}

/*
def extract_index(item_left_and_index):
  '''
  :param item_left_and_index: an integer represented as item_left || index
  :return: index extracted
  '''
  return item_left_and_index & (2 ** log_no_hashes - 1)
*/
uint32_t extract_index(uint32_t item_left_and_index) {
  return item_left_and_index & ((1 << Cuckoo::log_no_hashes_) - 1);
}

/*
def reconstruct_item(item_left_and_index, current_location, seed):
  '''
  :param item_left_and_index: an integer represented as item_left || index
  :param current_location: the corresponding location, i.e.
Murmur_hash(item_left) xor item_right :param seed: the seed of the Murmur hash
function :return: the integer item
  '''
  item_left = item_left_and_index >> log_no_hashes
  hashed_item_left = mmh3.hash(str(item_left), seed, signed=False) >> (32 -
output_bits)
  item_right = hashed_item_left ^ current_location
  return (item_left << output_bits) + item_right
*/
uint32_t reconstruct_item(uint32_t item_left_and_index,
                          uint32_t current_location, uint32_t seed) {
  auto item_left = item_left_and_index >> Cuckoo::log_no_hashes_;
  std::stringstream ss;
  ss << item_left;
  std::string str = ss.str();
  uint32_t hashed_item_left;
  MurmurHash3_x86_32(str.c_str(), str.length(), seed, &hashed_item_left);

  auto item_right =
      (hashed_item_left >> (32 - Cuckoo::output_bits_)) ^ current_location;
  return (item_left << Cuckoo::output_bits_) + item_right;
}

/*
def rand_point(bound, i):
  '''
  :param bound: an integer
  :param i: an integer less than bound
  :return: a uniform integer from [0, bound - 1], distinct from i
  '''
  value = randint(0, bound - 1)
  while (value == i):
    value = randint(0, bound - 1)
  return value
*/
uint32_t rand_point(uint32_t bound, uint32_t i) {
  uint32_t value = rand() % bound;
  while (value == i) {
    value = rand() % bound;
  }
  return value;
}

/*
def insert(self, item): #item is an integer
  current_location = location( self.hash_seed[self.insert_index], item)
  current_item = self.data_structure[ current_location]
  self.data_structure[ current_location ] = left_and_index(item,
self.insert_index)

  if (current_item == None):
    self.insert_index = randint(0, number_of_hashes - 1)
    self.depth = 0
  else:
    unwanted_index = extract_index(current_item)
    self.insert_index = rand_point(number_of_hashes, unwanted_index)
    if (self.depth < self.recursion_depth):
      self.depth +=1
      jumping_item = reconstruct_item(current_item, current_location,
self.hash_seed[unwanted_index])
      self.insert(jumping_item)
    else:
      self.FAIL = 1
*/
void Cuckoo::Insert(uint32_t item) {
  auto current_location = location(hash_seeds_[insert_index_], item);
  auto current_item = data_structure_[current_location];

  data_structure_[current_location] = left_and_index(item, insert_index_);

  if (!current_item.has_value()) {
    insert_index_ = rand() % 3;
    depth_ = 0;
  } else {
    auto unwanted_index = extract_index(current_item.value());
    insert_index_ = rand_point(3, unwanted_index);
    if (depth_ < recursion_depth_) {
      depth_++;
      auto jumping_item = reconstruct_item(
          current_item.value(), current_location, hash_seeds_[unwanted_index]);
      Insert(jumping_item);
    } else {
      throw "Failed hashing!";
    }
  }
}
