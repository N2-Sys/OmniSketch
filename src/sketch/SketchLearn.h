/**
 * @file SketchLearn.h
 * @author Shibo Yang<yangshibo@stu.pku.edu.cn>
 * @brief Sketch Learn
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include <common/hash.h>
#include <common/sketch.h>
#include <vector>
#include <algorithm>
#include <cmath>

namespace OmniSketch::Sketch {
/**
 * @brief Sketch Learn
 *
 * @tparam key_len  length of flowkey
 * @tparam hash_t   hashing class
 * @tparam T        type of the counter
 */
template <int32_t key_len, typename T,
          typename hash_t = Hash::AwareHash>
class SketchLearn : public SketchBase<key_len> {

public:
  /**
   * @brief The information learned by SketchLearn, which provides
   *        the estimated value and the probability array of the 
   *        specific flow
   *
   */
  class ans_t
  {
  public:
    uint8_t bit_flow[l + 2];        // the string describes the string, which consists of '0' and '1'
    uint8_t flow[(l + 7) / 8 + 1];  // the FlowKey represented by uint8_t array
    uint32_t size;                  // the size
    double_t prob_vector[l + 2];    // the probability array
    ans_t(uint8_t* bbit_flow, uint8_t* fflow, unsigned int32_t ssize = 0, double_t* pprob_vector = NULL)
    {
        strcpy((uint8_t *)bit_flow, (uint8_t *)bbit_flow);
        for (int32_t i = 0; i < key_len; i++)
        {
            flow[i] = fflow[i];
        }
        if (pprob_vector != NULL)
        {
            for (int32_t i = 1; i <= l; i++)
            {
                prob_vector[i] = pprob_vector[i];
            }
        }
        size = ssize;
    }
  };

  /**
   * @brief Two ways to represent a specific flow
   *
   */
  class two_types_of_flow
  {
  public:
    uint8_t bit_flow[l + 2];       // the string describes the string, which consists of '0' and '1'
    uint8_t flow[(l + 7) / 8 + 1]; // the FlowKey represented by uint8_t array
    two_types_of_flow(uint8_t* bbit_flow, uint8_t* fflow)
    {
        strcpy(bit_flow, bbit_flow);
        for (int32_t i = 0; i < (l + 7) / 8; i++)
        {
            flow[i] = fflow[i];
        }
        flow[(l + 7) / 8] = '\0';
    }
  };

private:

  /**
   * @brief sketch learn
   *
   * @param POSSIBLE_THRESHOLD  
   *         The threshold of hat_p, which is 0.99 as provided in the paper
   * 
   * @param STAR_THRESHOLD        
   *         If there are more than this many * in a regular expression, we 
   *         assume there is no large flow in the corresponding stack
   * 
   * @param MY_ERROR_THRESHOLD_SKETCH
   *         If the valuation is so many times higher than that of the value 
   *         in the smallest Sketch, that flow is considered likely to be a 
   *         false positive flow 
   * 
   * @param MY_ERROR_THRESHOLD_V0
   *         If the valuation is so many times higher than that of the total 
   *         value caught in V0 Sketch, that flow is considered likely to be 
   *         a false positive flow 
   * 
   * @param STEP
   *         The step size of the rate, which should decreases as theta 
   *         increases.
   *         Rate is used to determine whether to terminate the learning
   *         process.
   * 
   * @param START_THETA
   *         The initial value of theta
   * 
   * @param l
   *         The bit-level length of a flowkey
   *         We need l + 1 sketches for every bit of given flows and the 
   *         overall information of the given flows
   * 
   * @param r
   *         The depth of sketches
   * 
   * @param c 
   *         The width of sketches
   * 
   * @param V
   *         The sketches
   * 
   * @param p
   *         The array of mean values of Vk/V0
   * 
   * @param sigma
   *         The array of standard deviations of Vk/V0
   * 
   * @param updated
   *         To indicate whether some new information has been caught by
   *         the sketches.
   * 
   */
  
  static const double_t POSSIBLE_THRESHOLD = 0.99;           
  static const int32_t STAR_THRESHOLD = 11;                  
  static const double_t MY_ERROR_THRESHOLD_SKETCH = 2.0;     
  static const double_t MY_ERROR_THRESHOLD_V0 = 0.95;        
  static const double_t STEP = 0.005;                        
  static const double_t START_THETA = 0.5;                   

