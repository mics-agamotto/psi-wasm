#ifndef _APSI_H_
#define _APSI_H_

#include <cstdint>
#include <string>
#include <vector>

namespace params {
const uint32_t plain_modulus = 536903681;
const uint32_t log2_plain_modulus = 29;
const uint32_t poly_modulus_degree = 1 << 13;
const uint32_t number_of_hashes = 3;
const uint32_t bin_capacity = 536;
const uint32_t alpha = 16;
const uint32_t minibin_capacity = bin_capacity / alpha;
const uint32_t ell = 2;
const uint32_t logB_ell = 3;
const uint32_t base = 1 << ell;
}  // namespace params

void initializeSEAL();
void windowing(std::vector<std::vector<uint32_t>>& vec, uint32_t y,
               uint32_t bound, uint32_t modulus);
std::string hash_and_fhe_encrypt(std::vector<uint32_t>& items);
void process_psi_answer(std::string answer_raw);

#endif  // _APSI_H_
