/**
 * @file parser.h
 * @author dromniscience (you@domain.com)
 * @brief Class for pcap parser
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <IPv4Layer.h>
#include <IcmpLayer.h>
#include <Packet.h>
#include <PcapFileDevice.h>
#include <PcapPlusPlusVersion.h>
#include <RawPacket.h>
#include <SystemUtils.h>
#include <TcpLayer.h>
#include <UdpLayer.h>
#include <common/data.h>
#include <iostream>

/**
 * @todo Support "txt", "null" & "pcap" mode
 *
 */

namespace OmniSketch::Util {
/**
 * @brief Pcap Parser
 *
 * @tparam key_len length of flowkey
 */
template <int32_t key_len> class PcapParser {
  /**
   * @brief Verbose level
   *
   *  |Level        |Explanation              |
   *  |:------------|:------------------------|
   *  |0            |Errors messages          |
   *  |1            |+ Pcap file info         |
   *  |2            |+ Per packet info        |
   *
   */
  const int32_t verbose_level;
  /**
   * @brief Type of output
   *
   */
  enum Mode { NULLY, BINARY, TXT, PCAP } mode;
  /**
   * @brief Output format (Only used in txt and binary mode)
   *
   */
  Data::DataFormat *format;
  /**
   * @brief Pcap reader
   *
   */
  pcpp::IFileReaderDevice *reader;
  /**
   * @brief Whether the parser config is successfully parsed
   *
   */
  bool is_succeed;
  /**
   * @brief Name of input
   *
   */
  std::string input_pcap;
  /**
   * @brief Name of output
   *
   */
  std::string output_pcap;
  /**
   * @brief Filter
   *
   */
  std::string filter;
  /**
   * @brief Packet count (-1 means no upper bound)
   *
   */
  int32_t packet_count;
  /**
   * @brief Flow count (-1 means no upper bound)
   *
   */
  int32_t flow_count;
  /**
   * @brief Print file summary
   *
   */
  void printFileSummary() const;
  /**
   * @brief Get the Link Layer Type
   *
   */
  pcpp::LinkLayerType getLinkLayerType() const;

public:
  /**
   * @brief Open the config file
   *
   * @param config_file path to the config file
   * @param parser_path path into the file (concatenated with '.')
   * @param verbose     verbose level
   */
  PcapParser(const std::string_view &config_file,
             const std::string_view &parser_path =
                 std::string_view_literals::operator""sv("parser", 6),
             const int32_t verbose = 0);
  /**
   * @brief Return whether the parser config is successfully parsed
   *
   */
  bool succeed() const { return is_succeed; }
  /**
   * @brief Release the resources
   *
   */
  ~PcapParser();
  /**
   * @brief Dump the pcap/snoop packets in binary
   *
   * @return Number of packets parsed (exclude filtered packet)
   */
  int32_t dumpPcapPacketsInBinary() const;
  /**
   * @brief Dump the pcap/snoop packets in txt
   *
   * @return Number of packets parsed (exclude filtered packet)
   */
  int32_t dumpPcapPacketInPcap() const;
};

} // namespace OmniSketch::Util

//-----------------------------------------------------------------------------
//
///                        Implementation of templated methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Util {

