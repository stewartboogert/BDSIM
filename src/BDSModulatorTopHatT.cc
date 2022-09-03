/* 
Beam Delivery Simulation (BDSIM) Copyright (C) Royal Holloway, 
University of London 2001 - 2022.

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
#include "BDSDebug.hh"
#include "BDSException.hh"
#include "BDSModulatorTopHatT.hh"

#include "G4String.hh"

#include <string>

BDSModulatorTopHatT::BDSModulatorTopHatT(G4double T0In,
					 G4double T1In,
					 G4double amplitudeOffsetIn,
					 G4double amplitudeScaleIn):
  T0(T0In),
  T1(T1In),
  offset(amplitudeOffsetIn),
  scale(amplitudeScaleIn)
{
  if (T1 < T0)
    {
      G4String msg = "T1 (" + std::to_string(T1) + ") must be greater equal than T0 (";
      msg += std::to_string(T0) + ")";
      throw BDSException(__METHOD_NAME__, msg);
    }
}

G4double BDSModulatorTopHatT::Factor(const G4ThreeVector& /*xyz*/,
				 G4double T) const
{
  return T <= T1 && T >= T0 ? offset + scale : 0;
}
