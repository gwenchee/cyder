/** \file Component.cpp
 * \brief Implements the Component class used by the Generic Repository 
 * \author Kathryn D. Huff
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <typeinfo>
#include <boost/lexical_cast.hpp>

#include "CycException.h"
#include "Component.h"
#include "LumpedThermal.h"
#include "StubThermal.h"
#include "DegRateNuclide.h"
#include "LumpedNuclide.h"
#include "MixedCellNuclide.h"
#include "OneDimPPMNuclide.h"
#include "StubNuclide.h"
#include "Logger.h"
#include "EventManager.h"

using namespace std;
using boost::lexical_cast;

// Static variables to be initialized.
int Component::nextID_ = 0;

string Component::thermal_type_names_[] = {
  "LumpedThermal",
  "StubThermal"
};
string Component::nuclide_type_names_[] = {
  "DegRateNuclide",
  "LumpedNuclide",
  "MixedCellNuclide",
  "OneDimPPMNuclide",
  "StubNuclide", 
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Component::Component(Model* creator) :
  name_(""),
  type_(LAST_EBS),
  thermal_model_(StubThermal::create()),
  nuclide_model_(StubNuclide::create()),
  mat_table_(),
  parent_(),
  temp_(0),
  peak_inner_temp_(0),
  peak_outer_temp_(0),
  temp_lim_(373){

  creator_ = creator;
  set_geom(GeometryPtr(new Geometry()));
  comp_hist_ = CompHistory();
  mass_hist_ = MassHistory();

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Component::~Component(){ // @TODO is there anything to delete? Make This virtual? 
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Component::initModuleMembers(QueryEngine* qe){

  string name = qe->getElementContent("name");
  ComponentType type = componentEnum(qe->getElementContent("componenttype"));
  string mat = qe->queryElement("material_data")->getElementName();
  Radius inner_radius = lexical_cast<double>(qe->getElementContent("innerradius"));
  Radius outer_radius = lexical_cast<double>(qe->getElementContent("outerradius"));

  LOG(LEV_DEBUG2,"GRComp") << "The Component Class init(qe) function has been called.";;

  shared_from_this()->init(name, type, mat, inner_radius, outer_radius, thermal_model(qe->queryElement("thermalmodel")), nuclide_model(qe->queryElement("nuclidemodel")));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Component::init(string name, ComponentType type, string mat,
    Radius inner_radius, Radius outer_radius, ThermalModelPtr thermal_model, 
    NuclideModelPtr nuclide_model){

  ID_=nextID_++;
  
  name_ = name;
  type_ = type;
  set_mat_table(mat);
  geom()->set_radius(INNER, inner_radius);
  geom()->set_radius(OUTER, outer_radius);

  if ( !(thermal_model) || !(nuclide_model) ) {
    string err = "The thermal or nuclide model provided is null " ;
    throw CycException(err);
  } else { 
    thermal_model->set_geom(geom_);
    nuclide_model->set_geom(geom_);

    set_thermal_model(thermal_model);
    set_nuclide_model(nuclide_model);
  }

  comp_hist_ = CompHistory();
  mass_hist_ = MassHistory();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Component::copy(const ComponentPtr& src){
  ID_=nextID_++;

  set_name(src->name());
  set_type(src->type());
  set_mat_table(src->mat_table());

  // warning, you are currently copying the centroid as well. 
  // does this object lay on top of the one being copied?
  set_geom(src->geom()->copy(src->geom(),src->centroid()));

  if ( !(src->thermal_model()) ){
    string err = "The " ;
    err += name_;
    err += " model with ID: ";
    err += src->ID_;
    err += " does not have a thermal model";
    throw CycException(err);
  } else { 
    set_thermal_model(copyThermalModel(src->thermal_model_));
  }
  if ( !(src->nuclide_model())) {
    string err = "The " ;
    err += name_;
    err += " model with ID: ";
    err += src->ID_;
    err += " does not have a nuclide model";
    throw CycException(err);
  }else { 
    set_nuclide_model(copyNuclideModel(src->nuclide_model()));
  }

  temp_ = src->temp_;
  temp_lim_ = src->temp_lim_ ;

  comp_hist_ = CompHistory();
  mass_hist_ = MassHistory();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void Component::print(){
  std::deque<mat_rsrc_ptr> waste_list=wastes();
  LOG(LEV_DEBUG2,"GRComp") << "Component: " << shared_from_this()->name();
  LOG(LEV_DEBUG2,"GRComp") << "Contains Materials:";
  for(int i=0; i< waste_list.size() ; i++){
    LOG(LEV_DEBUG2,"GRComp") << waste_list[i];
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Component::updateContaminantTable(int the_time){
  // get the vec_hist
  std::pair<IsoVector, double> vec_pair = nuclide_model()->vec_hist(the_time);
  CompMapPtr comp = vec_pair.first.comp();
  double mass = vec_pair.second;
  // iterate over the vec_hist IsoVector
  std::map<int, double>::iterator entry;

  for( entry=comp->begin(); entry!=comp->end(); ++entry ){
    EM->newEvent("contaminants")
      ->addVal( "CompID", ID())
      ->addVal( "Time", the_time)
      ->addVal( "IsoID", (*entry).first)
      ->addVal( "MassKG", (*entry).second*mass)
      ->addVal( "AvailConc", nuclide_model()->conc_hist(the_time, (*entry).first))
      ->record();
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Component::absorb(mat_rsrc_ptr mat_to_add){
  try{
    nuclide_model()->absorb(mat_to_add);
  } catch ( exception& e ) {
    LOG(LEV_ERROR, "GRComp") << "Error occured in component absorb function." << e.what();
  }
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Component::extract(CompMapPtr comp_to_rem, double kg_to_rem){
  try{
    nuclide_model()->extract(comp_to_rem, kg_to_rem);
  } catch ( exception& e ) {
    LOG(LEV_ERROR, "GRComp") << "Error occured in component extract function." << e.what();
  }
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Component::transportHeat(int time){
  if ( !thermal_model_ ) {
    LOG(LEV_ERROR, "GRComp") << "Error, no thermal_model_ loaded before Component::transportHeat." ;
  } else {
    thermal_model_->transportHeat(time);
  }
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Component::transportNuclides(int the_time){
  if ( !nuclide_model() ) {
    LOG(LEV_ERROR, "GRComp") << "Error, no nuclide_model_ loaded before Component::transportNuclides." ;
  } else { 
    nuclide_model()->update_inner_bc(the_time, nuclide_daughters());
    nuclide_model()->transportNuclides(the_time);
  }
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ComponentPtr Component::load(ComponentType type, ComponentPtr to_load) {
  to_load->set_parent(ComponentPtr(shared_from_this()));
  daughters_.push_back(to_load);
  return shared_from_this();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Component::isFull() {
  // @TODO imperative, add better logic here 
  bool to_ret;
  double wp_len(0);
  std::vector<ComponentPtr>::iterator it;
  switch(type()) {
    case BUFFER : 
      for(it=daughters_.begin(); it!=daughters_.end(); ++it){
        wp_len += (*it)->geom()->length();
      }
      to_ret = (wp_len >= geom()->length());
      break;
    default : 
      to_ret=true;
      break;
  }
  
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ComponentType Component::type(){return type_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ComponentType Component::componentEnum(std::string type_name) {
  ComponentType toRet = LAST_EBS;
  string component_type_names[] = {"BUFFER", "FF", "WF", "WP"};
  for(int type = 0; type < LAST_EBS; type++){
    if(component_type_names[type] == type_name){
      toRet = (ComponentType)type;
    } 
  }
  if (toRet == LAST_EBS){
    string err_msg ="'";
    err_msg += type_name;
    err_msg += "' does not name a valid ComponentType.\n";
    err_msg += "Options are:\n";
    for(int name=0; name < LAST_EBS; name++){
      err_msg += component_type_names[name];
      err_msg += "\n";
    }
    throw CycException(err_msg);
  }
  return toRet;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ThermalModelType Component::thermalEnum(std::string type_name) {
  ThermalModelType toRet = LAST_THERMAL;
  for(int type = 0; type < LAST_THERMAL; type++){
    if(thermal_type_names_[type] == type_name){
      toRet = (ThermalModelType)type;
    } 
  }
  if (toRet == LAST_THERMAL){
    string err_msg ="'";
    err_msg += type_name;
    err_msg += "' does not name a valid ThermalModelType.\n";
    err_msg += "Options are:\n";
    for(int name=0; name < LAST_THERMAL; name++){
      err_msg += thermal_type_names_[name];
      err_msg += "\n";
    }     
    throw CycException(err_msg);
  }
  return toRet;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
NuclideModelType Component::nuclideEnum(std::string type_name) {
  NuclideModelType toRet = LAST_NUCLIDE;
  for(int type = 0; type < LAST_NUCLIDE; type++){
    if(nuclide_type_names_[type] == type_name){
      toRet = (NuclideModelType)type;
    }
  }
  if (toRet == LAST_NUCLIDE){
    string err_msg ="'";
    err_msg += type_name;
    err_msg += "' does not name a valid NuclideModelType.\n";
    err_msg += "Options are:\n";
    for(int name=0; name < LAST_NUCLIDE; name++){
      err_msg += nuclide_type_names_[name];
      err_msg += "\n";
    }
    throw CycException(err_msg);
  }
  return toRet;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
ThermalModelPtr Component::thermal_model(QueryEngine* qe){
  ThermalModelPtr toRet;

  string model_name = qe->getElementName();;
  
  switch(thermalEnum(model_name))
  {
    case LUMPED_THERMAL:
      toRet = ThermalModelPtr(LumpedThermal::create(qe));
      break;
    case STUB_THERMAL:
      toRet = ThermalModelPtr(StubThermal::create(qe));
      break;
    default:
      throw CycException("Unknown thermal model enum value encountered."); 
  }
  toRet->set_mat_table(mat_table());
  return toRet;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
NuclideModelPtr Component::nuclide_model(QueryEngine* qe){
  NuclideModelPtr toRet;

  string model_name = qe->getElementName();;
  QueryEngine* input = qe->queryElement(model_name);

  switch(nuclideEnum(model_name))
  {
    case DEGRATE_NUCLIDE:
      toRet = NuclideModelPtr(DegRateNuclide::create(input));
      break;
    case LUMPED_NUCLIDE:
      toRet = NuclideModelPtr(LumpedNuclide::create(input));
      break;
    case MIXEDCELL_NUCLIDE:
      toRet = NuclideModelPtr(MixedCellNuclide::create(input));
      break;
    case ONEDIMPPM_NUCLIDE:
      toRet = NuclideModelPtr(OneDimPPMNuclide::create(input));
      break;
    case STUB_NUCLIDE:
      toRet = NuclideModelPtr(StubNuclide::create(input));
      break;
    default:
      throw CycException("Unknown nuclide model enum value encountered."); 
  }
  toRet->set_mat_table(mat_table());
  return toRet;
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
ThermalModelPtr Component::copyThermalModel(ThermalModelPtr src){
  ThermalModelPtr toRet;
  switch( src->type() )
  {
    case LUMPED_THERMAL:
      toRet = ThermalModelPtr(LumpedThermal::create());
      break;
    case STUB_THERMAL:
      toRet = ThermalModelPtr(StubThermal::create());
      break;
    default:
      throw CycException("Unknown thermal model enum value encountered when copying."); 
  }      
  toRet->copy(src);
  toRet->set_mat_table(mat_table());
  return toRet;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
NuclideModelPtr Component::copyNuclideModel(NuclideModelPtr src){
  NuclideModelPtr toRet;
  switch(src->type())
  {
    case DEGRATE_NUCLIDE:
      toRet = NuclideModelPtr(DegRateNuclide::create());
      break;
    case LUMPED_NUCLIDE:
      toRet = NuclideModelPtr(LumpedNuclide::create());
      break;
    case MIXEDCELL_NUCLIDE:
      toRet = NuclideModelPtr(MixedCellNuclide::create());
      break;
    case ONEDIMPPM_NUCLIDE:
      toRet = NuclideModelPtr(OneDimPPMNuclide::create());
      break;
    case STUB_NUCLIDE:
      toRet = NuclideModelPtr(StubNuclide::create());
      break;
    default:
      throw CycException("Unknown nuclide model enum value encountered when copying."); 
  }      
  toRet->copy(*src);
  toRet->set_mat_table(mat_table());
  return toRet;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const std::vector<NuclideModelPtr> Component::nuclide_daughters(){
  std::vector<NuclideModelPtr> to_ret;
  std::vector<ComponentPtr>::iterator daughter;
  for( daughter = daughters_.begin(); daughter!=daughters_.end(); ++daughter){
    to_ret.push_back((*daughter)->nuclide_model());
  }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Component::addComponentToTable(ComponentPtr comp){
  EM->newEvent("components")
    ->addVal("compID", comp->ID())
    ->addVal("parentID", 0) // @TODO update with parent in setparent
    ->addVal("compType", int(comp->type()))
    ->addVal("name", comp->name())
    ->addVal("material_data", comp->mat_table()->mat())
    ->addVal("nuclidemodel", comp->nuclide_model()->name())
    ->addVal("thermalmodel", comp->thermal_model()->name())
    ->addVal("innerradius", comp->inner_radius())
    ->addVal("outerradius", comp->outer_radius())
    ->addVal("length", comp->geom()->length())
    ->addVal("x", comp->x())
    ->addVal("y", comp->y())
    ->addVal("z", comp->z())
    ->record();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const int Component::ID(){return ID_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const std::string Component::name(){return name_;} 

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void Component::set_mat_table(std::string mat){
  mat_table_ = MatDataTablePtr(MDB->table(mat));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void Component::set_mat_table(MatDataTablePtr mat_table){
  mat_table_ = MatDataTablePtr(mat_table);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const MatDataTablePtr Component::mat_table(){return mat_table_;} 

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const std::vector<ComponentPtr> Component::daughters(){return daughters_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
ComponentPtr Component::parent(){return parent_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const deque<mat_rsrc_ptr> Component::wastes(){return nuclide_model()->wastes();}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const Temp Component::temp_lim(){return temp_lim_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const Temp Component::peak_temp(BoundaryType type) { 
  return (type==INNER)?peak_inner_temp_:peak_outer_temp_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const Temp Component::temp(){return temp_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const Radius Component::inner_radius(){return geom_->inner_radius();}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const Radius Component::outer_radius(){return geom_->outer_radius();}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const point_t Component::centroid(){return geom_->centroid();}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const double Component::x(){return geom_->x();}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const double Component::y(){return geom_->y();}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
const double Component::z(){return geom_->z();}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
NuclideModelPtr Component::nuclide_model(){return nuclide_model_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
ThermalModelPtr Component::thermal_model(){return thermal_model_;}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -    
void Component::setPlacement(point_t centroid, double length){
  geom_->set_centroid(centroid);
  geom_->set_length(length); 
  nuclide_model_->update(0);
};

