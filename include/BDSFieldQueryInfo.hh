/* 
Beam Delivery Simulation (BDSIM) Copyright (C) Royal Holloway, 
University of London 2001 - 2021.

This file is part of BDSIM.

BDSIM is free software: you can redistribute it and/or modify 
it under the terms of the GNU General Public License as published 
by the Free Software Foundation version 3 of the License.

BDSIM is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BDSIM.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef BDSFIELDQUERYINFO_H
#define BDSFIELDQUERYINFO_H

#include "BDSFourVector.hh"

#include "G4AffineTransform.hh"
#include "G4String.hh"
#include "G4Types.hh"

#include <vector>

/**
 * @brief Holder class for all information required for a field query.
 * 
 * @author Laurie Nevay
 */

class BDSFieldQueryInfo
{
public:
  struct QueryDimensionInfo
  {
    G4int n = 1;
    G4double min = 0;
    G4double max = 0;
  };

  /// Usual constructor with number of points to query in each dimension.
  BDSFieldQueryInfo(const G4String& nameIn,
                    const G4String& outfileMagneticIn,
                    const G4String& outfileElectricIn,
                    G4bool queryMagneticIn,
                    G4bool queryElectricIn,
                    QueryDimensionInfo xInfoIn,
                    QueryDimensionInfo yInfoIn,
                    QueryDimensionInfo zInfoIn,
                    QueryDimensionInfo tInfoIn,
                    const G4AffineTransform& globalTransformIn = G4AffineTransform(),
                    G4bool overwriteExistingFilesIn = false);

  /// Alternative constructor with list of exact points to query.
  BDSFieldQueryInfo(const G4String& nameIn,
                    const G4String& outfileMagneticIn,
                    const G4String& outfileElectricIn,
                    G4bool queryMagneticIn,
                    G4bool queryElectricIn,
		    const std::vector<BDSFourVector<G4double>>& pointsToQueryIn,
		    G4bool overwriteExistingFilesIn = false);
  ~BDSFieldQueryInfo();
  
  G4String name;
  G4String outfileMagnetic;
  G4String outfileElectric;
  G4bool   queryMagnetic;
  G4bool   queryElectric;
  QueryDimensionInfo xInfo;
  QueryDimensionInfo yInfo;
  QueryDimensionInfo zInfo;
  QueryDimensionInfo tInfo;

  std::vector<BDSFourVector<G4double>> pointsToQuery;
  
  G4AffineTransform globalTransform;
  
  G4bool overwriteExistingFiles;
  
  /// Whether to query a specific set of points.
  G4bool SpecificPoints() const {return !pointsToQuery.empty();}
};

#endif
