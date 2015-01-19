//  
//   BDSIM, (C) 2001-2006 
//   
//   version 0.3
//  
//
//
//   Quadrupole class
//
//   History
//
//     21 Nov 2006 by Agapov,  v.0.3
//     22 Mar 2005 by Agapov, Carter,  v.0.2
//     x  x   2002 by Blair
//
//

#include "BDSGlobalConstants.hh" 
#include "BDSDebug.hh"

#include "BDSQuadrupole.hh"

#include "BDSMaterials.hh"
#include "BDSQuadMagField.hh"
#include "BDSQuadStepper.hh"

#include "G4FieldManager.hh"
#include "G4LogicalVolume.hh"
#include "G4Polyhedra.hh"
#include "G4PVPlacement.hh"               
#include "G4Tubs.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"
#include "G4VPhysicalVolume.hh"

//============================================================

#define BDSDEBUG 1

BDSQuadrupole::BDSQuadrupole(G4String aName, G4double aLength, 
			     G4double bpRad, G4double FeRad,
			     G4double bGrad, G4double tilt, G4double outR,
                             std::list<G4double> blmLocZ, std::list<G4double> blmLocTheta,
			     G4String aTunnelMaterial, G4String aMaterial, G4String spec):
  BDSMultipole(aName, aLength, bpRad, FeRad, blmLocZ, blmLocTheta, aTunnelMaterial, aMaterial),
  itsBGrad(bGrad)
{
#ifdef BDSDEBUG 
  G4cout<< __METHOD_NAME__ << "spec=" << spec << G4endl;
#endif
  // get specific quadrupole type
  G4String qtype = getParameterValueString(spec, "type");
#ifdef BDSDEBUG 
  G4cout<< __METHOD_NAME__ << "qtype="<<qtype<<G4endl;
#endif

  SetOuterRadius(outR);
  itsTilt=tilt;
}

void BDSQuadrupole::Build() 
{
#ifdef BDSDEBUG 
  G4cout<< __METHOD_NAME__ << G4endl;
#endif
  BDSMultipole::Build();
  
  if(BDSGlobalConstants::Instance()->GetIncludeIronMagFields())
    {
      G4double polePos[4];
      G4double Bfield[3];
      
      //coordinate in GetFieldValue
      polePos[0]=-BDSGlobalConstants::Instance()->GetMagnetPoleRadius()*sin(CLHEP::pi/4);
      polePos[1]=BDSGlobalConstants::Instance()->GetMagnetPoleRadius()*cos(CLHEP::pi/4);
      polePos[2]=0.;
      polePos[3]=-999.;//flag to use polePos rather than local track
      
      itsMagField->GetFieldValue(polePos,Bfield);
      G4double BFldIron=
	sqrt(Bfield[0]*Bfield[0]+Bfield[1]*Bfield[1])*
	BDSGlobalConstants::Instance()->GetMagnetPoleSize()/
	(BDSGlobalConstants::Instance()->GetComponentBoxSize()/2-
	 BDSGlobalConstants::Instance()->GetMagnetPoleRadius());
      
      // Magnetic flux from a pole is divided in two directions
      BFldIron/=2.;
      
      BuildOuterFieldManager(4, BFldIron,CLHEP::pi/4);
    }
#ifdef BDSDEBUG 
  G4cout<< __METHOD_END__ << G4endl;
#endif
}

void BDSQuadrupole::BuildBPFieldAndStepper()
{
#ifdef BDSDEBUG
  G4cout << __METHOD_NAME__ << G4endl;
#endif

  // set up the magnetic field and stepper
#ifdef BDSDEBUG
  G4cout << __METHOD_NAME__ << " - mag field " << G4endl;
#endif


  itsMagField=new BDSQuadMagField(1*itsBGrad); 

#ifdef BDSDEBUG
  G4cout << __METHOD_NAME__ << " - eq rhs " << G4endl;
#endif

  itsEqRhs=new G4Mag_UsualEqRhs(itsMagField);

#ifdef BDSDEBUG
  G4cout << __METHOD_NAME__ << " - stepper " << G4endl;
#endif

  itsStepper=new BDSQuadStepper(itsEqRhs);

#ifdef BDSDEBUG
  G4cout << __METHOD_NAME__ << " - quadStepper " << G4endl;
#endif

  BDSQuadStepper* quadStepper = dynamic_cast<BDSQuadStepper*>(itsStepper);

#ifdef BDSDEBUG
  G4cout << __METHOD_NAME__ << " - quadStepper - set bgrad " << G4endl;
#endif


  quadStepper->SetBGrad(itsBGrad);
#ifdef BDSDEBUG
  G4cout << __METHOD_END__ << G4endl;
#endif
}

