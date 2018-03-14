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
#include "BDSDebug.hh"
#include "BDSLine.hh"

#include <vector>

BDSLine::BDSLine(G4String nameIn):
  BDSAcceleratorComponent(nameIn,0,0,"line")
{;}

void BDSLine::AddComponent(BDSAcceleratorComponent* component)
{line.push_back(component);}

void BDSLine::Initialise()
{
#ifdef BDSDEBUG
  G4cout << __METHOD_NAME__ << G4endl;
#endif
  for (iterator it = begin(); it != end(); ++it)
    {
#ifdef BDSDEBUG
      G4cout << __METHOD_NAME__ << "Initialising component named: " << (*it)->GetName() << G4endl;
#endif
      (*it)->Initialise();
    }
}

void BDSLine::SetBiasVacuumList(std::list<std::string> biasVacuumListIn)
{
  BDSAcceleratorComponent::SetBiasVacuumList(biasVacuumListIn); // set if for this object
  for (auto component : *this)
    {component->SetBiasVacuumList(biasVacuumListIn);}
}

void BDSLine::SetBiasMaterialList(std::list<std::string> biasMaterialListIn)
{
  BDSAcceleratorComponent::SetBiasMaterialList(biasMaterialListIn);
  for (auto component : *this)
    {component->SetBiasMaterialList(biasMaterialListIn);}
}

void BDSLine::SetRegion(G4String regionIn)
{
  BDSAcceleratorComponent::SetRegion(regionIn); // set it for this object
  for (auto component: *this)
    {component->SetRegion(regionIn);}
}

  
