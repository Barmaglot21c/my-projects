// #define DEBUG

#ifdef DEBUG
#include <chrono>
#include <string>
#endif

#include <iostream>

#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

using namespace std;

int main() {
#ifdef DEBUG
  auto start = std::chrono::high_resolution_clock::now();
#endif

  TransportCatalogue catalogue;

  JsonReader reader(catalogue, cin);
  reader.FillBase();

#ifdef DEBUG
  auto fill_end = std::chrono::high_resolution_clock::now();
#endif

  reader.PrintStat(cout);

#ifdef DEBUG
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  auto fill_duration = std::chrono::duration_cast<std::chrono::milliseconds>(fill_end - start);
  auto print_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - fill_end);

  cerr << "Fill time: " << fill_duration << endl;
  cerr << "Print time: " << print_duration << endl;
  cerr << "Total time: " << duration << endl;
#endif
}