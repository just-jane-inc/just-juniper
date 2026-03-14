#include <algorithm>
#include <chrono>
#include <random>
#include <stdexcept>
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 generator(seed);

// This function is sponsored by: B.A.H.M.S. Inc. please treat
// it with the respect it deserves. - Meisaka
int RNG(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  return dist(generator);
}

std::string RandomChoice(std::vector<std::string> paths) {
  std::shuffle(paths.begin(), paths.end(), generator);
  return paths[0];
}

// i am just curious - diabloproject
bool Flip(double weight) {
  if (weight > 1.0 || weight < 0.0) {
    throw std::out_of_range("weight must be between 0.0 and 1.0");
  }

  std::uniform_real_distribution<double> dist(0.0, 1.0);
  return weight >= dist(generator);
}
