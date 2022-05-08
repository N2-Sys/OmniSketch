# OmniSketch

[OmniSketch](https://github.com/N2-Sys/OmniSketch) is a C++ framework for designing, simulating and testing sketch, a specific methodology in online network telemetry that targets at sketching statistical features of streaming data. It is designed to be efficient, consistent and easy to use. Currently it is supported on Windows, Linux and MacOS.

## Feature Overview

- **A wide variety of sketches** are implemented with minimal code. Some complicated sketches, such as ElasticSketch, are also supported. For a full list of the sketches available, check [this table](#table).
- **An auto-testing framework** that tests sketch performance and collects interested metrics with minimal configurations.
- **A generally applicable Counter Hierarchy framework** that enables a tremendous amount of space saving via hierarchical counter braiding, applicable to all value-based counters.
- **Pcap Parser** parses `.pcap` and `.pcapng` files to generate streaming data that interoperates with OmniSketch. Data format of a streaming data record is customizable.
- **Flexible definitions of a flowkey** are available. Currently, 1-tuple, 2-tuple and 5-tuple are supported.
- **No redundant code has to be rewritten.** Designer can focus mainly on the sketch algorithm, and let the framework do the rest, e.g. data parsing and testing details.
- Since sketching algorithms vary a lot, the framework includes only the most common methods. Do not panic! You can always write inject your own code and tell the framework to run it.

## Download & Build

Download from GitHub release page:
```shell
git clone --recurse-submodules https://github.com/N2-Sys/OmniSketch.git
cd OmniSketch
mkdir build; cd build
```

The project is built with CMake. If an additional module of PcapParser, which is capable of parsing `.pcap/.pcapng` files in quite a versatile manner and is interoperable with OmniSketch, is requested, please define the `BUILD_PCAP_PARSER` macro by
```shell
cmake .. -DBUILD_PCAP_PARSER=True
```
Upon defining this macro, CMake automatically checks for dependencies on [PcapPlusPlus](https://github.com/seladb/PcapPlusPlus) and [pcap](https://www.tcpdump.org) libraries, so MAKE SURE you have installed these libraries in advance. The default included directory of the PcapPlusPlus header files is `/usr/local/include/pcapplusplus` (which is certainly not true on Windows). To change this default searching path, provide to CMake a new argument:
```shell
cmake .. -DBUILD_PCAP_PARSER=True -DPCPP_INCLUDE_PATH=[path on your system]
# e.g. cmake .. -DBUILD_PCAP_PARSER=True -DPCPP_INCLUDE_PATH=/usr/local/include/pcapplusplus
```
In the simplest case, if PcapParser is not requested, the `PcapPlusPlus` and `pcap` libraries are never needed, so you can just build with
```shell
cmake ..
```
Gee, it saves you a lot of work!

## Design New Sketches

Here is an overview of how to design your own sketch in OmniSketch. For a detailed description, please check [the docs]().

1. Sketch algorithm should be in `src/sketch/`. Suppose you add a file `XXX.h` to this directory.
2. Sketch testing procedure is defined in `src/sketch_test/`. You should name your testing file as `XXXTest.h` accordingly.
3. Add a new sketch target in `CMakeLists.txt`. This is done in a single line `add_user_sketch(YYY XXX)`.
4. Write down the sketch config in a toml file. The default config file is `src/sketch_config.toml`. All the sketch configs are user-defined, but typically should contain (though not required)
  - Streaming data file
  - Format of the streaming data record
  - Metrics measured during testing
  - Sketch parameters
  - Other user-defined configs
> It is the user who controls what, where and how to parse in the config file. Omnisketch imposes no restrictions on how you organize the toml file and what you put in there. Fortunately, OmniSketch provides a rich set of tools to help you do it in just a couple of lines.

5. Goto the building directory `build/`, cmake and `make` it. (CMake is needed since a new target has just been added) From the time on, if the code is modified, only `make` is needed.
6. At this point, the sketch is compiled and linked. It is callable from the terminal with `./YYY -c config`. If no `-c` option is provided, `src/sketch_config.toml` is assumed to be the default config file. A possible output of Count Min Sketch runs as follows:
```shell
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
7. From now on, every time your sketch is about to run on different data and formats, or to collect some new statistics, all you have to do is simply modifying the config file. If the template header should be changed, you have to `make` a new driver.


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