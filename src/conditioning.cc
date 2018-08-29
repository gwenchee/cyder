// conditioning.cc
// Implements the Conditioning class

#include "conditioning.h"

namespace conditioning {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Conditioning::Conditioning(cyclus::Context* ctx) 
    : cyclus::Facility(ctx),
      latitude(0.0),
      longitude(0.0),
      coordinates(latitude, longitude) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>(
"The Conditioning Facility is experimental.");};

} // namespace conditioning