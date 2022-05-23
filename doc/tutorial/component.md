@page component Main components
@tableofcontents

Congratulations! Now you have had a solid understanding of OmniSketch's foundations. To further our knowledge, we turn to the main components of OmniSketch. They are the workhorses you must master!

@section configparser Config Parser

One of the initial questions that may spring to your mind is how to parse the config file. In OmniSketch, since many configurations are structured data, like a vector or a table, we choose [toml](https://toml.io/en/) to be the config language. The link points to a brief tutorial on the toml language and we parse the `.toml` file via a C++ repo, [tomlplusplus](https://marzer.github.io/tomlplusplus/). It is one of the submodules you fetched via `git clone --recurse-submodules` when cloning this framework!

Our Config Parser wraps around tomlplusplus by providing a file-system-like interface called [ConfigParser](@ref OmniSketch::Util::ConfigParser). Path to the root table of a toml file is always an empty string `""`, and this is the default place where an opened Config Parser is located. To parse a piece of information within a table, you first have to invoke [setWorkingNode()](@ref OmniSketch::Util::ConfigParser::setWorkingNode()) for the parser to step into that table, and then calls [parseConfig()](@ref OmniSketch::Util::ConfigParser::parseConfig()) on the argument you want to parse. Since the table can nest in toml, a path is always a `.`-concatenated string from the root to the destination table. (So in terms of a file system, there is only absolute path!) Let us figure it out via an example. Suppose we have a toml file `example.toml` in the `build/` (the directory where you should run your sketch).

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.toml
# example.toml

# root table
name = "OmniSketch"
fibonacci = [1, 1, 2, 3, 5, 8]

# table
[tutorial]
name = "Sketch"

  # nested table
  [tutorial.component]
  fibonacci = true
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

@subsection openconfig Open the Config File

To open the config file, you should provide a path to it and then check the opening status. The following lines of code are typically seen in every sketch test file. See [the testing class of the Bloom Filter](https://github.com/N2-Sys/OmniSketch/blob/main/src/sketch_test/BloomFilterTest.h#L73) for instance.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
// In function `void runTest(void)`
// using namespace OmniSketch::Util;
ConfigParser parser("example.toml");
if(!parser.succeed()){
  return;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

@subsection parseatroot Parse Root Table

Now the parser is at the root table. If you have `std::string name` and `std::vector<int32_t> fib` defined, you can parse with

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
if(!parser.parseConfig(name, "name")){
  return;
}
if(!parser.parseConfig(fib, "fibonacci"){
  return;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As you are aware, the first argument holds a reference to a runtime variable, while the second gives the key name in the config table.

@subsection switchtable Switch to a New Table

Let us continue to parse `name` in the table `tutorial`. This can be done in two lines.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
parser.setWorkingNode("tutorial");
if(!parser.parseConfig(name, "name")){
  return;
}
// assert(name == "Sketch");
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note that the content of `name` is no longer `"OmniSketch"`, but `"Sketch"`! Continuing our journey, now we are to parse the innermost `fibonacci`, which is a boolean variable. Suppose we have `bool var` defined.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
parser.setWorkingNode("tutorial.component");
if(!parser.parseConfig(var, "fibonacci")){
  return;
}
// assert(var == true);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you want to go back to the root table again, call `parser.setWorkingNode("")`.

@subsection closeconfigparser Close Config Parser

Indeed, you do not have to bother with the closing issue. Config Parser will free its resources once it is destroyed.

@subsection reminderpcapparser Reminder

You have the feeling of mastering Config Parser, don't you? But before we proceed to the next topic, here a few reminders.

- Toml is only capable of recording limited types of data directly, so is [parseConfig()](@ref OmniSketch::Util::ConfigParser::parseConfig()). Here is a list of all supported data types, you will find them sufficient in virtually all cases.
  - `int32_t`
  - `size_t`
  - `double`
  - `bool`
  - `std::string`
  - `std::vector<int32_t>`
  - `std::vector<size_t>`
  - `std::vector<double>`
  - `std::vector<std::string>`
  - `toml::array`
> But it does *NOT* mean that any type other than these cannot be parsed. Take a \f$M\times N\f$ matrix for example. In OmniSketch, you can parse it with a `toml::array`, which represents an object containing \f$M\f$ inner `toml::array`, which in turn contains \f$N\f$ elements each. From the returning `toml::array`, you may call underlying API documented in [tomlplusplus](https://marzer.github.io/tomlplusplus/) to get the matrix elements one by one.

- Again, we emphasize that all paths in Config Parser are absolute (i.e., start from the root) and are interspersed with dots (`.`), not slash (`/`) or backslash (`\`) as are in a usual file system. 
- On a parsing error, a descriptive message will be logged, so just return to end the function if in case it does happen. This is how all previous codes do.

@section dataformat Data Format

Data format controls the output format of @ref pcapparser and the way OmniSketch interpreting the input binary file. It specifies which fields are in a packet record, the length of each field, and the order in which they are deserialized. A valid data format runs as follows. It is excerpted from [sketch_config.toml](https://github.com/N2-Sys/OmniSketch/blob/main/src/sketch_config.toml).
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.toml
format = [["flowkey", "padding", "timestamp", "length", "padding"],
          [13,        3,         8,           2,        6]]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
It says that each packet record in `.bin` takes \f$13+3+8+2+6=32\f$ bytes, with the first 13 bytes being the flowkey, then following a 3-byte padding and so on. Its semantics is quite straightforward, isn't it?

@subsection syntaxdataformat Syntax
You may be wondering the determining rule of a valid data format expression in toml, and it is what we now touch upon. Though a context-free grammar (Yup, a context-sensitive grammar overpowers here. This is a mild exercise in automata theory.) suffices to express this rule, we find this description rather tricky and tedious, so we recede to a description in natural language here.

There are four distinct fields. Some have their length contained, while some have further constraints on their occurence. All these restrictions are summarized in this table.
| Field Name | Viable Length | Further Constraints                   |
|:-----------|:--------------|:--------------------------------------|
| flowkey    | 4, 8, 13      | Specify exactly once                  |
| timestamp  | 1, 2, 4, 8    | In microseconds. Specify at most once |
| length     | 1, 2, 4, 8    | Specify at most once                  |
| padding    | > 0           | None                                  |

In fact, any data format not violating any of these rules is valid. You can check the preceding example that it is indeed valid. Here are a few more examples to help you consolidate your understanding.

```text
[["timestamp", "length"], [2, 4]] # Invalid, no `flowkey` is specified
[["padding", "flowkey"], [0, 12]] # Invalid, `padding` length should > 0; `flowkey` cannot be 12 bytes
[["flowkey"], [4]] # Valid, the simplest data format
[["flowkey", 4]] # Invalid, brackets mismatch
[["timestamp", "flowkey", "padding", "timestamp"], [4, 8, 1, 4]] # Invalid, `timestamp` occurs twice
```

@subsection getdataformat Read Data Format
But how you get this data format? Well, as you may have noticed, `toml::array` is a parsable type of [Config Parser](@ref reminderpcapparser). On the other hand, [DataFormat](@ref OmniSketch::Data::DataFormat) should be constructed with just a `toml::array`. This is not a coincidence!

Let again look into [the testing class of the Bloom Filter](https://github.com/N2-Sys/OmniSketch/blob/main/src/sketch_test/BloomFilterTest.h#L89). You see these lines.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
if (!parser.parseConfig(arr, "format"))
    return;
Data::DataFormat format(arr);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It searches for a key named `format` in the current toml table and return a `toml::array` object. Later, it is used to construct a [DataFormat](@ref OmniSketch::Data::DataFormat) object. Shortly we will see how this [DataFormat](@ref OmniSketch::Data::DataFormat) object is used: It is passed to a [StreamData](@ref OmniSketch::Data::StreamData) object to parse the input data.

@section streamdata Stream Data

A runtime container is needed to hold the packet records, and this job is delegated to [StreamData](@ref OmniSketch::Data::StreamData). All OmniSketch knows about the data come from this class.

@subsection readstreamdata Read Stream Data

As with @ref configparser and @ref dataformat, reading is done upon constructing a class instance. You should give two arguments to the constructor: first a path to the `.bin` file, followed by a corresponding @ref dataformat. You should check the reading status with the [succeed()](@ref OmniSketch::Data::StreamData::succeed()) method.

> Make sure `key_len` in template header and the `flowkey` length in config file match. Otherwise, a runtime exception would be thrown.

@subsection iteratorstreamdata Iterators

Conceptually speaking, streaming data is nothing but a sequence of packets, so it is no wonder that [StreamData](@ref OmniSketch::Data::StreamData) supports random-access iterator. Moreover, the interface of getting an iterator out of this class resembles that of STL. There you fetch the size via [size()](@ref OmniSketch::Data::StreamData::size()), get a beginning or endding iterator via [begin()](@ref OmniSketch::Data::StreamData::begin()) and [end()](@ref OmniSketch::Data::StreamData::end()). In case that you would love to access a record in between, make a call to [diff()](@ref OmniSketch::Data::StreamData::diff()).

Two such iterators \f$\langle p,q\rangle\f$, with proper order, represents a range of data \f$[p,q)\f$. Note that the range is left-included and right-excluded. This chunk of data is always what your sketch are to dispose of.

@section gndtruth Ground Truth

Most [metrics](@ref metrics) compare the result of a query to sketch with the ground truth. That is basically why you have to maintain the real-world answer from time to time. Luckily, in OmniSketch this is done by the class [GndTruth](@ref OmniSketch::Data::GndTruth). It has two template parameters, first the length of flowkey and then a counter type. Throughout the project, we encourage the practice of explicitising length of every built-in numeric type, like `int32_t` in lieu of `int`, `uint64_t` in lieu of `unsigned long long`.

GndTruth is a bidirectional map between flowkey and its value. That is, you can not only query a flowkey for its value, but a value for all the corresponding flowkeys. It starts with a default constructor, and after that you should call its method to fill its content from yet another [GndTruth](@ref OmniSketch::Data::GndTruth) or [StreamData](@ref OmniSketch::Data::StreamData) instance. Again, [the testing class of the Bloom Filter](https://github.com/N2-Sys/OmniSketch/blob/main/src/sketch_test/BloomFilterTest.h#L129) for inspiration. You may notice that in OmniSketch there are two methods for counting: `InPacket` and `InLength`. Difference is easy to tell: `InPacket` counts each packet, regardless of its length, as 1; Yet `InLength` counts its length.

@subsection iteratorgndtruth Iterators

Since [GndTruth](@ref OmniSketch::Data::GndTruth) is in essence a bidirectional map, its iterator must differ from that of [StreamData](@ref OmniSketch::Data::StreamData) a bit. Any iterator stemmed from this class points to a bidirectional key-value pair: one for flowkey (accessible with the method `get_left()` or the member `second`), the other for the value (accessible with the method `get_right()` or the member `first`).

What's more, the iterator is random-access. This feature greatly facilitates an efficient implementation of OmniSketch, but you are not likely to encounter such a use case when you design new sketch.

@subsection estimate Estimation

When it comes to heavy hitters and heavy changers, your sketch may output a list of candidates, each being a key-value pair. Though occasionally your sketch may not be able to predict the value of a flowkey, you may just fill in the value with all 0.

[Estimation](@ref OmniSketch::Data::Estimation) is just another wrapper around [GndTruth](@ref OmniSketch::Data::GndTruth), and the available interfaces are hugely changed. It provides you with several insertion methods and should be the returning type for [getHeavyHitter()](@ref OmniSketch::Sketch::SketchBase::getHeavyHitter()) or [getHeavyChanger()](@ref OmniSketch::Sketch::SketchBase::getHeavyChanger()). Follow the link to docs whenever necessary.

@section sketchntest Sketch & Test

Finally we have arrive at a place where sketch is implemented and tested. You see that all sketches reside in the directory `src/sketch`. In order for the testing class to find the methods in base sketch, we take a quintessential object-oriented method, derivation. Your sketch should be subclass of [SketchBase](@ref OmniSketch::Sketch::SketchBase), and any method overriden should be declared with a `override` specifier. See the detailed description of [SketchBase](@ref OmniSketch::Sketch::SketchBase) for a complete instruction on how to do this. Besides, you are encouraged to take a look at the current sketches and mimic their way of coding.

The sketch, on its design, had better have as its template parameters the length of flowkey, type of hashing class and counter type. But a non-template implementation works just fine. Here is a template file provided for your convenience.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
/**
 * @file [SKETCH_FILENAME]
 * @author [AUTHOR]
 * @brief [INFO]
 *
 */
#pragma once

#include <common/hash.h>
#include <common/sketch.h>

namespace OmniSketch::Sketch {
/**
 * @brief [INFO]
 *
 * @tparam key_len  length of flowkey
 * @tparam T        type of the counter
 * @tparam hash_t   hashing class
 */
template <int32_t key_len, typename T, typename hash_t = Hash::AwareHash>
class [SKETCH] : public SketchBase<key_len, T> {
private:
  ...

public:
  [SKETCH_CONSTRUCTOR]
  [SKETCH_DESTRUCTOR]

  [OVERRIDEN_METHODS]
  /*
   * void update(const FlowKey<key_len> &flowkey, T val) override;
   * T query(const FlowKey<key_len> &flowkey) const override;
   * size_t size() const override;
   *
   */

  [OTHER_METHODS]
};

} // namespace OmniSketch::Sketch

//-----------------------------------------------------------------------------
//
//                        Implementation of templated methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Sketch {
...
} // namespace OmniSketch::Sketch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The testing side follows the same rationale. All your testing classes should be derived from [TestBase](@ref OmniSketch::Test::TestBase), which defines test routines using the pointer to [SketchBase](@ref OmniSketch::Sketch::SketchBase). The main task in subclass is to override the [runTest()](@ref OmniSketch::Test::TestBase::runTest()) method just as in [BloomFilterTest](https://github.com/N2-Sys/OmniSketch/blob/main/src/sketch_test/BloomFilterTest.h). Here is a template file provided for your convenience. You may be curious about how to override [runTest()](@ref OmniSketch::Test::TestBase::runTest()) and what the last three mysterious lines do. Shortly we will explain it.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.cpp
/**
 * @file [TEST_FILENAME]
 * @author [AUTHOR]
 * @brief [INFO]
 *
 */
#pragma once

#include <common/test.h>
#include <sketch/[SKETCH_FILENAME]>

namespace OmniSketch::Test {
/**
 * @brief [INFO]
 *
 */
template <int32_t key_len, typename hash_t = Hash::AwareHash>
class [SKETCH_TEST] : public TestBase<key_len> {

public:
  [CONSTRUCTOR]

  void runTest() override;
};

} // namespace OmniSketch::Test

//-----------------------------------------------------------------------------
//
//                        Implementation of templated methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Test {
...
} // namespace OmniSketch::Test

// Driver instance:
//      AUTHOR: [AUTHOR]
//      CONFIG: [DEFAULT_CONFIG]  # with respect to the `src/` directory
//    TEMPLATE: [TEMPLATE_HEADER]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

@subsection overrideruntest Override runTest()

Now let us expand our mastery of testing. What should we do in [runTest()](@ref OmniSketch::Test::TestBase::runTest())? Well, there is no panacea, because "a thousand sketches have a thousand testing methods", but basically we will do the following sequentially.

1. Parse the configurations via @ref configparser.
2. Allocate one or several sketch instances with the parsed configurations as the constructing arguments and downcast the pointer(s) to `SketchBase<key_len> *`. It is even more robust if you create a `unique_ptr<SketchBase<key_len>>` object.
3. Use the data format parsed and path to input data to read in streaming data via @ref streamdata.
4. Data should be used at your discretion. On the one hand, they are fed into the sketch via overriden method. On the other hand, @ref gndtruth scans through the data to help you get the ground truth value.
5. Note that we do not have to call the overriden methods in sketch directly. All the methods `testXXX()` in `TestBase` call them de facto. With a method name called `testXXX()`, we supply a key named `XXX` in the config file to indicate which metrics to collect. The following table shows which metrics can be collected in the corresponding testing methods.
|Test method |Required overriden function|Available Metrics (Test::Metric)|
|:-----------|:--------------------------|:-------------------------------|
|[testSize()](@ref OmniSketch::Test::TestBase::testSize())  |[size()](@ref OmniSketch::Sketch::SketchBase::size()) | SIZE                           |
|[testInsert()](@ref OmniSketch::Test::TestBase::testInsert())|[insert()](@ref OmniSketch::Sketch::SketchBase::insert())|RATE                           |
|[testUpdate()](@ref OmniSketch::Test::TestBase::testUpdate())|[update()](@ref OmniSketch::Sketch::SketchBase::update())|RATE                           |
|[testQuery()](@ref OmniSketch::Test::TestBase::testQuery()) |[query()](@ref OmniSketch::Sketch::SketchBase::query())|RATE, ARE, AAE, ACC, PODF, DIST |
|[testLookup()](@ref OmniSketch::Test::TestBase::testLookup())|[lookup()](@ref OmniSketch::Sketch::SketchBase::lookup())|RATE, TP, FP, PRC              |
|[testHeavyHitter()](@ref OmniSketch::Test::TestBase::testHeavyHitter())|[getHeavyHitter()](@ref OmniSketch::Sketch::SketchBase::getHeavyHitter())|TIME,ARE,PRC,RCL,F1|
|[testHeavyChanger()](@ref OmniSketch::Test::TestBase::testHeavyChanger())|[getHeavyChanger()](@ref OmniSketch::Sketch::SketchBase::getHeavyChanger())|TIME,ARE,PRC,RCL,F1|

> The rule of thumb is that if you are not sure what to do next, feel free to refer to existing sketch tests!

@subsection trailingcomment Trailing Comment

It is suggested that both sketch and test are templated, so how to generate a ready-to-be-compiled driver?

The driver has to some minimal task: It looks for `-c` option from terminal to change the default config file, and then allocates a sketch test instance and invoke its overriden `runTest()` method. This process is so mechanic that we provide a python script `src/generate_driver.py` to do such job.

You never have to run this python script manually. Indeed, the command `add_user_sketch` in cmake is specially customized so that each time you `make` (not `cmake`) the project, if the sketch file or test file should be changed, a new driver is automatically compiled and linked for you, using the name from `add_user_sketch`.

How does `generate_driver.py` know what driver you want to generate? Now trailing comment in the last three lines kicks into play. Technically speaking, three pieces of information should be provided: author's name (after `AUTHOR:`), default configuration file for this test class (after `CONFIG:`), plus a template header with arguments bracketed inside a `<>` (after `TEMPLATE:`).

The order of these lines are uninterested. What matters is that they appear on the last three distinct lines, beginning with a line comment or inside a block comment. If the test class is not templated you leave the space after `TEMPLATE:` empty. To convince yourself of the exact rule of writing trailing comment, you read through the python script. Nevertheless, the safest and worthiest way is to copy comment from existing testing files and modify it slightly.

