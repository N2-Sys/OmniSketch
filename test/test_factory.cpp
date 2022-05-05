/**
 * @file test_factory.cpp
 * @author dromniscience (you@domain.com)
 * @brief Main routine of the test factory
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "test_factory.h"

/**
 * @cond TEST
 * @brief Global testing configs
 *
 */
#define DEFAULT_REPEAT_TIMES 5
#define DEFAULT_RANDOM_SEED 0U

int g_repeat = DEFAULT_REPEAT_TIMES;
unsigned g_seed = DEFAULT_RANDOM_SEED;
bool g_success = true;
std::string_view g_test_name = "";
/** @endcond */

/**
 * @cond TEST
 * @brief Main routine
 */
int main(int argc, char *argv[]) {
  bool need_help = false;

  option options[] = {{"repeat", required_argument, nullptr, 'r'},
                      {"seed", required_argument, nullptr, 's'},
                      {"help", no_argument, nullptr, 'h'}};

  int opt;
  while ((opt = getopt_long(argc, argv, "r:s:h", options, nullptr)) != -1) {
    switch (opt) {
    case 'r':
      sscanf(optarg, "%d", &g_repeat);
      g_repeat = g_repeat > 0 ? g_repeat : DEFAULT_REPEAT_TIMES;
      break;
    case 's':
      sscanf(optarg, "%u", &g_seed);
      break;
    case 'h':
      printf("Usage: %s [-h] [-r repeat] [-s seed]\n", argv[0]);
      return 1;
    default:
      break;
    }
  }

  std::cout << "Initializing random number generator with seed " << g_seed
            << std::endl;
  std::cout << "Repeating each test " << g_repeat << " times" << std::endl;

  VERIFY(OmniSketch::OmniSketchTest::get_registered_tests().size() > 0);
  for (const auto &ptr : OmniSketch::OmniSketchTest::get_registered_tests()) {
    g_test_name = ptr->name();
    ptr->operator()();
  }

  return g_success ? 0 : 1;
}
/** @endcond */
