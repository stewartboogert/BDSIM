/*
Beam Delivery Simulation (BDSIM) Copyright (C) Royal Holloway,
University of London 2001 - 2018.

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

#ifndef BDSUNDULATOR_H
#define BDSUNDULATOR_H

#include "globals.hh"
#include "BDSAcceleratorComponent.hh"
#include "BDSFieldInfo.hh"
/**
 * @brief Undulator.
 *
 * @author Will Shields
 */

class BDSUndulator: public BDSAcceleratorComponent
{
public:
  BDSUndulator(G4String name,
               G4double length,
               G4double outerDiameter,
               G4double periodIn,
               G4double magnetHeightIn,
               G4double magnetWidthIn,
               G4double undulatorGapIn,
               G4double undulatorLengthIn,
               BDSBeamPipeInfo* beamPipeInfoIn,
               BDSFieldInfo*   vacuumFieldInfoIn,
               G4String materialIn = "iron"
  );

    virtual ~BDSUndulator();

protected:
  virtual void Build();

  virtual void BuildContainerLogicalVolume();

  /// Function for building geometry for a single undulator period.
  void BuildUndulatorMagnet();

  G4double outerDiameter;
  BDSFieldInfo* vacuumFieldInfo;
  G4double undulatorPeriod;
  G4String material;
  G4double magnetHeight;
  G4double magnetWidth;
  G4double undulatorGap;
  G4double undulatorLength;
};

#endif