template <int32_t key_len>
PcapParser<key_len>::PcapParser(const std::string_view &config_file,
                                const std::string_view &parser_path,
                                const int32_t verbose)
    : verbose_level(verbose), is_succeed(true), reader(nullptr),
      format(nullptr), packet_count(-1), flow_count(-1) {
  // Parse config
  ConfigParser parser(config_file);
  if (!parser.succeed()) {
    is_succeed = false;
    return;
  }

  parser.setWorkingNode(parser_path);

  // Indispensable configs
  if (!parser.parseConfig(input_pcap, "input")) {
    is_succeed = false;
    input_pcap = "";
  }
  if (!parser.parseConfig(output_pcap, "output")) {
    is_succeed = false;
    output_pcap = "";
  }
  std::string output_mode;
  if (!parser.parseConfig(output_mode, "mode")) {
    is_succeed = false;
  }

  if (output_mode == "null") {
    mode = NULLY;
  } else if (output_mode == "binary") {
    mode = BINARY;
  } else if (output_mode == "txt") {
    mode = TXT;
  } else if (output_mode == "pcap") {
    mode = PCAP;
  } else {
    LOG(ERROR, fmt::format("{}: \"mode\" should be one of the \"null\", "
                           "\"binary\", \"txt\", \"pcap\", but got {} instead.",
                           config_file, output_mode));
    is_succeed = false;
  }
  if (!is_succeed)
    return;
  if (mode != NULLY && output_pcap == "") {
    LOG(ERROR, fmt::format("Output file cannot be empty."));
    is_succeed = false;
    return;
  }
  if (mode == BINARY || mode == TXT) {
    toml::array array;
    if (!parser.parseConfig(array, "format")) {
      is_succeed = false;
      return;
    }
    try {
      format = new Data::DataFormat(array);
      if (key_len != format->getKeyLength()) {
        throw std::runtime_error("Runtime Error: Key length in \"format\" is " +
                                 std::to_string(format->getKeyLength()) +
                                 " yet in the template it is " +
                                 std::to_string(key_len) + ".");
      }
    } catch (const std::runtime_error &exp) {
      LOG(ERROR, exp.what());
      is_succeed = false;
      return;
    }
  }

  // Optional
  if (!parser.parseConfig(packet_count, "packet_count", false)) {
    packet_count = -1;
  }
  if (!parser.parseConfig(flow_count, "flow_count", false)) {
    flow_count = -1;
  }
  if (!parser.parseConfig(filter, "filter", false)) {
    filter = "";
  }

  // open pcap file
  reader = pcpp::IFileReaderDevice::getReader(input_pcap);

  if (!reader->open()) {
    delete reader;
    reader = nullptr;
    LOG(ERROR, fmt::format("Error opening input pcap file {}\n", input_pcap));
    is_succeed = false;
    return;
  }
  // set a filter if provided
  if (filter != "") {
    if (!reader->setFilter(filter)) {
      delete reader;
      reader = nullptr;
      LOG(ERROR, fmt::format("Couldn't set filter '{}'", filter));
      is_succeed = false;
      return;
    }
  }
  // print file summary
  if (verbose_level > 0) {
    printFileSummary();
  }
}

template <int32_t key_len> PcapParser<key_len>::~PcapParser() {
  if (format)
    delete format;
  if (reader)
    delete reader;
}

template <int32_t key_len> void PcapParser<key_len>::printFileSummary() const {
  if (!reader) {
    throw std::runtime_error("Runtime Error: No pcap file is opened.");
  }
  auto linkLayerToString = [](pcpp::LinkLayerType linkLayer) -> std::string {
    if (linkLayer == pcpp::LINKTYPE_ETHERNET)
      return "Ethernet";
    if (linkLayer == pcpp::LINKTYPE_IEEE802_5)
      return "IEEE 802.5 Token Ring";
    else if (linkLayer == pcpp::LINKTYPE_LINUX_SLL)
      return "Linux cooked capture";
    else if (linkLayer == pcpp::LINKTYPE_NULL)
      return "Null/Loopback";
    else if (linkLayer == pcpp::LINKTYPE_RAW ||
             linkLayer == pcpp::LINKTYPE_DLT_RAW1 ||
             linkLayer == pcpp::LINKTYPE_DLT_RAW2) {
      std::ostringstream stream;
      stream << "Raw IP (" << linkLayer << ")";
      return stream.str();
    }

    std::ostringstream stream;
    stream << (int)linkLayer;
    return stream.str();
  };

  std::cout << "File summary:" << std::endl;
  std::cout << "   File name: " << reader->getFileName() << std::endl;
  std::cout << "   File size: " << reader->getFileSize() << " bytes"
            << std::endl;

  if (dynamic_cast<pcpp::PcapFileReaderDevice *>(reader)) {
    pcpp::PcapFileReaderDevice *pcapReader =
        dynamic_cast<pcpp::PcapFileReaderDevice *>(reader);
    pcpp::LinkLayerType linkLayer = pcapReader->getLinkLayerType();
    std::cout << "   Link layer type: " << linkLayerToString(linkLayer)
              << std::endl;
  } else if (dynamic_cast<pcpp::SnoopFileReaderDevice *>(reader) != NULL) {
    pcpp::SnoopFileReaderDevice *snoopReader =
        dynamic_cast<pcpp::SnoopFileReaderDevice *>(reader);
    pcpp::LinkLayerType linkLayer = snoopReader->getLinkLayerType();
    std::cout << "   Link layer type: " << linkLayerToString(linkLayer)
              << std::endl;
  } else if (dynamic_cast<pcpp::PcapNgFileReaderDevice *>(reader) != NULL) {
    pcpp::PcapNgFileReaderDevice *pcapNgReader =
        dynamic_cast<pcpp::PcapNgFileReaderDevice *>(reader);
    if (pcapNgReader->getOS() != "")
      std::cout << "   OS: " << pcapNgReader->getOS() << std::endl;
    if (pcapNgReader->getCaptureApplication() != "")
      std::cout << "   Capture application: "
                << pcapNgReader->getCaptureApplication() << std::endl;
    if (pcapNgReader->getCaptureFileComment() != "")
      std::cout << "   File comment: " << pcapNgReader->getCaptureFileComment()
                << std::endl;
    if (pcapNgReader->getHardware() != "")
      std::cout << "   Capture hardware: " << pcapNgReader->getHardware()
                << std::endl;
  }

  std::cout << std::endl;
}