void BDSQuadrupole::BuildOuterLogicalVolume(G4bool /*OuterMaterialIsVacuum*/)
{
  // build magnet (geometry + magnetic field)
  // according to quad type
    
  G4String geometry = BDSGlobalConstants::Instance()->GetMagnetGeometry();
  
  if(geometry =="standard") 
    BuildStandardOuterLogicalVolume(); // standard - quad with poles and pockets
  else if(geometry =="cylinder")  
    BuildCylindricalOuterLogicalVolume(); // cylinder outer volume
  else //default - cylinder - standard
    BuildCylindricalOuterLogicalVolume(); // cylinder outer volume

  //
  // define sensitive volumes for hit generation
  //
  if(BDSGlobalConstants::Instance()->GetSensitiveComponents()){
#ifdef BDSDEBUG
    G4cout << "BDSQuadrupole.cc:> setting sensitive outer volume" << G4endl;
#endif
    AddSensitiveVolume(itsOuterLogicalVolume);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////
// 				Cylindrical geometry					 //
///////////////////////////////////////////////////////////////////////////////////////////


void BDSQuadrupole::BuildCylindricalOuterLogicalVolume()
{
  G4double outerRadius = itsOuterR;
  if(itsOuterR==0) outerRadius = BDSGlobalConstants::Instance()->GetComponentBoxSize()/2;

  outerRadius = outerRadius/sqrt(2.0); //Why divide by sqrt of 2?

  itsOuterLogicalVolume=
    new G4LogicalVolume(
			
			new G4Tubs(itsName+"_outer_solid",
				   itsInnerIronRadius,
				   outerRadius * sqrt(2.0),
				   itsLength/2,
				   0,CLHEP::twopi*CLHEP::radian),
			BDSMaterials::Instance()->GetMaterial("Iron"),
			itsName+"_outer");

  // insert the outer volume into the marker volume
  itsPhysiComp = 
    new G4PVPlacement(
		      0,                      // no rotation
		      (G4ThreeVector)0,                      // its position
		      itsOuterLogicalVolume,  // its logical volume
		      itsName+"_outer_phys",  // its name
		      itsMarkerLogicalVolume, // its mother  volume
		      false,                  // no boolean operation
		      0, BDSGlobalConstants::Instance()->GetCheckOverlaps());                     // copy number
  
#ifndef NOUSERLIMITS
  itsOuterUserLimits =
    new G4UserLimits("quadrupole cut",itsLength,DBL_MAX,BDSGlobalConstants::Instance()->GetMaxTime(),
		     BDSGlobalConstants::Instance()->GetThresholdCutCharged());
  itsOuterLogicalVolume->SetUserLimits(itsOuterUserLimits);
#endif
  
  // color-coding for the pole
  G4VisAttributes* VisAtt = 
    new G4VisAttributes(G4Colour(1., 0., 0.));
  VisAtt->SetForceSolid(true);
  itsOuterLogicalVolume->SetVisAttributes(VisAtt);
}

///////////////////////////////////////////////////////////////////////////////////////////
// 				Detailed geometry					 //
///////////////////////////////////////////////////////////////////////////////////////////
void BDSQuadrupole::SetVisAttributes()
{
  itsVisAttributes = new G4VisAttributes(true);
  itsVisAttributes->SetColor(1,0,0);
  itsVisAttributes->SetForceSolid(true);
  itsVisAttributes->SetVisibility(true);
}


void BDSQuadrupole::BuildStandardOuterLogicalVolume()
{
  G4double outerRadius = itsOuterR;
  if(itsOuterR==0) outerRadius = BDSGlobalConstants::Instance()->GetComponentBoxSize()/2;

  outerRadius = outerRadius/sqrt(2.0);

  G4int n_poles = 4; // number of poles
  double mag_inradius = outerRadius/2.0 * sqrt(2.0); // inner radius

  double zplanepos [2] = {0,itsLength};
  double pole_extra_length = 0.05*CLHEP::m;

  double rinner [2] = {mag_inradius, mag_inradius};
  //G4double rinner [2] = {itsInnerIronRadius,itsInnerIronRadius};
  G4double router [2] = {outerRadius * sqrt(2.0),outerRadius * sqrt(2.0)};

  double pole_inradius = itsInnerIronRadius;
  double pole_extradius = mag_inradius+pole_extra_length;
  //double itstilt = 0;

  itsOuterLogicalVolume=
    new G4LogicalVolume(

			new G4Polyhedra(itsName+"_outer_solid", 
					0.*CLHEP::degree, 
					360.*CLHEP::degree, 
					2*n_poles, 
					2, 
					zplanepos, 
					rinner, 
					router),
			BDSMaterials::Instance()->GetMaterial("Iron"),
			itsName+"_outer");

 /////////////////////////////////////////////////////////////////
  
  // Defining poles
  G4ThreeVector positionQuad = G4ThreeVector(0,0,0);
  G4Tubs* poleS
    = new G4Tubs("pole",
		 pole_inradius,
		 pole_extradius,
		 itsLength/2.0,
		 0.,
		 180.0/n_poles*CLHEP::deg);
  
  G4LogicalVolume* PoleSLV = 
    new G4LogicalVolume(poleS,             //its solid
                        BDSMaterials::Instance()->GetMaterial("Iron"),   //its material
                        "PoleSLV");        //its name
  
  
  for (G4int n = 0; n < n_poles; n++) {

    // Calculate position with respect to the reference frame 
    // of the mother volume
    G4RotationMatrix* rm = new G4RotationMatrix();
    rm->rotateZ((n+0.5)*360.0/n_poles*CLHEP::degree-itsTilt*180.0/CLHEP::pi*CLHEP::degree);
    G4ThreeVector uz = G4ThreeVector(0.,0.,itsLength/2.0);     
    G4ThreeVector position = uz;

    // Place the poles with the appropriate transformation
   
    new G4PVPlacement(rm,             //rotation,
		      uz,             //position
                      PoleSLV,            //its logical volume
                      "poleS",               //its name
                      itsOuterLogicalVolume,             //its mother  volume
                      false,                 //no boolean operation
                      n,                     //copy number
                      BDSGlobalConstants::Instance()->GetCheckOverlaps());       // checking overlaps 

  }

  // color-coding for the pole
  G4VisAttributes* VisAtt = 
    new G4VisAttributes(G4Colour(1., 0., 0.));
  VisAtt->SetForceSolid(true);
  PoleSLV->SetVisAttributes(VisAtt);

  //////////////////////////////////////////////////////////////////////////////////////////////////////////
    
  G4RotationMatrix* rm_outer = new G4RotationMatrix();
  rm_outer->rotateZ(360.0/n_poles/4.0*CLHEP::deg-itsTilt*180.0/CLHEP::pi*CLHEP::degree);
  G4ThreeVector uz = G4ThreeVector(0.,0.,-itsLength/2.0); 
  // insert the outer volume into the marker volume
  itsPhysiComp = 
    new G4PVPlacement(
		      rm_outer,                      // no rotation
		      //(G4ThreeVector)0,                      // its position
		      uz,
		      itsOuterLogicalVolume,  // its logical volume
		      itsName+"_outer_phys",  // its name
		      itsMarkerLogicalVolume, // its mother  volume
		      false,                  // no boolean operation
		      0, BDSGlobalConstants::Instance()->GetCheckOverlaps());                     // copy number
  
#ifndef NOUSERLIMITS
  itsOuterUserLimits =
    new G4UserLimits("quadrupole cut",itsLength,DBL_MAX,BDSGlobalConstants::Instance()->GetMaxTime(),
		     BDSGlobalConstants::Instance()->GetThresholdCutCharged());
  itsOuterLogicalVolume->SetUserLimits(itsOuterUserLimits);
#endif
}

BDSQuadrupole::~BDSQuadrupole()
{
}
