#include <emscripten/bind.h>

#include <iostream>
#include <vector>

#include "apsi.h"

using namespace std;
using namespace emscripten;

string encryptQuery() {
  initializeSEAL();

  vector<uint32_t> query;
  query.push_back(123454321);
  query.push_back(222222);
  query.push_back(3213);
  query.push_back(44231);
  query.push_back(515314);
  return hash_and_fhe_encrypt(query);
}

void debugMsg(intptr_t exceptionPtr) {
  cout << std::string(reinterpret_cast<std::exception*>(exceptionPtr)->what())
       << endl;
}

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("encryptQuery", &encryptQuery);
  emscripten::function("processPSI", &process_psi_answer);
  emscripten::function("debugMsg", &debugMsg);
}

int main() { return 0; }
