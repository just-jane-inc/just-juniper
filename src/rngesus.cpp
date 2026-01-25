#include <algorithm>
#include <chrono>
#include <random>

int RNG(int min, int max) {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 engine(seed); // Mersenne Twister 32-bit engine
  std::uniform_int_distribution<int> dist(min, max);
  return dist(engine);
}

std::string RandomChoice(std::vector<std::string> paths) {
  std::random_device rd;
  std::mt19937 generator(rd());
  std::shuffle(paths.begin(), paths.end(), generator);

  return paths[0];
}

bool Flip(double weight) {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 engine(seed);
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  return weight >= dist(engine);
}
