import sys, os, re

def help(name: str):
  print("usage:")
  print(f"  {name} [input_file]\n")
  print(f"explanation:")
  print(f"  The input file should be XXXTest.h. "
        f"The last three lines of the file should contain the author's name, "
        f"default config file, and template header for the driver.")
  sys.exit(-1)

def extract(line: str, argv):
  seg = re.split("([A-Z]+:|#)", line)
  if len(seg) < 3:
    print(f"({argv[0]}) {argv[1]}: Unrecognized author/config/template info lines at the rear.")
    sys.exit(-1)
  return [seg[1][:-1], seg[2].strip()]

def pick(a, b, c, name: str):
  if a[0] == name:
    return a[1]
  if b[0] == name:
    return b[1]
  return c[1]

def main(argv):
  file, author, config, template = "", "", "", ""
  has_template = False

  if len(argv) != 2:
    help(argv[0])
  if not argv[1].endswith("Test.h"):
    print(f"({argv[0]}) File name must end with Test.h, got {argv[1]}.")
    sys.exit(-1)
  if not os.path.isfile(argv[1]):
    print(f"({argv[0]}) {argv[1]}: Not a regular file.")
    sys.exit(-1)
  with open(argv[1], "r") as fp:
    lines = fp.readlines()
    while lines and lines[-1] == "\n":
      lines.pop()
    if len(lines) < 3:
      print(f"({argv[0]}) {argv[1]}: Missing author/config/template info.")
      sys.exit(-1)
    a, b, c = extract(lines[-3], argv), extract(lines[-2], argv), extract(lines[-1], argv)
    if {a[0], b[0], c[0]} != {"AUTHOR", "CONFIG", "TEMPLATE"}:
      print(f"({argv[0]}) {argv[1]}: Missing author/config/template info.")
      sys.exit(-1)
    author = pick(a, b, c, "AUTHOR")
    config = pick(a, b, c, "CONFIG")
    template = pick(a, b, c, "TEMPLATE")
    if not config.startswith('/'):
      config = "../src/" + config
    file = re.split(r'[\\/]', argv[1])[-1][:-6]
    driver_name = file + "Driver.cpp"
    sketch_name = " ".join(re.findall("[A-Z][^A-Z]*", file))
    driver = f"""/**
 * @file {driver_name}
 * @author {author}
 * @brief Driver of {sketch_name}
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <sketch_test/{file}Test.h>
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
      std::make_unique<Test::{file}Test{template}>(config_file);
  ptr->runTest();
  return 0;
}}

static void Help(const char *ptr) {{
  fmt::print("Usage: {{}} [-c config]\\n", ptr);
  exit(0);
}}"""
    with open(f"driver/{driver_name}", "w") as fp:
      fp.write(driver)

if __name__ == "__main__":
  main(sys.argv)