  static const int32_t l = 8 * key_len;                      
  int32_t r;                                                 
  int32_t c;                                                 

  hash_t* hash_function;
  T ***V;
  double_t* p;
  double_t* sigma;
  bool updated;

  uint8_t* current_string;
  int32_t num_of_star;
  std::vector<two_types_of_flow> possible_flows;
  std::vector<ans_t> large_flows;
  std::vector<ans_t> extracted_large_flows;
  std::vector<ans_t> flows_to_remove;

  int32_t get_bit(uint8_t* a, int32_t pos);
  void set_bit(uint8_t* a, int32_t pos, int32_t v);
  double_t normalCFD(double_t value);
  bool my_cmp(uint8_t* s1, uint8_t* s2);

  /**
   * @brief Find the flows which has a very high probability
   *        to be large flows
   *
   */
  void find_possible_flows(int32_t i, int32_t j, 
                           int32_t k, uint8_t* T);
  /**
   * @brief Assume that a large flow is hash into V[k][i][j],
   *        this function calculate the possibility of that, 
   *        the k-th bit of the flow is 1
   *
   */
  double_t cal_hat_p(double_t theta, int32_t i, int32_t j,
                     T*** V, double_t* p, 
                     double_t* sigma, int32_t k);
  /**
   * @brief Filter out fake streams, using the calculated prob_vector.
   *        This function should be implemented by the network 
   *        operators, so it is not implemented here.
   *
   */
  void large_flow_filter();
  /**
   * @brief Calculate p and sigma
   *
   */
  void Sketch2N_p_sigma();
  /**
   * @brief Ectract large flows in the stack (i,j)
   *        with the threshold theta
   *
   */
  void ExtractLargeFlows(double_t theta, int32_t i, int32_t j,
                         T*** V, double_t* p, double_t* sigma);
  /**
   * @brief Remove the extracted large flows from the sketches
   *        The large flows should be provided in flows_to_remove
   *
   */
  void RemoveFlows();
  /**
   * @brief Determine wether or not all the large flows is 
   *        extracted, if so, learning should be terminated
   *
   */
  bool Terminate(double_t theta);
  /**
   * @brief To learn from the sketches and extract all the large
   *        flows.
   *
   */
  void Sketch_Learning();

public:
  /**
   * @brief Construct by specifying depth and width
   *
   */
  SketchLearn(int32_t depth_, int32_t width_);
  /**
   * @brief Release the pointer
   *
   */
  ~SketchLearn();
  /**
   * @brief Update a flowkey with certain value
   *
   */
  void update(const FlowKey<key_len> &flowkey, T val) override;
  /**
   * @brief Get Heavy Hitter
   *
   */
  Data::Estimation<key_len, T> getHeavyHitter(double_t threshold) const override;
  /**
   * @brief Query a flowkey
   *
   */
  T query(const FlowKey<key_len> &flowkey) const override;
  /**
   * @brief Get the size of the sketch
   *
   */
  size_t size();

  void clear();
};

} // namespace OmniSketch::Sketch

//-----------------------------------------------------------------------------
//
///                        Implementation of templated methods
//
//-----------------------------------------------------------------------------

