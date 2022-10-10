#include "apsi.h"

#include <iostream>
#include <sstream>

#include "base64.h"
#include "cuckoo.h"
#include "json.h"
#include "seal/seal.h"

using std::vector;
using namespace seal;

/* Iterative Function to calculate (x^y)%p in O(log y) */
static int power_mod(long long x, unsigned int y, int p)
{
  int res = 1; // Initialize result

  x = x % p; // Update x if it is more than or
             // equal to p

  if (x == 0)
    return 0; // In case x is divisible by p;

  while (y > 0)
  {
    // If y is odd, multiply x with result
    if (y & 1)
      res = (res * x) % p;

    // y must be even now
    y = y >> 1; // y = y/2
    x = (x * x) % p;
  }
  return res;
}

/*
def windowing(y, bound, modulus):
  '''
  :param: y: an integer
  :param bound: an integer
  :param modulus: a modulus integer
  :return: a matrix associated to y, where we put y ** (i+1)*base ** j mod
modulus in the (i,j) entry, as long as the exponent of y is smaller than some
bound
  '''
  windowed_y = [[None for j in range(logB_ell)] for i in range(base-1)]
  for j in range(logB_ell):
      for i in range(base-1):
          if ((i+1) * base ** j - 1 < bound):
              windowed_y[i][j] = pow(y, (i+1) * base ** j, modulus)
  return windowed_y
*/
void windowing(vector<vector<uint32_t>> &vec, uint32_t y, uint32_t bound,
               uint32_t modulus)
{
  for (int i = 0; i < params::base - 1; i++)
  {
    vector<uint32_t> subvec;
    for (int j = 0; j < params::logB_ell; j++)
    {
      subvec.push_back(0);
    }
    vec.push_back(subvec);
  }
  for (int i = 0; i < params::base - 1; i++)
  {
    for (int j = 0; j < params::logB_ell; j++)
    {
      int val = (i + 1) * power_mod(params::base, j, modulus);
      if (val - 1 < bound)
      {
        vec[i][j] = power_mod(y, val, modulus);
      }
    }
  }
}

namespace apsi
{
  EncryptionParameters *parms;
  SEALContext *ctx;
  KeyGenerator *keygen;
  PublicKey *pk;
  Cuckoo *cuckoo;
} // namespace apsi

void initializeSEAL()
{
  if (apsi::ctx != nullptr)
  {
    return;
  }

  apsi::parms = new EncryptionParameters(scheme_type::bfv);
  size_t poly_modulus_degree = 8192;
  apsi::parms->set_poly_modulus_degree(poly_modulus_degree);
  apsi::parms->set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
  apsi::parms->set_plain_modulus(536903681);
  apsi::ctx = new SEALContext(*apsi::parms);

  apsi::keygen = new KeyGenerator(*apsi::ctx);
  apsi::pk = new PublicKey();
  apsi::keygen->create_public_key(*apsi::pk);

  vector<uint32_t> seeds;
  seeds.push_back(123456789);
  seeds.push_back(987654320);
  seeds.push_back(132435469);
  apsi::cuckoo = new Cuckoo(seeds);
}

std::string hash_and_fhe_encrypt(vector<uint32_t> &items)
{
  for (auto item : items)
  {
    std::cout << "Inserting " << item << std::endl;
    apsi::cuckoo->Insert(item);
  }

  vector<vector<vector<uint32_t>>> windowed_items;
  for (int i = 0; i < Cuckoo::number_of_bins_; i++)
  {
    auto item = apsi::cuckoo->GetItemAt(i);
    auto item_val = item.value_or(1 << params::log2_plain_modulus);
    vector<vector<uint32_t>> window;
    windowing(window, item_val, params::minibin_capacity,
              params::plain_modulus);
    windowed_items.push_back(window);
  }

  BatchEncoder encoder(*apsi::ctx);
  Encryptor encryptor(*apsi::ctx, *apsi::pk);

  json::jobject serialized_ciphertexts;
  for (int i = 0; i < params::base - 1; i++)
  {
    for (int j = 0; j < params::logB_ell; j++)
    {
      int val = (i + 1) * power_mod(params::base, j, params::plain_modulus);
      if (val - 1 < params::minibin_capacity)
      {
        vector<int64_t> plain_query;
        for (int k = 0; k < windowed_items.size(); k++)
        {
          plain_query.push_back(windowed_items[k][i][j]);
        }

        Plaintext query;
        encoder.encode(plain_query, query);

        Ciphertext enc_query;
        encryptor.encrypt(query, enc_query);

        std::stringstream stream;
        enc_query.save(stream);

        std::stringstream key;
        key << "ct_" << i << "_" << j;
        serialized_ciphertexts[key.str()] = base64_encode(stream.str());
      }
    }
  }

  std::stringstream parm_stream;
  apsi::parms->save(parm_stream);
  serialized_ciphertexts["parms"] = base64_encode(parm_stream.str());

  std::stringstream pk_stream;
  apsi::pk->save(pk_stream);
  serialized_ciphertexts["pk"] = base64_encode(pk_stream.str());

  std::stringstream sk_stream;
  apsi::keygen->secret_key().save(sk_stream);
  serialized_ciphertexts["sk"] = base64_encode(sk_stream.str());

  return serialized_ciphertexts.as_string();
}

std::string process_psi_answer(std::string answer_raw)
{
  std::cout << "process_psi_answer" << std::endl;

  auto answer = json::jobject::parse(answer_raw);
  vector<std::string> cts = answer["ciphertexts"];

  Decryptor decryptor(*apsi::ctx, apsi::keygen->secret_key());
  BatchEncoder encoder(*apsi::ctx);

  vector<vector<uint64_t>> plaintexts;
  for (auto ct_encoded : cts)
  {
    std::stringstream b64_stream;
    b64_stream << '\"' << ct_encoded << '\"';
    auto ct_encoded_unescaped =
        json::parsing::unescape_characters(b64_stream.str().c_str());

    std::stringstream stream;
    stream << base64_decode(ct_encoded_unescaped);

    Ciphertext ct;
    ct.load(*apsi::ctx, stream);

    Plaintext pt;
    decryptor.decrypt(ct, pt);

    vector<uint64_t> vec;
    encoder.decode(pt, vec);
    plaintexts.push_back(vec);
  }

  std::cout << "decrypted answers" << std::endl;

  vector<uint32_t> found_items;
  for (int i = 0; i < params::alpha; i++)
  {
    for (int j = 0; j < params::poly_modulus_degree; j++)
    {
      if (plaintexts[i][j] == 0)
      {
        auto item = apsi::cuckoo->GetItemAt(j).value();
        auto common_element = reconstruct_item(
            item, j,
            apsi::cuckoo->GetHashSeeds()[item % (1 << Cuckoo::log_no_hashes_)]);
        found_items.push_back(common_element);
      }
    }
  }

  json::jobject serialized_resp;
  serialized_resp["items"] = found_items;

  return serialized_resp.as_string();
}
