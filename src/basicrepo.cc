// basicrepo.cc
// Implements the BasicRepo class
#include "basicrepo.h"

namespace basicrepo {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BasicRepo::BasicRepo(cyclus::Context* ctx) 
    : cyclus::Facility(ctx),
      latitude(0.0),
      longitude(0.0),
      coordinates(latitude, longitude) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>(
      "The BasicRepo Facility is experimental.");};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// pragmas

#pragma cyclus def schema basicrepo::BasicRepo

#pragma cyclus def annotations basicrepo::BasicRepo

#pragma cyclus def initinv basicrepo::BasicRepo

#pragma cyclus def snapshotinv basicrepo::BasicRepo

#pragma cyclus def infiletodb basicrepo::BasicRepo

#pragma cyclus def snapshot basicrepo::BasicRepo

#pragma cyclus def clone basicrepo::BasicRepo

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BasicRepo::InitFrom(BasicRepo* m) {
#pragma cyclus impl initfromcopy basicrepo::BasicRepo
  cyclus::toolkit::CommodityProducer::Copy(m);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BasicRepo::InitFrom(cyclus::QueryableBackend* b) {
#pragma cyclus impl initfromdb basicrepo::BasicRepo

  using cyclus::toolkit::Commodity;
  Commodity commod = Commodity(out_commods.front());
  cyclus::toolkit::CommodityProducer::Add(commod);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod, throughput);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BasicRepo::EnterNotify() {
  cyclus::Facility::EnterNotify();
  buy_policy.Init(this, &inventory, std::string("inventory"));

  // dummy comp, use in_recipe if provided
  cyclus::CompMap v;
  cyclus::Composition::Ptr comp = cyclus::Composition::CreateFromAtom(v);
  if (in_recipe != "") {
    comp = context()->GetRecipe(in_recipe);
  }

  if (in_commod_prefs.size() == 0) {
    for (int i = 0; i < in_commods.size(); ++i) {
      in_commod_prefs.push_back(cyclus::kDefaultPref);
    }
  } else if (in_commod_prefs.size() != in_commods.size()) {
    std::stringstream ss;
    ss << "in_commod_prefs has " << in_commod_prefs.size()
       << " values, expected " << in_commods.size();
    throw cyclus::ValueError(ss.str());
  }

  for (int i = 0; i != in_commods.size(); ++i) {
    buy_policy.Set(in_commods[i],in_commod_prefs[i]);
  }
  buy_policy.Start();

  if (out_commods.size() == 1) {
    sell_policy.Init(this, &stocks, std::string("stocks"))
        .Set(out_commods.front())
        .Start();
  } else {
    std::stringstream ss;
    ss << "out_commods has " << out_commods.size() << " values, expected 1.";
    throw cyclus::ValueError(ss.str());
  }
  RecordPosition();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string BasicRepo::str() {
  std::stringstream ss;
  std::string ans, out_str;
  if (out_commods.size() == 1) {
    out_str = out_commods.front();
  } else {
    out_str = "";
  }
  if (cyclus::toolkit::CommodityProducer::Produces(
          cyclus::toolkit::Commodity(out_str))) {
    ans = "yes";
  } else {
    ans = "no";
  }
  ss << cyclus::Facility::str();
  ss << " has facility parameters {"
     << "\n"
     << "     Output Commodity = " << out_str << ",\n"
     << "     Residence Time = " << residence_time << ",\n"
     << "     Throughput = " << throughput << ",\n"
     << " commod producer members: "
     << " produces " << out_str << "?:" << ans << "'}";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BasicRepo::Tick() {
  // Set available capacity for Buy Policy
  inventory.capacity(current_capacity());

  LOG(cyclus::LEV_INFO3, "ComCnv") << prototype() << " is ticking {";

  if (current_capacity() > cyclus::eps_rsrc()) {
    LOG(cyclus::LEV_INFO4, "ComCnv")
        << " has capacity for " << current_capacity() << " kg of packagedmatl.";
  }
  LOG(cyclus::LEV_INFO3, "ComCnv") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BasicRepo::Tock() {
  LOG(cyclus::LEV_INFO3, "ComCnv") << prototype() << " is tocking {";

  BeginProcessing_();  // place unprocessed inventory into processing

  if (ready_time() >= 0 || residence_time == 0 && !inventory.empty()) {
    ReadyMatl_(ready_time());  // place processing into ready
  }

  ProcessMat_(throughput);  // place ready into stocks

  LOG(cyclus::LEV_INFO3, "ComCnv") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BasicRepo::AddMat_(cyclus::PackagedMaterial::Ptr mat) {
  LOG(cyclus::LEV_INFO5, "ComCnv") << prototype() << " is initially holding "
                                   << inventory.quantity() << " total.";

  try {
    inventory.Push(mat);
  } catch (cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }


  LOG(cyclus::LEV_INFO5, "ComCnv")
      << prototype() << " added " << mat->quantity()
      << " of packagedmatl to its inventory, which is holding "
      << inventory.quantity() << " total.";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BasicRepo::BeginProcessing_() {

    std::cout << "basicrepo inventory" << std::endl;
    std::cout << inventory.quantity() << std::endl;
  while (inventory.count() > 0) {
    try {
      processing.Push(inventory.Pop());
      entry_times.push_back(context()->time());

      LOG(cyclus::LEV_DEBUG2, "ComCnv")
          << "BasicRepo " << prototype()
          << " added resources to processing at t= " << context()->time();
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }

    std::cout << "basicrepo processing" << std::endl;
    std::cout << processing.quantity() << std::endl;

  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BasicRepo::ProcessMat_(double cap) {
  using cyclus::PackagedMaterial;
  using cyclus::ResCast;
  using cyclus::toolkit::ResBuf;
  using cyclus::toolkit::Manifest;

  if (!ready.empty()) {
    try {
      double max_pop = std::min(cap, ready.quantity());

      if (discrete_handling) {
        if (max_pop == ready.quantity()) {
          stocks.Push(ready.PopN(ready.count()));
        } else {
          double cap_pop = ready.Peek()->quantity();
          while (cap_pop <= max_pop && !ready.empty()) {
            stocks.Push(ready.Pop());
            cap_pop += ready.empty() ? 0 : ready.Peek()->quantity();
          }
        }
      } else {
        stocks.Push(ready.Pop(max_pop, cyclus::eps_rsrc()));
      }
      
        std::cout << "basicrepo stocks" << std::endl;
        std::cout << stocks.quantity() << std::endl;

      LOG(cyclus::LEV_INFO1, "ComCnv") << "BasicRepo " << prototype()
                                       << " moved resources"
                                       << " from ready to stocks"
                                       << " at t= " << context()->time();
    } catch (cyclus::Error& e) {
      e.msg(Agent::InformErrorMsg(e.msg()));
      throw e;
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BasicRepo::ReadyMatl_(int time) {
  using cyclus::toolkit::ResBuf;

  int to_ready = 0;

  while (!entry_times.empty() && entry_times.front() <= time) {
    entry_times.pop_front();
    ++to_ready;
  }

  ready.Push(processing.PopN(to_ready));

    std::cout << "basicrepo ready" << std::endl;
    std::cout << ready.quantity() << std::endl;
}

void BasicRepo::RecordPosition() {
  std::string specification = this->spec();
  context()
      ->NewDatum("AgentPosition")
      ->AddVal("Spec", specification)
      ->AddVal("Prototype", this->prototype())
      ->AddVal("AgentId", id())
      ->AddVal("Latitude", latitude)
      ->AddVal("Longitude", longitude)
      ->Record();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructBasicRepo(cyclus::Context* ctx) {
  return new BasicRepo(ctx);
}

}  // namespace basicrepo
