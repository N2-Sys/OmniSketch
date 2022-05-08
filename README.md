## OmniSketch

OmniSketch is a C++ application for designing, simulating and testing of the sketch, a specific methodology in online network telemetry. It is designed to be efficient, consistent and easy to use. Currently it is supported on Windows, Linux and MacOS.

## Feature Overview

- **A wide variety of sketches** are implemented with minimal code. Some complicated sketches, such as ElasticSketch, are supported. For a full list of the sketches available, check [this table](#table).
- **A auto-testing framework** that tests and collects sketch performance on a given streaming data.
- **Counter Hierarchy framework** enables much more efficient space using through hierarchical counter braiding.
- **Pcap Parser** parses `.pcap` and `.pcapng` files to generate streaming data that is usable with OmniSketch. Data format of a streaming data record is also configurable.
- **Multiple definitions of a flowkey** are available. 1-tuple, 2-tuple and 5-tuple are supported.

## Download & Build

Download from GitHub release page:
```shell
git clone --recurse-submodules https://github.com/N2-Sys/OmniSketch.git
cd OmniSketch
mkdir build; cd build
```

The project is built with CMake. If you would love to build up an additional module of PcapParser, which is capable of parsing `.pcap/.pcapng` files in quite a versatile manner and interoperable with OmniSketch, please
```shell
cmake .. -DBUILD_PCAP_PARSER=True
```
Upon defining the `BUILD_PCAP_PARSER` macro like this, CMake automatically checks for [PcapPlusPlus](https://github.com/seladb/PcapPlusPlus) and [pcap](https://www.tcpdump.org) libraries, so MAKE SURE you have installed it in advance. The default included directories of the PcapPlusPlus header files is `/usr/local/include/pcapplusplus`. To change this included path, provide to CMake a new argumnet:
```shell
cmake .. -DBUILD_PCAP_PARSER=True -DPCPP_INCLUDE_PATH=[path on your system]
# e.g. cmake .. -DBUILD_PCAP_PARSER=True -DPCPP_INCLUDE_PATH=/usr/local/include/pcapplusplus
```
In the simplest case, if PcapParser is not in need, you can just build with
```shell
cmake ..
```
Note that now the `PcapPlusPlus` and `pcap` libraries are not needed.

## Design New Sketches

Overview:

1. Sketch algorithm is in `src/sketch/`. 
2. Sketch testing procedure is defined in `src/sketch_test/`.
3. Add a new sketch target in `CMakeLists.txt`.
4. Write down the sketch config in a toml file. The default config file is `src/sketch_config.toml`. All the sketch configs are user-defined, but typically should contain (though not required)
  - Streaming data file
  - Format of the streaming data record
  - Metrics measured during testing
  - Sketch parameters
  - Other user-defined configs
5. Goto the building directory `build`, cmake and make it. (CMake is needed since a new target is just added) From the time on, if the code is modified, only `make` is needed.
6. At this point, the sketch is compiled and linked. It is callable from terminal with `-c config` option and will display streaming data info and print the sketch statistics.

Details: (Use Count Min Sketch as an example)

> Suppose you decide to implement CM Sketch. For the final executable you decided to name it `CM` for brevity, so every time you can run `./CM [-c config]` to call it from the `build/` directory. You want when the sketch is created, it uses `data/records.bin` as the input and has 5 rows of counters with each row 80,001 counters. Furthermore, suppose the test code you write for CM first updates records sequentially into the sketch, and then for every flowkey queries the sketch for flow size. You want the update rate and the query rate, ARE (Average Relative Error) & AAE (Average Absolute Error) be measured. Finally, you want to config whether or not flow size is counted in packet or in byte. 

1. You implement Count Min Sketch in a file `src/sketch/{name}.h`. Say, `src/sketch/CMSketch.h`
2. You write your Count Min Sketch code. Since you have name the sketch file as `CMSketch.h`, the test file should be `src/sketch_test/CMSketchTest.h`. In general, `src/sketch_test/{name}Test.h`
3. In `CMakeLists.txt`, you append a line `add_user_sketch(CM {name})`. In our example, it is `add_user_sketch(CM CMSketch)`.
4. The config file you use, let's say, is `src/sketch_config.toml`. You may write these configs down as
```toml
[CM] # Count Min Sketch

  [CM.para]
  depth = 5
  width = 80001

  [CM.data]
  # Whether count in 
  cnt_method = "InPacket"
  # Run from `build/`
  data = "../data/records.bin"
  format = [["flowkey", "padding", "timestamp", "length", "padding"], [13, 3, 8, 2, 6]]

  [CM.test]
  update = ["RATE"]
  query = ["RATE", "ARE", "AAE"]
```
Or, if you prefer a flattened organization, the following is typically OK.
```toml
[CM] # Count Min Sketch

depth = 5
width = 80001
cnt_method = "InPacket"
data = "../data/records.bin"
format = [["flowkey", "padding", "timestamp", "length", "padding"], [13, 3, 8, 2, 6]]
update = ["RATE"]
query = ["RATE", "ARE", "AAE"]
```
No matter how you write the config, in the test code you have to fetch these configuartions via `ConfigParser` accordingly.
5. Goto `build/`. You want the pcap parser be built, so you run `cmake .. -DBUILD_PCAP_PARSER=True`. If no error prompts, it is the time for `make`.
6. In `build/` you see a file `CM`. Now you run `./CM -c ../src/sketch_config.toml` and it outputs, say:
```
terminal> ./CM -c ../src/sketch_config.toml
   INFO| Loading config from ../src/sketch_config.toml... @utils.cpp:62
VERBOSE| Config loaded. @utils.cpp:76
VERBOSE| Preparing test data... @data.h:718
   INFO| Loading records from ../data/records.bin... @data.h:720
VERBOSE| Records Loaded. @data.h:746
DataSet: 1090120 records with 99999 keys (../data/records.bin)
============     Count Min      ============
  Mem Footprint: 1.52646 MB
    Update Rate: 1853.95 Mpac/s
     Query Rate: 1694.9 Mpac/s
      Query ARE: 0.161831
      Query AAE: 0.252943
============================================
```

## API Docs
Please follow [this link]().


## Tables of Sketches
These are the sketches already implemented and tested.

Status: **h**ave, **c**hecked, checked but with **d**oubt, **t**ested.
<a id="table"></a>
|                         |Status|
| ----------------------- | ---- |
| cm sketch               | t    |
| count sketch            | t    |
| cu sketch               | t    |
| bloom filter            | t    |
| counting bloom filter   | t    |
| LD-sketch               | t    |
| MV-sketch               | t    |
| hashpipe                | t    |
| FM-sketch(PCSA)         | t    |
| Linear Counting         |      |
| Kmin(KMV)               |      |
| Deltoid                 | t    |
| flow radar              | t    |
| sketch learn            |      |
| elastic sketch          | t    |
| univmon                 |      |
| nitro sketch            | t    |
| reversible sketch       |      |
| Mrac                    | t    |
| k-ary sketch            | t    |
| seqHash                 |      |
| TwoLevel                | t    |
| multi-resolution bitmap |      |
| lossy count             | t    |
| space saving            | t    |
| HyperLogLog             | t    |
| Misra-Gries             | t    |
| Fast Sketch             | t    |
| CounterBraids           | t    |
| HeavyKeeper             | d    |