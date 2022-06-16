/**
 * @file sketch.h
 * @author dromniscience (you@domain.com)
 * @brief Base sketch
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

// A bunch of files to include!
#include "data.h"

/**
 * @brief Warehouse of sketches
 *
 */
namespace OmniSketch::Sketch {

/**
 * @brief Base sketch
 *
 * @details This class is provided for the sake of testing. By virtue of
 * polymorphism, testing class is able to call overriden methods in subclasses.
 * Thus, it is suggested that you put **override** specifier at your
 * overriden method in the derived class (cf. files in `sketch/` folder). If the
 * signature mismatches, Test::TestBase may complain by prompting an error.
 *
 * @tparam T        type of the counter
 * @tparam key_len  length of flowkey
 *
 * @attention Write your method in subclass with the same signature if it
 * is functionally equivalent to one of the following:
 * <table>
 *    <tr>
 *        <td><b>Functional Description</b></td>
 *        <td><b>Partial Signature (Click the links to see more)</b></td>
 *   </tr>
 *   <tr>
 *        <td>query sketch size</td>
 *        <td>size() const</td>
 *   </tr>
 *   <tr>
 *        <td>insert flowkey without value</td>
 *        <td>insert(const FlowKey<key_len> &)</td>
 *   </tr>
 *   <tr>
 *        <td>insert flowkey with value</td>
 *        <td>update(const FlowKey<key_len> &, T)</td>
 *   </tr>
 *   <tr>
 *        <td>look up a flowkey (*if exists*)</td>
 *        <td>lookup(const FlowKey<key_len> &) const</td>
 *   </tr>
 *   <tr>
 *        <td>heavy hitter</td>
 *        <td>getHeavyHitter(double) const</td>
 *   </tr>
 *   <tr>
 *     <td>heavy changer</td>
 *     <td>
 * getHeavyChanger(std::unique_ptr<SketchBase<key_len,T>> &, double) const
 *     </td>
 *   </tr>
 *   <tr>
 *        <td>decode flowkeys with values</td>
 *        <td>decode()</td>
 *   </tr>
 * </table>
 *
 */
template <int32_t key_len, typename T = int64_t> class SketchBase {
public:
  /**
   * @brief Return the size of the sketch
   *
   */
  virtual size_t size() const {
    static bool emit = false; // avoid burst of LOG
    if (!emit) {
      LOG(ERROR, "Erroneously called SketchBase::size() const.");
      emit = true;
    }
    return 0;
  }
  /**
   * @brief Insert a flowkey without value
   *
   */
  virtual void insert(const FlowKey<key_len> &flowkey) {
    static bool emit = false; // avoid burst of LOG
    if (!emit) {
      LOG(ERROR, "Erroneously called SketchBase::insert(const FlowKey &).");
      emit = true;
    }
    return;
  }
  /**
   * @brief Update a flowkey with certain value
   *
   */
  virtual void update(const FlowKey<key_len> &flowkey, T value) {
    static bool emit = false; // avoid burst of LOG
    if (!emit) {
      LOG(ERROR, "Erroneously called SketchBase::update(const FlowKey &, T).");
      emit = true;
    }
    return;
  }
  /**
   * @brief Query the sketch for the estimated size of a flowkey
   *
   */
  virtual T query(const FlowKey<key_len> &flowkey) const {
    static bool emit = false; // avoid burst of LOG
    if (!emit) {
      LOG(ERROR, "Erroneously called SketchBase::query(const FlowKey &).");
      emit = true;
    }
    return 0;
  }
  /**
   * @brief Look up a flowkey in the sketch
   * @return `true` means there exists; `false` otherwise.
   */
  virtual bool lookup(const FlowKey<key_len> &flowkey) const {
    static bool emit = false; // avoid burst of LOG
    if (!emit) {
      LOG(ERROR, "Erroneously called SketchBase::lookup(const FlowKey &).");
      emit = true;
    }
    return false;
  }
  /**
   * @brief Get all the heavy hitters
   * @return See Data::Estimation for more info.
   */
  virtual Data::Estimation<key_len, T> getHeavyHitter(double threshold) const {
    static bool emit = false; // avoid burst of LOG
    if (!emit) {
      LOG(ERROR, "Erroneously called SketchBase::getHeavyHitter(double).");
      emit = true;
    }
    return {};
  }
  /**
   * @brief Get all the heavy changers
   * @return See Data::Estimation for more info.
   */
  virtual Data::Estimation<key_len, T>
  getHeavyChanger(std::unique_ptr<SketchBase<key_len, T>> &ptr_sketch,
                  double threshold) const {
    static bool emit = false; // avoid burst of LOG
    if (!emit) {
      LOG(ERROR, "Erroneously called SketchBase::getHeavyChanger(double).");
      emit = true;
    }
    return {};
  }
  /**
   * @brief Decode all flowkeys along with their values
   * @return An Estimation that contains all decoded flowkeys with estimated
   * value. Particularly useful for reversible sketches.
   *
   */
  virtual Data::Estimation<key_len, T> decode() {
    static bool emit = false;
    if (!emit) {
      LOG(ERROR, "Erroneously called SketchBase::decode().");
      emit = true;
    }
    return {};
  }
};

} // namespace OmniSketch::Sketch