template <int32_t key_len>
int32_t PcapParser<key_len>::dumpPcapPacketsInBinary() const {
  if (!reader) {
    throw std::runtime_error("Runtime Error: No pcap file is opened.");
  }
  if (!format) {
    throw std::runtime_error("Runtime Error: No format is specified.");
  }

  std::ofstream fout(output_pcap, std::ios::binary); // automatically destroyed
  if (!fout.is_open()) {
    throw std::runtime_error("Runtime Error: Could not open output file " +
                             output_pcap);
  }

  // packet count
  int32_t packet_count_so_far = 0;
  // flow sets
  Data::Estimation<key_len, int64_t> all_flows;

  pcpp::RawPacket raw_packet;
  while (reader->getNextPacket(raw_packet) &&
         packet_count_so_far != packet_count) {
    // parse the raw packet into a parsed packet
    pcpp::Packet parsed_packet(&raw_packet);
    // timestamp
    timespec time_nano = raw_packet.getPacketTimeStamp();
    uint64_t timestamp = time_nano.tv_sec * static_cast<uint64_t>(1000000) +
                         time_nano.tv_nsec / 1000;
    // IPv4 header
    pcpp::IPv4Layer *ip_layer = parsed_packet.getLayerOfType<pcpp::IPv4Layer>();
    if (!ip_layer)
      continue;

    uint32_t src_ip = Net2Host32(ip_layer->getSrcIPv4Address().toInt());
    uint32_t dst_ip = Net2Host32(ip_layer->getDstIPv4Address().toInt());

    uint8_t protocol;
    uint16_t src_port, dst_port;
    uint64_t length = Net2Host16(ip_layer->getIPv4Header()->totalLength);

    pcpp::TcpLayer *tcp_layer = parsed_packet.getLayerOfType<pcpp::TcpLayer>();
    pcpp::UdpLayer *udp_layer = parsed_packet.getLayerOfType<pcpp::UdpLayer>();

    if (tcp_layer) {
      src_port = tcp_layer->getSrcPort();
      dst_port = tcp_layer->getDstPort();
      protocol = tcp_layer->getProtocol();
    } else if (udp_layer) {
      src_port = udp_layer->getSrcPort();
      dst_port = udp_layer->getDstPort();
      protocol = udp_layer->getProtocol();
    } else if (key_len == 13) {
      continue;
    }

    FlowKey<key_len> *key_ptr = nullptr;
    if (key_len == 4) {
      key_ptr = new FlowKey<key_len>(src_ip);
    } else if (key_len == 8) {
      key_ptr = new FlowKey<key_len>(src_ip, dst_ip);
    } else if (key_len == 13) {
      key_ptr =
          new FlowKey<key_len>(src_ip, dst_ip, src_port, dst_port, protocol);
    }
    all_flows.insert(*key_ptr);

    // deserialize flow key
    OmniSketch::Data::Record<key_len> record;
    record.flowkey.copy(0, *key_ptr, 0, key_len);
    record.length = length;
    record.timestamp = timestamp;
    int8_t byte[format->getRecordLength()];
    format->writeAsFormat(record, byte);
    delete key_ptr;
    // flow count
    if (all_flows.size() == flow_count)
      goto finished;
    // write to file
    fout.write(reinterpret_cast<const char *>(byte), format->getRecordLength());

    // verbosity: per-packet info
    if (verbose_level > 1) {
      std::cout << "#" << packet_count_so_far << std::endl;
      std::cout << parsed_packet.toString() << std::endl;
    }

    packet_count_so_far += 1;
  }
finished:
  // verbosity: file info
  if (verbose_level > 0) {
    std::cout << "Finished. Printed " << packet_count_so_far << " packets ("
              << all_flows.size() << " flows)" << std::endl;
  }

  // return the number of packets that were printed
  return packet_count_so_far;
}

template <int32_t key_len>
int32_t PcapParser<key_len>::dumpPcapPacketInPcap() const {
  if (!reader) {
    throw std::runtime_error("Runtime Error: No pcap file is opened.");
  }
}

} // namespace OmniSketch::Util