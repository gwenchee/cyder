#ifndef CYDER_PMSINKS_PMSINK_H_
#define CYDER_PMSINKS_PMSINK_H_

#include <string>
#include <list>
#include <vector>

#include "cyclus.h"
#include "cyder_version.h"

// forward declaration
namespace pmsink {
class Pmsink;
} // namespace pmsink


namespace pmsink {

class PmSink 
  : public cyclus::Facility,
    public cyclus::toolkit::CommodityProducer,
    public cyclus::toolkit::Position {
 public:  
  /// @param ctx the cyclus context for access to simulation-wide parameters
  PmSink(cyclus::Context* ctx);
  
  #pragma cyclus decl

  #pragma cyclus note {"doc": "PmSink is a simple facility which accepts any number of commodities " \
                              "and holds them for a user specified amount of time. The commodities accepted "\
                              "are chosen based on the specified preferences list. Once the desired amount of material "\
                              "has entered the facility it is passed into a 'processing' buffer where it is held until "\
                              "the residence time has passed. The material is then passed into a 'ready' buffer where it is "\
                              "queued for removal. Currently, all input commodities are lumped into a single output commodity. "\
                              "PmSink also has the functionality to handle materials in discrete or continuous batches. Discrete "\
                              "mode, which is the default, does not split or combine material batches. Continuous mode, however, "\
                              "divides material batches if necessary in order to push materials through the facility as quickly "\
                              "as possible."}

  /// A verbose printer for the PmSink Facility
  virtual std::string str();

  // --- Facility Members ---
  
  // --- Agent Members ---
  /// Sets up the PmSink Facility's trade requests
  virtual void EnterNotify();

  /// The handleTick function specific to the PmSink.
  virtual void Tick();

  /// The handleTick function specific to the PmSink.
  virtual void Tock();

 protected:
  ///   @brief adds a packagedmaterial into the incoming commodity inventory
  ///   @param mat the material to add to the incoming inventory.
  ///   @throws if there is trouble with pushing to the inventory buffer.
  void Store_(double cap);


  ///   @brief adds a material into the incoming commodity inventory
  ///   @param mat the material to add to the incoming inventory.
  ///   @throws if there is trouble with pushing to the inventory buffer.
  // void AddMat_(cyclus::Material::Ptr mat);

  /// @brief Move all unprocessed inventory to processing
  //void BeginProcessing_();

  /// @brief Move as many ready resources as allowable into stocks
  /// @param cap current throughput capacity 
  //void ProcessMat_(double cap);

  /// @brief move ready resources from processing to ready at a certain time
  /// @param time the time of interest
  //void ReadyMatl_(int time);

    /* --- PmSink Members --- */

  /// @brief current maximum amount that can be added to processing
  inline double current_capacity() const { 
    return (max_inv_size - store.quantity()); }

  /// @brief returns the time key for ready materials
  //int ready_time(){ return context()->time() - residence_time; }

  /* --- Module Members --- */

  #pragma cyclus var {"tooltip":"input commodity",\
                      "doc":"commodities accepted by this facility",\
                      "uilabel":"Input Commodities",\
                      "uitype":["oneormore","incommodity"]}
  std::vector<std::string> in_commods;

  #pragma cyclus var {"default": [],\
                      "doc":"preferences for each of the given commodities, in the same order."\
                      "Defauts to 1 if unspecified",\
                      "uilabel":"In Commody Preferences", \
                      "range": [None, [1e-299, 1e299]], \
                      "uitype":["oneormore", "range"]}
  std::vector<double> in_commod_prefs;
/*
  #pragma cyclus var {"tooltip":"output commodity",\
                      "doc":"commodity produced by this facility. Multiple commodity tracking is"\
                      " currently not supported, one output commodity catches all input commodities.",\
                      "uilabel":"Output Commodities",\
                      "uitype":["oneormore","outcommodity"]}
  std::vector<std::string> out_commods;

  #pragma cyclus var {"default":"",\
                      "tooltip":"input recipe",\
                      "doc":"recipe accepted by this facility, if unspecified a dummy recipe is used",\
                      "uilabel":"Input Recipe",\
                      "uitype":"inrecipe"}
  std::string in_recipe;

  #pragma cyclus var {"default": 0,\
                      "tooltip":"residence time (timesteps)",\
                      "doc":"the minimum holding time for a received commodity (timesteps).",\
                      "units":"time steps",\
                      "uilabel":"Residence Time", \
                      "uitype": "range", \
                      "range": [0, 12000]}
  int residence_time;
*/
  #pragma cyclus var {"default": 1e299,\
                     "tooltip":"throughput per timestep (kg)",\
                     "doc":"the max amount that can be moved through the facility per timestep (kg)",\
                     "uilabel":"Throughput",\
                     "uitype": "range", \
                     "range": [0.0, 1e299], \
                     "units":"kg"}
  double throughput;

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum inventory size (kg)",\
                      "doc":"the maximum amount of material that can be in all pmsink buffer stages",\
                      "uilabel":"Maximum Inventory Size",\
                      "uitype": "range", \
                      "range": [0.0, 1e299], \
                      "units":"kg"}
  double max_inv_size; 
/*
  #pragma cyclus var {"default": False,\
                      "tooltip":"Bool to determine how PmSink handles batches",\
                      "doc":"Determines if PmSink will divide resource objects. Only controls material "\
                            "handling within this facility, has no effect on DRE material handling. "\
                            "If true, batches are handled as discrete quanta, neither split nor combined. "\
                            "Otherwise, batches may be divided during processing. Default to false (continuous))",\
                      "uilabel":"Batch Handling"}
  bool discrete_handling;                    
*/
  #pragma cyclus var {"tooltip":"Incoming packagedmaterial buffer"}
  cyclus::toolkit::ResBuf<cyclus::PackagedMaterial> inventory;

  #pragma cyclus var {"tooltip":"packaged material sink forever"}
  cyclus::toolkit::ResBuf<cyclus::PackagedMaterial> store;
/*
  #pragma cyclus var {"tooltip":"Output material buffer"}
  cyclus::toolkit::ResBuf<cyclus::Material> stocks;

  #pragma cyclus var {"tooltip":"Buffer for material held for required residence_time"}
  cyclus::toolkit::ResBuf<cyclus::Material> ready;

  //// list of input times for materials entering the processing buffer
  #pragma cyclus var{"default": [],\
                      "internal": True}
  std::list<int> entry_times;

  #pragma cyclus var {"tooltip":"Buffer for material still waiting for required residence_time"}
  cyclus::toolkit::ResBuf<cyclus::Material> processing;
*/
  //// A policy for requesting material
  cyclus::toolkit::PackagedMatlBuyPolicy buy_policy;

  //// A policy for sending material
  //cyclus::toolkit::MatlSellPolicy sell_policy;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical latitude in degrees as a double", \
    "doc": "Latitude of the agent's geographical position. The value should " \
           "be expressed in degrees as a double." \
  }
  double latitude;

  #pragma cyclus var { \
    "default": 0.0, \
    "uilabel": "Geographical longitude in degrees as a double", \
    "doc": "Longitude of the agent's geographical position. The value should " \
           "be expressed in degrees as a double." \
  }
  double longitude;

  cyclus::toolkit::Position coordinates;

  void RecordPosition();

  friend class PmSinkTest;
};

}  // namespace pmsink

#endif // CYCLUS_PMSINKS_PMSINK_H_