namespace OmniSketch::Sketch {

template <int32_t key_len, typename T, typename hash_t>
int32_t SketchLearn<key_len, T, hash_t>::get_bit(uint8_t* a, int32_t pos){
      int32_t byte = pos / 8;
      int32_t bit = pos % 8;
      if ((a[byte] & (1 << (bit))) == 0)
      {
          return 0;
      }
      else
      {
          return 1;
      }
    }

template <int32_t key_len, typename T, typename hash_t>
void SketchLearn<key_len, T, hash_t>::set_bit(uint8_t* a, int32_t pos, int32_t v){
    int32_t byte = pos / 8;
    int32_t bit = pos % 8;
    if (v == 1)
    {
        a[byte] = a[byte] | (1 << (bit));
    }
    else
    {
        a[byte] = a[byte] & ~(1 << (bit));
    }

}

template <int32_t key_len, typename T, typename hash_t>
void SketchLearn<key_len, T, hash_t>::Sketch2N_p_sigma(){
    double_t sum;
    double_t square_sum;
    double_t tmp_r;
    for (size_t k = 0; k <= ID_length * 8; k++)
    {
        sum = 0;
        square_sum = 0;
        for (size_t i = 0; i < r; i++)
        {
            for (size_t j = 1; j <= c; j++)
            {
                tmp_r = (double_t)(V[k][i][j]) / (double_t)(V[0][i][j]);
                sum += tmp_r;
                square_sum += tmp_r * tmp_r;
            }
        }
        p[k] = (double_t)sum / (double_t)(r * c);
        sigma[k] = sqrt(square_sum / (double_t)(r * c) - p[k] * p[k]);
    }
}

template <int32_t key_len, typename T, typename hash_t>
double_t SketchLearn<key_len, T, hash_t>::normalCFD(double_t value){
    return 0.5 * erfc(-value / sqrt(2));
}

template <int32_t key_len, typename T, typename hash_t>
void SketchLearn<key_len, T, hash_t>::find_possible_flows
  (int32_t i, int32_t j, int32_t k, uint8_t* candidate_string){
    if (k == l + 1)
    {
        uint8_t ans[(l + 7) / 8 + 1];

        for (int32_t kk = 1; kk <= l; kk++)
        {
            set_bit((uint8_t*)ans, kk - 1, current_string[kk] == '1' ? 1 : 0);
        }

        ans[(l + 7) / 8] = '\0';
        if (hash_function[i](ans) == j)
        {
            int32_t flag = 0;
            possible_flows.push_back(two_types_of_flow(current_string, ans));
        }
        return;
    }
    else
    {
        if (candidate_string[k] != '*')
        {
            current_string[k] = candidate_string[k];
            find_possible_flows(i, j, k + 1, candidate_string);
        }
        else
        {
            current_string[k] = '0';
            find_possible_flows(i, j, k + 1, candidate_string);
            current_string[k] = '1';
            find_possible_flows(i, j, k + 1, candidate_string);
        }
    }
    return;
  }

template <int32_t key_len, typename T, typename hash_t>
void SketchLearn<key_len, T, hash_t>::ExtractLargeFlows
  (double_t theta, int32_t i, int32_t j,T*** V, double_t* p, double_t* sigma){
    
    extracted_large_flows.clear();
    
    // 第一步，计算每个bit的概率估值
    double_t hat_p[l + 1];
    for (int32_t k = 1; k <= l; k++)
    {
        hat_p[k] = cal_hat_p(theta, i, j, V, p, sigma, k);
    }
    
    //  第二步，找到所有候选的大流，存在possible_flows里面
    num_of_star = 0;
    uint8_t candidate_string[l + 2];
    for (int32_t k = 1; k <= l; k++)
    {
        if (hat_p[k] > POSSIBLE_THRESHOLD)
        {
            candidate_string[k] = '1';
        }
        else if (1 - hat_p[k] > POSSIBLE_THRESHOLD)
        {
            candidate_string[k] = '0';
        }
        else
        {
            candidate_string[k] = '*';
            num_of_star++;
        }
    }
    candidate_string[l + 1] = '\0';
    candidate_string[0] = '#';
    if (num_of_star > STAR_THRESHOLD)
    {
        return;
    }
    current_string[l + 1] = '\0';
    current_string[0] = '#';
    possible_flows.clear();
    find_possible_flows(i, j, 1, candidate_string);
    
    //  第三步，估计大流的频率和可能性向量
    double_t estimated_frequency[l + 1];
    double_t estimated_p[l + 1];
    for (vector<two_types_of_flow>::iterator item = possible_flows.begin();
        item != possible_flows.end(); item++)
    {
        int32_t min_sketch = 0xfffffff;
        for (int32_t k = 1; k <= l; k++)
        {
            if (item->bit_flow[k] == '1')
            {
                min_sketch = (V[k][i][j] < min_sketch)? V[k][i][j] : min_sketch;
                double_t r = (double_t)V[k][i][j] / V[0][i][j];
                estimated_frequency[k] = ((r - p[k]) / (1 - p[k])) * V[0][i][j];
                estimated_p[k] = hat_p[k];
            }
            else
            {
                min_sketch = (V[0][i][j] - V[k][i][j] < min_sketch)? V[0][i][j] - V[k][i][j] : min_sketch;
                double_t r = (double_t)V[k][i][j] / V[0][i][j];
                estimated_frequency[k] = (1 - r / p[k]) * V[0][i][j];

                estimated_p[k] = 1 - hat_p[k];
            }
        }
        sort(estimated_frequency + 1, estimated_frequency + 1 + l);
        double_t ans_estimated_frequency = estimated_frequency[l / 2];
        if(ans_estimated_frequency > min_sketch)
        {
            if(ans_estimated_frequency > MY_ERROR_THRESHOLD_SKETCH * min_sketch && 
               ans_estimated_frequency > MY_ERROR_THRESHOLD_V0 * V[0][i][j])
            {
                break;
            }
            ans_estimated_frequency = min_sketch;
        }
        extracted_large_flows.push_back(ans_t(item->bit_flow, item->flow, ans_estimated_frequency, estimated_p));
    }
    
    //  第四步，去sketch里查候选流的数据，删掉过小的
    if (r == 1)
    {
        return;
    }
    for (vector<ans_t>::iterator item = extracted_large_flows.begin(); item != extracted_large_flows.end(); )
    {
        for (int32_t ii = 0; ii < r; ii++)
        {
            if (ii == i)
            {
                continue;
            }
            int32_t jj = hash_function[ii](FlowKey<key_len>(item->flow));
            for (int32_t k = 1; k <= l; k++)
            {
                if (item->bit_flow[k] == '0' && V[0][ii][jj] - V[k][ii][jj] < item->size)
                {
                    item->size = V[0][ii][jj] - V[k][ii][jj];
                }
                else if (item->bit_flow[k] == '1' && V[k][ii][jj] < item->size)
                {
                    item->size = V[k][ii][jj];
                }
            }
        }
        if (item->size < theta * V[0][i][j])
        {
            item = extracted_large_flows.erase(item);
            if (item == extracted_large_flows.end())break;
        }
        else 
        {
            item++;
        }
    }
    return;
  }

template <int32_t key_len, typename T, typename hash_t>
double_t SketchLearn<key_len, T, hash_t>::cal_hat_p
  (double_t theta, int32_t i, int32_t j, T*** V, 
  double_t* p, double_t* sigma, int32_t k){
    double_t r = (double_t)V[k][i][j] / V[0][i][j];
    if (r < theta)
    {
        return 0;
    }
    if (1 - r < theta)
    {
        return 1;
    }
    double_t ans = 0;
    double_t prob_1 = (V[k][i][j] - theta * V[0][i][j]) /
        (V[0][i][j] - theta * V[0][i][j]);
    double_t prob_0 = (V[k][i][j]) /
        (V[0][i][j] - theta * V[0][i][j]);
    double_t normal_val1 = normalCFD((prob_1 - p[k]) / sigma[k]);
    double_t normal_val0 = normalCFD((prob_0 - p[k]) / sigma[k]);
    return normal_val1 * p[k] + (1 - normal_val0) * (1 - p[k]);
  }

template <int32_t key_len, typename T, typename hash_t>
void SketchLearn<key_len, T, hash_t>::RemoveFlows(){
    vector<ans_t> FF = flows_to_remove;
    uint32_t tmp_hash[r + 1];
    for (int32_t it = 0; it < FF.size(); it++)
    {
        uint8_t* ans = FF[it].flow;

        for (size_t i = 0; i < r; i++)
        {
            tmp_hash[i] = hash_function[i](ans);
            V[0][i][tmp_hash[i]] -= FF[it].size;
        }
        for (size_t k = 1; k <= ID_length * 8; k++)
        {
            if (0 != get_bit((uint8_t*)ans, k - 1))
            {
                for (size_t i = 0; i < r; i++)
                {
                    V[k][i][tmp_hash[i]] -= FF[it].size;
                }
            }
        }
    }
}

template <int32_t key_len, typename T, typename hash_t>
bool SketchLearn<key_len, T, hash_t>::Terminate(double_t theta){
  
    double_t RATE1 = 0.6826 + STEP * log2(theta);
    double_t RATE2 = 0.9544 + STEP * log2(theta);
    double_t RATE3 = 0.9973 + STEP * log2(theta);

    for (size_t k = 1; k <= ID_length * 8; k++)
    {
        size_t sigma_num1 = 0, sigma_num2 = 0, sigma_num3 = 0;
        for (int32_t i = 0; i < r; i++)
        {
            for (int32_t j = 1; j <= c; j++)
            {
                double_t r = (double_t)V[k][i][j] / V[0][i][j];
                if (r <= p[k] + 3.0 * sigma[k] && r >= p[k] - 3.0 * sigma[k])
                    sigma_num3++;
                if (r <= p[k] + 2.0 * sigma[k] && r >= p[k] - 2.0 * sigma[k])
                    sigma_num2++;
                if (r <= p[k] + 1.0 * sigma[k] && r >= p[k] - 1.0 * sigma[k])
                    sigma_num1++;
            }
        }
        if(sigma_num1 == 0 && sigma_num2 == 0 && sigma_num3 == 0)
        {
            printf("I WONDER WHY PROGRAME RUNNING REACH HERE\n");
        }
        double_t rate1 = (double_t)sigma_num1 / (double_t)(r * c);
        double_t rate2 = (double_t)sigma_num2 / (double_t)(r * c);
        double_t rate3 = (double_t)sigma_num3 / (double_t)(r * c);
        
        if (rate1 < RATE1)
            return false;
        if (rate2 < RATE2)
            return false;
        if (rate3 < RATE3)
            return false;
    }
    return true;
}

template <int32_t key_len, typename T, typename hash_t>
bool SketchLearn<key_len, T, hash_t>::my_cmp(uint8_t* s1, uint8_t* s2){
    for (size_t i = 0; i < ID_length; i++)
    {
        if (s1[i] != s2[i])
        {
            return false;
        }
    }
    return true;
}

template <int32_t key_len, typename T, typename hash_t>
SketchLearn<key_len, T, hash_t>::SketchLearn(int32_t depth_, int32_t width_)
    : r(depth_), c(Util::NextPrime(width_)){
    hash_function = new hash_t[r];
    V = new T **[l + 1];
    for(int32_t i = 0; i < l + 1; i++)
    {
      V[i] = new T *[r];
      V[i][0] = new T[r * (c + 1)];
      for(int32_t j = 1; j < r; j++)
      {
        V[i][j] = V[i][j-1] + (c + 1);
      }
    }
    p = new double_t[l + 1];
    sigma = new double_t[l + 1];
    current_string = new uint8_t[l + 2];
    num_of_star = 0;
    updated = false;
}

template <int32_t key_len, typename T, typename hash_t>
SketchLearn<key_len, T, hash_t>::~SketchLearn(){
   delete[] hash_function;
   for(int32_t i = 0; i < l + 1; i++)
   {
     delete[] V[i][0];
     delete[] V[i];
   }
   delete[] V;
   delete[] p;
   delete[] sigma;
   delete[] current_string;

   possible_flows.clear();
   large_flows.clear();
   extracted_large_flows.clear();
   flows_to_remove.clear();
}

template <int32_t key_len, typename T, typename hash_t>
size_t SketchLearn<key_len, T, hash_t>::size(){
   return sizeof(*this)
          + r * sizeof(hash_t)
          + r * (c + 1) * sizeof(T)
          + 2 * (l + 1) * sizeof(double_t)
          + (l + 2) * sizeof(uint_8);
}

template <int32_t key_len, typename T, typename hash_t>
void SketchLearn<key_len, T, hash_t>::update(const FlowKey<key_len> &flowkey, T val){
    if(!updated)
    {
      updated = true;
    }
    uint32_t tmp_hash[r + 1];
    for (size_t i = 0; i < r; i++)
    {
        tmp_hash[i] = hash_function[i](flowkey);
        V[0][i][tmp_hash[i]] += val;
    }
    for (size_t k = 1; k <= key_len * 8; k++)
    {
        // 0 则对 V[k] 无影响
        if (0 != flowkey.getBit(k - 1))
        {
            for (size_t i = 0; i < r; i++)
            {
                V[k][i][tmp_hash[i]] += val;
            }
        }
    }
}

template <int32_t key_len, typename T, typename hash_t>
void SketchLearn<key_len, T, hash_t>::Sketch_Learning(){
    double_t theta = START_THETA;
    int32_t nnnn = 0;
    large_flows.clear();
    while (1)
    {
        int32_t my_flow_num = 0;
        std::vector<ans_t> FF;
        for (int32_t i = 0; i < r; i++)
        {
            for (int32_t j = 1; j <= c; j++)
            {
                if (0 == V[0][i][j])
                {
                    continue;
                }
                ExtractLargeFlows(theta, i, j, V, p, sigma);
                std::vector<ans_t> temp_F = extracted_large_flows;
                if (!temp_F.empty())
                {
                    my_flow_num++;
                    for (std::vector<ans_t>::iterator it = temp_F.begin(); it < temp_F.end(); it++)
                    {
                        bool temp_Fin = false;
                        if (!FF.empty())
                        {
                            for (std::vector<ans_t>::iterator iter = FF.begin(); iter < FF.end(); iter++)
                            {
                                if (strcmp(iter->bit_flow, it->bit_flow) == 0)
                                {
                                    temp_Fin = true;
                                    break;
                                }
                            }
                        }
                        if (!temp_Fin)
                            FF.push_back(*it);
                    }
                }
            }
        }

        //本次循环找出大流时，剔除大流，重新计算期望、方差
        if (!FF.empty())
        {
            for (std::vector<ans_t>::iterator it = FF.begin(); it < FF.end(); it++)
            {
                bool FF_in = false;
                std::vector<ans_t>::iterator temp_pos = FF.begin();
                if (!large_flows.empty())
                {
                    for (std::vector<ans_t>::iterator iter = large_flows.begin(); iter < large_flows.end(); iter++)
                    {
                        if (strcmp(iter->bit_flow, it->bit_flow) == 0)
                        {
                            FF_in = true;
                            temp_pos = iter;
                            break;
                        }
                    }
                }
                if (!FF_in)
                    large_flows.push_back(*it);
                else if (FF_in)
                {
                    temp_pos->size += it->size;
                }
            }
            flows_to_remove.clear();
            flows_to_remove = FF;
            RemoveFlows();
            Sketch2N_p_sigma();
        }
        printf("%d loop is completed______________, theta = %lf\n\n", nnnn, theta);
        nnnn++;

        if (Terminate(theta))
            break;
        //没有找出大流，theta减半
        if (FF.empty())
            theta /= 2;
    }
    large_flow_filter();
}

template <int32_t key_len, typename T, typename hash_t>
void SketchLearn<key_len, T, hash_t>::large_flow_filter(){
  return;
}

template <int32_t key_len, typename T, typename hash_t>
Data::Estimation<key_len, T> SketchLearn<key_len, T, hash_t>::getHeavyHitter(double_t threshold) const {
    if(updated || large_flows.size() == 0)
    {
      Sketch_Learning();
      updated = false;
    }
    Data::Estimation<key_len, T> heavy_hitters;
    for(auto it : large_flows)
    {
      if(it.size >= threshold)
      {
        heavy_hitters[FlowKey<key_len>(it.flow)] = it.size;
      }
    }
    return heavy_hitters;
}

template <int32_t key_len, typename T, typename hash_t>
T SketchLearn<key_len, T, hash_t>::query(const FlowKey<key_len> &flowkey) const{
    if(updated || large_flows.size() == 0)
    {
      Sketch_Learning();
      updated = false;
    }
    for(auto it : large_flows)
    {
      if(FlowKey<key_len>(it.flow) == flowkey)
      {
        return it.size;
      }
    }
    int32_t result = 0xfffffff;
    for (int32_t ii = 0; ii < r; ii++)
    {
        int32_t jj = hash_function[ii](FlowKey<key_len>(item->flow));
        for (int32_t k = 1; k <= l; k++)
        {
            if ( flowkey.getBit(k - 1) == 0 && V[0][ii][jj] - V[k][ii][jj] < result)
            {
                result = V[0][ii][jj] - V[k][ii][jj];
            }
            else if (flowkey.getBit(k - 1) == 1 && V[k][ii][jj] < result)
            {
                result = V[k][ii][jj];
            }
        }
    }
    return result;
}

template <int32_t key_len, typename T, typename hash_t>
void SketchLearn<key_len, T, hash_t>::clear(){
  for(int32_t i = 0; i < l + 1; i++)
  {
    for(int32_t j = 0; j < r; j++)
    {
      for(int32_t k = 0; k < c + 1; k++)
      {
        V[i][j][k] = 0;
      }
    }
  }
  for(int32_t i = 0; i < l + 1; i++)
  {
    p[i] = sigma[i] = 0;
  }
  updated = false;
  possible_flows.clear();
  large_flows.clear();
  extracted_large_flows.clear();
  flows_to_remove.clear();
  num_of_star = 0;
}

}
