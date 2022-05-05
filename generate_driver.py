import sys, getopt, os, re

def help(name):
  print("usage:")
  print(f"  {name} -f [input_file] -a [author] -c [default_config_file] -t [\"template_header\"]\n")
  print("example:")
  print(f"  {name} -f sketch_test/BloomFilterTest.h -a dromniscience -c ../test.toml -t \"13,Hash::AwareHash\"\n")
  print("explanation:")
  print(f"  1. `input_file` must begin with \"test/\".")
  print(f"  2. Working directory for `default_config_file` is assumed to be \"build/\".")
  print(f"     Be careful when you use a relative path.")
  sys.exit(-1)

def main(argv):
  file, author, config, template = "", "", "", ""
  has_template = False
  try:
    opts, args = getopt.getopt(argv[1:],"hf:a:c:t:",["file=","author=","config=", "template="])
  except getopt.GetoptError:
    help(argv[0])
  for opt, arg in opts:
    # Check arguments
    if opt == "-h":
      help(argv[0])
    if opt == "-f" or opt == "--file":
      file = arg
      if file.startswith("./"):
        file = file[2:]
      if not file.startswith("test/"):
        print("Error: Input file must be in the `test/` directory.")
        sys.exit(1)
      file = file[5:]
      files = os.listdir("test/")
      if file not in files:
        print("Error: Input file not found in test/")
        sys.exit(1)
      if not file.endswith("Test.h"):
        print("Error: Naming convention in `test/` is violated.")
        sys.exit(1)
      files = os.listdir("sketch/")
      tmp = file[:-6] + ".h"
      if (file[:-6] + ".h") not in files:
        print(f"Error: No file named {tmp} in the `sketch/` directory.")
        sys.exit(1)
    elif opt == "-a" or opt == "--author":
      author = arg
    elif opt == "-c" or opt == "--config":
      config = arg
    elif opt == "-t" or opt == "--template":
      has_template = True
      template = arg.replace(",", ", ");
  # Make sure all four arguments are given
  if not file:
    print("Error: Input file must be specified.")
    sys.exit(2)
  if not author:
    print("Error: Author must be specified.")
    sys.exit(2)
  if not config:
    print("Error: Default config file must be specified.")
    sys.exit(2)
  if not has_template:
    print("Error: Template header must be specified.")
    sys.exit(2)

  # include files [in sketch/]
  driver_name = file[:-6] + "Driver.cpp"
  # sketch name [partitioned at the capital letter]
  sketch_name = re.findall("[A-Z][^A-Z]*", file[:-6])
  sketch_name = ' '.join(sketch_name)
  print(file, driver_name, sketch_name, author, config, template)

  driver = f"""/**
 * @file {driver_name}
 * @author {author}
 * @brief Driver of {sketch_name}
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <sketch_test/{file}>
#include <getopt.h>
#include <iostream>

using namespace OmniSketch;

static void Help(const char *ptr);

// Main
int main(int argc, char *argv[]) {{
  std::string config_file = "{config}";

  // parse command line arguments
  int opt;
  option options[] = {{{{"config", required_argument, nullptr, 'c'}},
                      {{"help", no_argument, nullptr, 'h'}},
                      {{"verbose", no_argument, nullptr, 'v'}}}};
  while ((opt = getopt_long(argc, argv, "c:hv", options, nullptr)) != -1) {{
    switch (opt) {{
    case 'c':
      config_file = optarg;
      break;
    case 'h':
      Help(argv[0]); // never return
      break;
      // case 'v':
      //   TODO: Enable verbose
      //   break;
    default:
      break;
    }}
  }}
  auto ptr =
      std::make_unique<Test::{file[:-2]}{("<" + template + ">") if template else ""}>(config_file);
  ptr->runTest();
  return 0;
}}

static void Help(const char *ptr) {{
  fmt::print("Usage: {{}} [-c config]\\n", ptr);
  exit(0);
}}"""
  with open("driver/" + driver_name, "w") as fp:
    fp.write(driver)


if __name__ == "__main__":
  main(sys.argv)