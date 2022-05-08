/**
 * @file parser.cpp
 * @author dromniscience (you@domain.com)
 * @brief Pcap Parser
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "parser.h"
#include <getopt.h>

int main(int argc, char *argv[]) {
  int option_index = 0;
  int opt = 0;
  int verbose_level = 0;
  std::string config_file;

  struct option pcap_parser_options[] = {{"help", no_argument, 0, 'h'},
                                         {"verbose", no_argument, 0, 'v'},
                                         {"config", required_argument, 0, 'c'},
                                         {0, 0, 0, 0}};
  auto printUsage = []() {
    std::cout << std::endl
              << "Usage:" << std::endl
              << "------" << std::endl
              << pcpp::AppName::get() << " [-c config] [-v]" << std::endl
              << std::endl
              << "Options:" << std::endl
              << std::endl
              << "    -c config : Config file of parser" << std::endl
              << "    -v        : Verbose level " << std::endl
              << "    -h        : Display this help message and exit"
              << std::endl
              << std::endl;
  };

  while ((opt = getopt_long(argc, argv, "c:vh", pcap_parser_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 0:
      break;
    case 'h':
      printUsage();
      exit(0);
      break;
    case 'v':
      verbose_level += 1;
      break;
    case 'c':
      config_file = optarg;
      break;
    default:
      printUsage();
      exit(-1);
    }
  }

  // Truncate Verbose level
  verbose_level = std::min(verbose_level, 2);

  OmniSketch::Util::PcapParser<13> pcap_parser(config_file, "parser",
                                               verbose_level);
  if (!pcap_parser.succeed())
    exit(-1);
  pcap_parser.dumpPcapPacketsInBinary();
}