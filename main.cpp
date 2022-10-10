#include <emscripten/bind.h>

#include <iostream>
#include <vector>

#include "apsi.h"
#include "json.h"

using namespace std;
using namespace emscripten;

string encryptQuery(string query_raw) {
  vector<uint32_t> items = json::jobject::parse(query_raw)["items"];
  cout << "Received query in plaintext" << endl;
  for (auto item : items) {
    cout << "Item: " << item << endl;
  }
  return hash_and_fhe_encrypt(items);
}

void debugMsg(intptr_t exceptionPtr) {
  cout << std::string(reinterpret_cast<std::exception*>(exceptionPtr)->what())
       << endl;
}

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("initializeSEAL", &initializeSEAL);
  emscripten::function("encryptQuery", &encryptQuery);
  emscripten::function("processPSI", &process_psi_answer);
  emscripten::function("debugMsg", &debugMsg);
}

int main() { return 0; }
