#ifndef CONDITIONING_TESTS_H_
#define CONDITIONING_TESTS_H_

#include <gtest/gtest.h>

#include "conditioning.h"

#include "context.h"
#include "facility_tests.h"
#include "agent_tests.h"

namespace conditioning {
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class ConditioningTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc_;
  Conditioning* src_facility_;

  virtual void SetUp();
  virtual void TearDown();
  void InitParameters();
  void SetUpConditioning();
  void TestInitState(conditioning::Conditioning* fac);
  //void TestAddMat(conditioning::Conditioning* fac, 
  //    cyclus::Material::Ptr mat);
  void TestBuffers(conditioning::Conditioning* fac, double inv, double 
      proc, double ready, double stocks);
  void TestStocks(conditioning::Conditioning* fac, std::map<std::string, std::map<std::string, double>> package_properties);
  void TestReadyTime(conditioning::Conditioning* fac, int t);
  void TestCurrentCap(conditioning::Conditioning* fac, double inv);

  std::vector<std::string> in_c1, out_c1;
  std::string in_r1;

  int residence_time, package_size;
  double throughput, max_inv_size;
  std::map<std::string, std::map<std::string, double>> package_properties; 
  //bool discrete_handling;
};
} // namespace conditioning
#endif // CONDITIONING_TESTS_H_