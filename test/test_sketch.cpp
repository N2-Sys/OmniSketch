/**
 * @file test_sketch.cpp
 * @author dromniscience (you@domain.com)
 * @brief Test base sketch
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "test_factory.h"
#include <common/test.h>

template <int32_t key_len, typename T>
class MySketch : public OmniSketch::Sketch::SketchBase<key_len, T> {
public:
  size_t size() const override { return 100000; }
  void insert(const OmniSketch::FlowKey<key_len> &flowkey) override {
    ::usleep(1000);
  }
  void update(const OmniSketch::FlowKey<key_len> &flowkey, T val) override {
    ::usleep(1);
  }
  T query(const OmniSketch::FlowKey<key_len> &flowkey) const override {
    ::usleep(1);
    return 1;
  }
  bool lookup(const OmniSketch::FlowKey<key_len> &flowkey) const override {
    ::usleep(1);
    return *reinterpret_cast<const int32_t *>(flowkey.cKey()) != 0xe;
  }
  OmniSketch::Data::Estimation<key_len, T>
  getHeavyHitter(double threshold) const override {
    OmniSketch::Data::Estimation<key_len, T> est;
    OmniSketch::FlowKey<4> key_1(0x1), key_2(0x7), key_3(0x3);
    est.update(key_1, 5);
    est.update(key_2, 4);
    est.update(key_3, 1);

    return est;
  }
  OmniSketch::Data::Estimation<key_len, T> getHeavyChanger(
      std::unique_ptr<OmniSketch::Sketch::SketchBase<key_len, T>> &ptr_sketch,
      double threshold) const override {
    OmniSketch::Data::Estimation<key_len, T> est;
    OmniSketch::FlowKey<4> key_1(0x1), key_2(0x7), key_3(0x3);
    est.update(key_1, 4);
    // est.update(key_2, 4);
    est.update(key_3, 1);

    return est;
  }
  OmniSketch::Data::Estimation<key_len, T> decode() override {
    OmniSketch::Data::Estimation<key_len, T> est;
    OmniSketch::FlowKey<4> key_1(0x1), key_2(0x7), key_3(0x3);
    est.update(key_1, 5); // gnd truth: 5
    est.update(key_2, 3); // gnd truth: 5
    est.update(key_3, 5); // gnd truth: 4

    return est;
  }
};

void TestTest() {
  using namespace OmniSketch::Test;
  using namespace OmniSketch::Data;
  using std::string_view_literals::operator""sv;

  static constexpr std::string_view input = R"(
        name = [["flowkey"], [4]]
    )"sv;
  toml::table array = toml::parse(input);
  DataFormat format(*array["name"].as_array());

  char name[L_tmpnam];
  std::tmpnam(name);

  const int32_t flowkey[32] = {0x1, 0x3, 0x8, 0xa, 0x8, 0xa, 0x1, 0x5,
                               0x5, 0x2, 0x5, 0x9, 0x1, 0x4, 0x4, 0x6,
                               0x8, 0x1, 0x2, 0xa, 0x6, 0x7, 0x1, 0x3,
                               0x3, 0x3, 0x4, 0x4, 0x7, 0x7, 0x7, 0x7};
  const int32_t flowkey_2[8] = {0x1, 0x3, 0x5, 0x7, 0x9, 0xb, 0xd, 0xe};

  char content[128];
  for (int i = 0; i < 32; ++i) {
    *reinterpret_cast<int32_t *>(content + 4 * i) = flowkey[i];
  }

  std::ofstream fout(name, std::ios::binary);
  fout.write(content, sizeof(content));
  fout.close();

  StreamData<4> data(name, format);
  VERIFY(data.succeed() == true);
  std::remove(name);

  char content_2[32];
  for (int i = 0; i < 8; ++i) {
    *reinterpret_cast<int32_t *>(content_2 + 4 * i) = flowkey_2[i];
  }

  std::ofstream fout_2(name, std::ios::binary);
  fout_2.write(content_2, sizeof(content_2));
  fout_2.close();

  StreamData<4> data_2(name, format);
  VERIFY(data_2.succeed() == true);
  std::remove(name);

  TestBase<4, int32_t> test("My Sketch", "test_sketch.toml", "XXX.tmp.test");
  std::unique_ptr<OmniSketch::Sketch::SketchBase<4, int32_t>> ptr(
      new MySketch<4, int32_t>);

  GndTruth<4, int32_t> gnd_truth, gnd_truth_2;
  gnd_truth.getGroundTruth(data.begin(), data.end(), InPacket);
  gnd_truth_2.getGroundTruth(data_2.begin(), data_2.end(), InPacket);
  VERIFY(gnd_truth.size() == 10);
  VERIFY(gnd_truth_2.size() == 8);

  test.testSize(ptr);
  test.testInsert(ptr, data.begin(), data.end());
  test.testUpdate(ptr, data.begin(), data.end(), InPacket);
  test.testQuery(ptr, gnd_truth);
  test.testLookup(ptr, gnd_truth_2, gnd_truth);
  test.testDecode(ptr, gnd_truth);

  GndTruth<4, int32_t> gnd_truth_3;
  gnd_truth_3.getHeavyHitter(gnd_truth, 4.9 / 32, Percentile);
  test.testHeavyHitter(ptr, 5.0 / 32, gnd_truth_3);
  test.testHeavyChanger(ptr, ptr, 5.0 / 32, gnd_truth_3);
  test.show();
}

OMNISKETCH_DECLARE_TEST(sketch) {
  for (int i = 0; i < g_repeat; ++i) {
    TestTest();
  }
}