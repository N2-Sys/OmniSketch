/**
 * @file test_data.cpp
 * @author dromniscience (you@domain.com)
 * @brief Test data-processing tools
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "test_factory.h"
#include <common/data.h>
#include <unordered_map>

/**
 * @cond TEST
 *
 */
void TestDataFormat() {
  using std::string_view_literals::operator""sv;
  using namespace OmniSketch::Data;

  // normal
  try {
    static constexpr std::string_view input = R"(
        name = [["flowkey", "length", "padding", "timestamp", "padding"], [8, 4, 1, 2, 1]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());
    VERIFY(format.getRecordLength() == 16);

    Record<8> record;
    const int8_t a[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                          0x09, 0x0a, 0x0b, 0x0c, 0x00, 0x0e, 0x0f, 0x00};
    int8_t b[16];

    format.readAsFormat(record, a);
    format.writeAsFormat(record, b);
    VERIFY(memcmp(a, b, 16) == 0);

  } catch (const std::runtime_error &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
  try {
    static constexpr std::string_view input = R"(
        name = [["length", "padding", "flowkey"], [1, 2, 4]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());
    VERIFY(format.getRecordLength() == 7);

    Record<4> record;
    const int8_t a[7] = {0x01, 0x00, 0x00, 0x04, 0x05, 0x06, 0x07};
    int8_t b[7];

    format.readAsFormat(record, a);
    format.writeAsFormat(record, b);
    VERIFY(memcmp(a, b, 7) == 0);

  } catch (const std::runtime_error &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
  // exception
  try {
    static constexpr std::string_view input = R"(
        name = [["length", "padding"], [2, 2]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());
    SET_FAILURE_FLAG;
  } catch (const std::runtime_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    static constexpr std::string_view input = R"(
        name = [["length", "flowkey"], [2, 2]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());
    SET_FAILURE_FLAG;
  } catch (const std::runtime_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    static constexpr std::string_view input = R"(
        name = [["length", "flowkey", "padding"], [1, 4, 0]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());
    SET_FAILURE_FLAG;
  } catch (const std::runtime_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    static constexpr std::string_view input = R"(
        name = [["length", "flowkey", "flowkey"], [2, 4, 4]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());
    SET_FAILURE_FLAG;
  } catch (const std::runtime_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
  try {
    static constexpr std::string_view input = R"(
        name = [["length", "flowkey"], [2, 4]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());

    Record<8> record;
    const int8_t a[6] = {};
    format.readAsFormat(record, a);
    SET_FAILURE_FLAG;
  } catch (const std::runtime_error &exp) {
    VERIFY_EXCEPTION(exp);
  }
}

void TestGndTruth() {
  using std::string_view_literals::operator""sv;
  using namespace OmniSketch::Data;

  try {

    static constexpr std::string_view input = R"(
        name = [["flowkey", "length", "padding", "timestamp"], [4, 4, 2, 2]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());

    char name[L_tmpnam];
    std::tmpnam(name);

    const int32_t flowkey[10] = {0x1F1F1, 0x2F2F2, 0x1F1F1, 0x3F3F3, 0x4F4F4,
                                 0x1F1F1, 0x2F2F2, 0x3F3F3, 0x5F5F5, 0x1F1F1};
    const int32_t length[10] = {0x1,  0x2,  0x4,  0x8,   0x10,
                                0x20, 0x40, 0x80, 0x100, 0x200};
    const int16_t timestamp[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::unordered_map<int32_t, std::pair<int64_t, int64_t>> map;

    char content[120];
    for (int i = 0; i < 10; ++i) {
      *reinterpret_cast<int32_t *>(content + 12 * i) = flowkey[i];
      *reinterpret_cast<int32_t *>(content + 12 * i + 4) = length[i];
      *reinterpret_cast<int16_t *>(content + 12 * i + 10) = timestamp[i];
      auto tmp = map[flowkey[i]];
      map[flowkey[i]] = {tmp.first + length[i], tmp.second + 1};
    }
    std::ofstream fout(name, std::ios::binary);
    fout.write(content, sizeof(content));
    fout.close();

    StreamData<4> data(name, format);
    std::remove(name);

    VERIFY(data.succeed() == true);
    VERIFY(data.empty() == false);
    VERIFY(data.size() == 10);

    for (int i = 0; i < 10; ++i) {
      VERIFY(data.diff(i)->length == length[i]);
      VERIFY(*reinterpret_cast<const int32_t *>(data.diff(i)->flowkey.cKey()) ==
             flowkey[i]);
      VERIFY(data.diff(i)->timestamp == timestamp[i]);
    }
    VERIFY(data.begin() == data.diff(0));
    VERIFY(data.end() == data.diff(data.size()));

    GndTruth<4, int64_t> gnd_truth_1, gnd_truth_2;
    VERIFY(gnd_truth_1.empty());
    VERIFY(gnd_truth_2.empty());
    gnd_truth_1.getGroundTruth(data.begin(), data.end(), InLength);
    gnd_truth_2.getGroundTruth(data.begin(), data.end(), InPacket);
    VERIFY(!gnd_truth_1.empty());
    VERIFY(!gnd_truth_2.empty());

    int64_t currrent = std::numeric_limits<int64_t>::max();
    int64_t maximum = -1;
    for (const auto &kv : gnd_truth_1) {
      VERIFY(currrent >= kv.get_right());
      currrent = kv.get_right();
      if (maximum < 0)
        maximum = kv.get_right();
    }
    VERIFY(gnd_truth_1.max() == maximum);
    VERIFY(gnd_truth_1.min() == currrent);

    int32_t max = std::numeric_limits<int32_t>::max();
    for (const auto &kv : gnd_truth_1) {
      VERIFY(
          kv.get_right() ==
          map[*reinterpret_cast<const int32_t *>(kv.get_left().cKey())].first);
      VERIFY(max >= kv.get_right());
      max = kv.get_right();
    }
    max = std::numeric_limits<int32_t>::max();
    for (const auto &kv : gnd_truth_2) {
      VERIFY(
          kv.get_right() ==
          map[*reinterpret_cast<const int32_t *>(kv.get_left().cKey())].second);
      VERIFY(max >= kv.get_right());
      max = kv.get_right();
    }
    gnd_truth_1.swap(gnd_truth_1);
    max = std::numeric_limits<int32_t>::max();
    for (const auto &kv : gnd_truth_1) {
      VERIFY(
          kv.get_right() ==
          map[*reinterpret_cast<const int32_t *>(kv.get_left().cKey())].first);
      VERIFY(max >= kv.get_right());
      max = kv.get_right();
    }
    gnd_truth_1.swap(gnd_truth_2);
    max = std::numeric_limits<int32_t>::max();
    for (const auto &kv : gnd_truth_1) {
      VERIFY(
          kv.get_right() ==
          map[*reinterpret_cast<const int32_t *>(kv.get_left().cKey())].second);
      VERIFY(max >= kv.get_right());
      max = kv.get_right();
    }
    max = std::numeric_limits<int32_t>::max();
    for (const auto &kv : gnd_truth_2) {
      VERIFY(
          kv.get_right() ==
          map[*reinterpret_cast<const int32_t *>(kv.get_left().cKey())].first);
      VERIFY(max >= kv.get_right());
      max = kv.get_right();
    }

    VERIFY(gnd_truth_1.size() == 5);
    VERIFY(gnd_truth_2.size() == 5);
    for (const auto &kv : gnd_truth_1) {
      VERIFY(gnd_truth_1.at(kv.get_left()) == kv.get_right());
    }
    for (const auto &kv : gnd_truth_2) {
      VERIFY(gnd_truth_2.at(kv.get_left()) == kv.get_right());
    }

  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
}

void TestEqualRange() {
  using std::string_view_literals::operator""sv;
  using namespace OmniSketch::Data;

  try {

    static constexpr std::string_view input = R"(
        name = [["flowkey", "length", "padding"], [8, 2, 6]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());

    char name[L_tmpnam];
    std::tmpnam(name);

    const int64_t flowkey[12] = {0x1F1F1, 0x2F2F2, 0x1F1F1, 0x3F3F3,
                                 0x4F4F4, 0x1F1F1, 0x2F2F2, 0x3F3F3,
                                 0x5F5F5, 0x1F1F1, 0x5F5F5, 0x6F6F6};
    const int16_t length[12] = {0x1, 0x2, 0x1, 0x1, 0x5, 0x1,
                                0x3, 0x3, 0x2, 0x1, 0x2, 0x5};
    std::unordered_map<int64_t, std::pair<int64_t, int64_t>> map;

    char content[192];
    for (int i = 0; i < 12; ++i) {
      *reinterpret_cast<int64_t *>(content + 16 * i) = flowkey[i];
      *reinterpret_cast<int16_t *>(content + 16 * i + 8) = length[i];
      auto tmp = map[flowkey[i]];
      map[flowkey[i]] = {tmp.first + length[i], tmp.second + 1};
    }
    std::ofstream fout(name, std::ios::binary);
    fout.write(content, sizeof(content));
    fout.close();

    StreamData<8> data(name, format);
    VERIFY(data.succeed() == true);
    std::remove(name);

    GndTruth<8, int64_t> gnd_truth_1, gnd_truth_2;
    gnd_truth_1.getGroundTruth(data.begin(), data.end(), InLength);
    gnd_truth_2.getGroundTruth(data.begin(), data.end(), InPacket);
    auto pair = gnd_truth_1.equalRange(4);
    VERIFY(gnd_truth_1.totalValue() == 27);
    VERIFY(gnd_truth_2.totalValue() == 12);

    std::set<int64_t> tmp1, tmp2 = {0x1F1F1, 0x3F3F3, 0x5F5F5};
    VERIFY(pair.second - pair.first == tmp2.size());
    for (auto ptr = pair.first; ptr != pair.second; ++ptr) {
      tmp1.insert(*reinterpret_cast<const int64_t *>(ptr->get_left().cKey()));
    }
    VERIFY(tmp1 == tmp2);

    pair = gnd_truth_1.equalRange(5);
    tmp1.clear();
    tmp2 = {0x2F2F2, 0x4F4F4, 0x6F6F6};
    VERIFY(pair.second - pair.first == tmp2.size());
    for (auto ptr = pair.first; ptr != pair.second; ++ptr) {
      tmp1.insert(*reinterpret_cast<const int64_t *>(ptr->get_left().cKey()));
    }
    VERIFY(tmp1 == tmp2);

    pair = gnd_truth_2.equalRange(4);
    VERIFY(pair.second - pair.first == 1 &&
           *reinterpret_cast<const int64_t *>(pair.first->get_left().cKey()) ==
               0x1F1F1);
    pair = gnd_truth_2.equalRange(1);
    tmp1.clear();
    tmp2 = {0x4F4F4, 0x6F6F6};
    VERIFY(pair.second - pair.first == tmp2.size());
    for (auto ptr = pair.first; ptr != pair.second; ++ptr) {
      tmp1.insert(*reinterpret_cast<const int64_t *>(ptr->get_left().cKey()));
    }
    VERIFY(tmp1 == tmp2);
    pair = gnd_truth_2.equalRange(3);
    VERIFY(pair.first == pair.second);
    pair = gnd_truth_2.equalRange(0);
    VERIFY(pair.first == pair.second);
    pair = gnd_truth_2.equalRange(5);
    VERIFY(pair.first == pair.second);
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
}

void TestHeavyHitter() {
  using std::string_view_literals::operator""sv;
  using namespace OmniSketch::Data;

  // Threshold
  try {
    static constexpr std::string_view input = R"(
        name = [["flowkey", "length"], [4, 4]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());

    char name[L_tmpnam];
    std::tmpnam(name);

    const int32_t flowkey[22] = {0xa, 0x3, 0x8, 0x8, 0x8, 0x8, 0x1, 0x5,
                                 0x5, 0x2, 0x5, 0x9, 0x1, 0x4, 0x4, 0x5,
                                 0x8, 0x1, 0x2, 0x5, 0x6, 0x7};
    const int32_t length[22] = {0x1, 0x8, 0x4, 0x1, 0x3, 0x2, 0x9, 0x6,
                                0x6, 0x1, 0x6, 0x2, 0x2, 0x3, 0x2, 0x6,
                                0xa, 0x9, 0x9, 0x6, 0x2, 0x2};
    std::unordered_map<int32_t, std::pair<int32_t, int32_t>> map, ans, gave;

    char content[176];
    for (int i = 0; i < 22; ++i) {
      *reinterpret_cast<int32_t *>(content + 8 * i) = flowkey[i];
      *reinterpret_cast<int32_t *>(content + 8 * i + 4) = length[i];
      auto tmp = map[flowkey[i]];
      map[flowkey[i]] = {tmp.first + length[i], tmp.second + 1};
    }

    std::ofstream fout(name, std::ios::binary);
    fout.write(content, sizeof(content));
    fout.close();

    StreamData<4> data(name, format);
    VERIFY(data.succeed() == true);
    std::remove(name);

    for (int thres = 0; thres <= 100; ++thres) {
      ans.clear();
      gave.clear();
      GndTruth<4, int32_t> gnd_truth;
      gnd_truth.getHeavyHitter(data.begin(), data.end(), InLength,
                               thres / 100.0, Percentile);
      int64_t total = 0;
      for (const auto &kv : map) {
        if (kv.second.first > thres) {
          ans.insert(kv);
          ans[kv.first].second = 0;
          total += kv.second.first;
        }
      }
      VERIFY(gnd_truth.size() == ans.size());
      for (auto &kv : gnd_truth) {
        gave[*reinterpret_cast<const int32_t *>(kv.get_left().cKey())] =
            std::pair<int32_t, int32_t>(kv.get_right(), 0);
      }
      VERIFY(ans == gave);
      VERIFY(gnd_truth.totalValue() == total);
    }
    for (int thres = 0; thres <= 22; ++thres) {
      ans.clear();
      gave.clear();
      GndTruth<4, int32_t> gnd_truth;
      gnd_truth.getHeavyHitter(data.begin(), data.end(), InPacket, thres / 22.0,
                               Percentile);
      int64_t total = 0;
      for (const auto &kv : map) {
        if (kv.second.second > thres) {
          ans.insert(kv);
          ans[kv.first].first = 0;
          total += kv.second.second;
        }
      }
      VERIFY(gnd_truth.size() == ans.size());
      for (auto &kv : gnd_truth) {
        gave[*reinterpret_cast<const int32_t *>(kv.get_left().cKey())] =
            std::pair<int32_t, int32_t>(0, kv.get_right());
      }
      VERIFY(ans == gave);
      VERIFY(gnd_truth.totalValue() == total);
    }
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
  try {
    static constexpr std::string_view input = R"(
        name = [["flowkey", "padding", "length"], [8, 4, 4]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());

    char name[L_tmpnam];
    std::tmpnam(name);

    const int64_t flowkey[32] = {0x1, 0x3, 0x8, 0xa, 0x8, 0xa, 0x1, 0x5,
                                 0x5, 0x2, 0x5, 0x9, 0x1, 0x4, 0x4, 0x6,
                                 0x8, 0x1, 0x2, 0xa, 0x6, 0x7, 0x1, 0x3,
                                 0x3, 0x3, 0x4, 0x4, 0x7, 0x7, 0x7, 0x7};
    const int32_t length[32] = {0x1, 0x1,  0x1,  0x1,   0x1,  0x1, 0x1, 0x1,
                                0x1, 0x10, 0x1,  0x100, 0x1,  0x1, 0x1, 0x10,
                                0x1, 0x1,  0x10, 0x1,   0x10, 0x1, 0x1, 0x1,
                                0x1, 0x1,  0x1,  0x1,   0x1,  0x1, 0x1, 0x1};
    std::unordered_map<int64_t, int32_t> map, ans, gave;

    char content[512];
    for (int i = 0; i < 32; ++i) {
      *reinterpret_cast<int64_t *>(content + 16 * i) = flowkey[i];
      *reinterpret_cast<int32_t *>(content + 16 * i + 12) = length[i];
      map[flowkey[i]] += 1;
    }

    std::ofstream fout(name, std::ios::binary);
    fout.write(content, sizeof(content));
    fout.close();

    StreamData<8> data(name, format);
    VERIFY(data.succeed() == true);
    std::remove(name);

    GndTruth<8, int32_t> gnd_truth;
    gnd_truth.getGroundTruth(data.begin(), data.end(), InPacket);
    VERIFY(gnd_truth.totalValue() == 32);

    for (int thres = 0; thres <= 32; ++thres) {
      ans.clear();
      gave.clear();
      GndTruth<8, int32_t> hh;
      hh.getHeavyHitter(gnd_truth, thres / 32.0, Percentile);
      int64_t total = 0;

      for (const auto &kv : map) {
        if (kv.second > thres) {
          ans.insert(kv);
          total += kv.second;
        }
      }
      VERIFY(ans.size() == hh.size());
      VERIFY(hh.totalValue() == total);
      for (const auto &kv : hh) {
        gave[*reinterpret_cast<const int64_t *>(kv.get_left().cKey())] =
            kv.get_right();
      }
      VERIFY(ans == gave);
    }
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }

  // TopK
  try {
    static constexpr std::string_view input = R"(
        name = [["flowkey", "padding", "length"], [8, 4, 4]]
    )"sv;
    toml::table array = toml::parse(input);
    DataFormat format(*array["name"].as_array());

    char name[L_tmpnam];
    std::tmpnam(name);

    const int64_t flowkey[32] = {0x1, 0x3, 0x8, 0xa, 0x8, 0xa, 0x1, 0x5,
                                 0x5, 0x2, 0x5, 0x9, 0x1, 0x4, 0x4, 0x6,
                                 0x8, 0x1, 0x2, 0xa, 0x6, 0x7, 0x1, 0x3,
                                 0x3, 0x3, 0x4, 0x4, 0x7, 0x7, 0x7, 0x7};
    const int32_t length[32] = {0x1, 0x1,  0x1,  0x1,   0x1,  0x1, 0x1, 0x1,
                                0x1, 0x10, 0x1,  0x100, 0x1,  0x1, 0x1, 0x10,
                                0x1, 0x1,  0x10, 0x1,   0x10, 0x1, 0x1, 0x1,
                                0x1, 0x1,  0x1,  0x1,   0x1,  0x1, 0x1, 0x1};
    std::unordered_map<int64_t, int32_t> map, ans, gave;

    char content[512];
    for (int i = 0; i < 32; ++i) {
      *reinterpret_cast<int64_t *>(content + 16 * i) = flowkey[i];
      *reinterpret_cast<int32_t *>(content + 16 * i + 12) = length[i];
      map[flowkey[i]] += 1;
    }

    std::ofstream fout(name, std::ios::binary);
    fout.write(content, sizeof(content));
    fout.close();

    StreamData<8> data(name, format);
    VERIFY(data.succeed() == true);
    std::remove(name);

    GndTruth<8, int32_t> gnd_truth;
    gnd_truth.getGroundTruth(data.begin(), data.end(), InPacket);
    int64_t currrent = std::numeric_limits<int64_t>::max();
    for (const auto &kv : gnd_truth) {
      VERIFY(currrent >= kv.get_right());
      currrent = kv.get_right();
    }

    GndTruth<8, int32_t> hh_1, hh_2, hh_3;
    hh_1.getHeavyHitter(gnd_truth, 2, TopK);
    VERIFY(hh_1.size() == 2);
    ans[0x1] = 5;
    ans[0x7] = 5;
    for (const auto &kv : hh_1) {
      gave[*reinterpret_cast<const int64_t *>(kv.get_left().cKey())] =
          kv.get_right();
    }
    VERIFY(ans == gave);
    VERIFY(hh_1.totalValue() == 10);
    currrent = std::numeric_limits<int64_t>::max();
    for (const auto &kv : hh_1) {
      VERIFY(currrent >= kv.get_right());
      currrent = kv.get_right();
    }

    hh_2.getHeavyHitter(gnd_truth, 4, TopK);
    VERIFY(hh_2.size() == 4);
    ans[0x3] = 4;
    ans[0x4] = 4;
    gave.clear();
    for (const auto &kv : hh_2) {
      gave[*reinterpret_cast<const int64_t *>(kv.get_left().cKey())] =
          kv.get_right();
    }
    VERIFY(ans == gave);
    VERIFY(hh_2.totalValue() == 18);
    currrent = std::numeric_limits<int64_t>::max();
    for (const auto &kv : hh_2) {
      VERIFY(currrent >= kv.get_right());
      currrent = kv.get_right();
    }

    hh_3.getHeavyHitter(gnd_truth, 7, TopK);
    VERIFY(hh_3.size() == 7);
    ans[0x5] = 3;
    ans[0x8] = 3;
    ans[0xa] = 3;
    gave.clear();
    for (const auto &kv : hh_3) {
      gave[*reinterpret_cast<const int64_t *>(kv.get_left().cKey())] =
          kv.get_right();
    }
    VERIFY(ans == gave);
    VERIFY(hh_3.totalValue() == 27);
    currrent = std::numeric_limits<int64_t>::max();
    for (const auto &kv : hh_3) {
      VERIFY(currrent >= kv.get_right());
      currrent = kv.get_right();
    }
  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
}

void TestHeavyChanger() {
  using std::string_view_literals::operator""sv;
  using namespace OmniSketch::Data;

  try {
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
    std::unordered_map<int32_t, int32_t> map = {{0x1, 1}, {0x2, 0}, {0x3, 2},
                                                {0x4, 0}, {0x5, 3}, {0x6, 0},
                                                {0x7, 5}, {0x8, 1}, {0x9, 1},
                                                {0xa, 1}},
                                         ans, gave;

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

    for (int thres = 0; thres <= 14; ++thres) {
      GndTruth<4, int32_t> gnd_truth_1;
      gnd_truth_1.getHeavyChanger(data.begin(), data.diff(16), data.diff(16),
                                  data.end(), InPacket, thres / 14.0,
                                  Percentile);
      ans.clear();
      gave.clear();
      for (const auto &kv : map) {
        if (kv.second > thres)
          ans.insert(kv);
      }
      VERIFY(ans.size() == gnd_truth_1.size());
      for (const auto &kv : gnd_truth_1) {
        gave[*reinterpret_cast<const int32_t *>(kv.get_left().cKey())] =
            kv.get_right();
      }
    }
    VERIFY(ans == gave);

    GndTruth<4, int32_t> gnd_truth_1, gnd_truth_2, gnd_truth_3;
    gnd_truth_1.getGroundTruth(data.begin(), data.diff(16), InPacket);
    gnd_truth_2.getGroundTruth(data.diff(16), data.end(), InPacket);

    VERIFY(gnd_truth_1.size() == 9);
    VERIFY(gnd_truth_1.totalValue() == 16);
    VERIFY(gnd_truth_2.totalValue() == 16);
    gnd_truth_3.getHeavyChanger(std::move(gnd_truth_1), std::move(gnd_truth_2),
                                3, TopK);
    ans = std::unordered_map<int32_t, int32_t>({{0x3, 2}, {0x5, 3}, {0x7, 5}});
    VERIFY(gnd_truth_3.size() == ans.size());
    VERIFY(gnd_truth_1.size() == 0);
    gave.clear();

    for (const auto &kv : gnd_truth_3) {
      gave[*reinterpret_cast<const int32_t *>(kv.get_left().cKey())] =
          kv.get_right();
    }
    VERIFY(gave == ans);
    VERIFY(gnd_truth_1.totalValue() == 0);
    VERIFY(gnd_truth_2.totalValue() == 16);
    VERIFY(gnd_truth_3.totalValue() == 10);

    GndTruth<4, int32_t> gnd_truth_4, gnd_truth_5, gnd_truth_6;
    gnd_truth_4.getGroundTruth(data.begin(), data.diff(16), InPacket);
    gnd_truth_5.getGroundTruth(data.diff(16), data.end(), InPacket);
    VERIFY(gnd_truth_4.size() == 9);
    gnd_truth_6.getHeavyChanger(gnd_truth_4, gnd_truth_5, 7, TopK);
    ans = std::unordered_map<int32_t, int32_t>(
        {{0x3, 2}, {0x5, 3}, {0x7, 5}, {0x1, 1}, {0x8, 1}, {0x9, 1}, {0xa, 1}});
    VERIFY(gnd_truth_6.size() == ans.size());
    VERIFY(gnd_truth_4.size() == 9);
    gave.clear();
    for (const auto &kv : gnd_truth_6) {
      gave[*reinterpret_cast<const int32_t *>(kv.get_left().cKey())] =
          kv.get_right();
    }
    VERIFY(gave == ans);
    VERIFY(gnd_truth_4.totalValue() == 16);
    VERIFY(gnd_truth_5.totalValue() == 16);
    VERIFY(gnd_truth_6.totalValue() == 14);

  } catch (const std::exception &exp) {
    VERIFY_NO_EXCEPTION(exp);
  }
}

void TestEstimation() {
  using OmniSketch::FlowKey;
  using OmniSketch::Data::Estimation;

  Estimation<4, int32_t> estimate;
  FlowKey<4> key_1(1), key_2(2), key_3(3), key_4(4);
  estimate[key_1] += 100;
  VERIFY(estimate[key_1] == 100);
  VERIFY(estimate.at(key_1) == 100);
  VERIFY(estimate[key_1] == 100);
  VERIFY(estimate.size() == 1);
  VERIFY(estimate.count(key_4) == 0);
  estimate.insert(key_2);
  VERIFY(estimate.at(key_2) == 0);
  VERIFY(estimate.update(key_2, 3) == false);
  VERIFY(estimate[key_2] == 3);
  estimate.update(key_3, 2022);
  VERIFY(estimate.size() == 3);
  VERIFY(estimate.insert(key_3) == false);
  VERIFY(estimate[key_3] == 2022);
  VERIFY(estimate.count(key_4) == 0);
  estimate[key_4];
  VERIFY(estimate.size() == 4);
  VERIFY(estimate.count(key_4) == 1);
  VERIFY(estimate[key_4] == 0);
}

OmniSketch::Data::Estimation<4> ReturnAnEstimation(int32_t value) {
  using OmniSketch::FlowKey;
  using OmniSketch::Data::Estimation;

  static int32_t call = 1;

  Estimation<4> estimate;
  FlowKey<4> key_1(1), key_2(2), key_3(3), key_4(4);
  estimate[key_1] += value;
  estimate.update(key_2, 2022);
  estimate[key_3];
  estimate[key_4] = call;

  call++;

  return estimate;
}

OmniSketch::Data::GndTruth<4> ReturnAGroundTruth(int32_t value) {
  using std::string_view_literals::operator""sv;
  using namespace OmniSketch::Data;

  static int32_t call = 1;
  static constexpr std::string_view input = R"(
        name = [["flowkey", "length", "padding", "timestamp"], [4, 4, 2, 2]]
    )"sv;
  toml::table array = toml::parse(input);
  DataFormat format(*array["name"].as_array());

  char name[L_tmpnam];
  std::tmpnam(name);

  const int32_t flowkey[10] = {0x1F1F1, 0x2F2F2, 0x1F1F1, 0x3F3F3, 0x4F4F4,
                               0x1F1F1, 0x2F2F2, 0x3F3F3, 0x5F5F5, 0x1F1F1};
  const int32_t length[10] = {0x1,  0x2,  0x4,  call,  0x10,
                              0x20, 0x40, 0x80, value, 0x200};
  call++;
  const int16_t timestamp[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::unordered_map<int32_t, std::pair<int64_t, int64_t>> map;

  char content[120];
  for (int i = 0; i < 10; ++i) {
    *reinterpret_cast<int32_t *>(content + 12 * i) = flowkey[i];
    *reinterpret_cast<int32_t *>(content + 12 * i + 4) = length[i];
    *reinterpret_cast<int16_t *>(content + 12 * i + 10) = timestamp[i];
    auto tmp = map[flowkey[i]];
    map[flowkey[i]] = {tmp.first + length[i], tmp.second + 1};
  }
  std::ofstream fout(name, std::ios::binary);
  fout.write(content, sizeof(content));
  fout.close();

  StreamData<4> data(name, format);
  std::remove(name);

  VERIFY(data.succeed() == true);
  VERIFY(data.empty() == false);
  VERIFY(data.size() == 10);

  for (int i = 0; i < 10; ++i) {
    VERIFY(data.diff(i)->length == length[i]);
    VERIFY(*reinterpret_cast<const int32_t *>(data.diff(i)->flowkey.cKey()) ==
           flowkey[i]);
    VERIFY(data.diff(i)->timestamp == timestamp[i]);
  }
  VERIFY(data.begin() == data.diff(0));
  VERIFY(data.end() == data.diff(data.size()));

  GndTruth<4, int64_t> gnd_truth;
  VERIFY(gnd_truth.empty());
  gnd_truth.getGroundTruth(data.begin(), data.end(), InLength);
  VERIFY(!gnd_truth.empty());

  return gnd_truth;
}

void TestMovable() {
  using OmniSketch::FlowKey;
  using OmniSketch::Data::Estimation;
  using OmniSketch::Data::GndTruth;

  int32_t value = 114514;
  static int32_t counter = 1;

  Estimation<4> est = ReturnAnEstimation(value);
  VERIFY(est[FlowKey<4>(1)] == value);
  VERIFY(est[FlowKey<4>(2)] == 2022);
  VERIFY(est[FlowKey<4>(3)] == 0);
  VERIFY(est[FlowKey<4>(4)] == counter);
  VERIFY(est.size() == 4);
  Estimation<4> est_2 = std::move(est);

  int32_t iteration = 0;
  for (const auto &kv : est_2) {
    switch (kv.get_left().getIp()) {
    case 1:
      VERIFY(kv.get_right() == value);
      break;
    case 2:
      VERIFY(kv.get_right() == 2022);
      break;
    case 3:
      VERIFY(kv.get_right() == 0);
      break;
    case 4:
      VERIFY(kv.get_right() == counter);
      break;
    default:
      VERIFY(false);
    }
    iteration++;
  }
  VERIFY(iteration == 4);
  VERIFY(est_2.size() == 4);

  GndTruth<4> gnd = ReturnAGroundTruth(value);
  VERIFY(gnd.size() == 5);
  std::unordered_map<int32_t, int64_t> map = {{0x1F1F1, 0x225},
                                              {0x2F2F2, 0x42},
                                              {0x3F3F3, counter + 0x80},
                                              {0x4F4F4, 0x10},
                                              {0x5F5F5, value}};
  for (const auto &kv : gnd) {
    VERIFY(map.at(kv.get_left().getIp()) == kv.get_right());
  }
  VERIFY(gnd.size() == 5);
  GndTruth<4> gnd_2 = std::move(gnd);
  for (const auto &kv : gnd_2) {
    VERIFY(map.at(kv.get_left().getIp()) == kv.get_right());
  }
  VERIFY(gnd_2.size() == 5);
  counter++;
}

OMNISKETCH_DECLARE_TEST(data) {
  for (int i = 0; i < g_repeat; i++) {
    TestDataFormat();
    TestGndTruth();
    TestEqualRange();
    TestHeavyHitter();
    TestHeavyChanger();
    TestEstimation();
    TestMovable();
  }
}

/** @endcond */