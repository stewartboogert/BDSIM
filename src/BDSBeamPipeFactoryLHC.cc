#include "BDSBeamPipeFactoryBase.hh"
#include "BDSBeamPipeFactoryLHC.hh"
#include "BDSBeamPipe.hh"
#include "BDSDebug.hh"
#include "BDSExtent.hh"
#include "BDSGlobalConstants.hh"

#include "globals.hh"                      // geant4 globals / types
#include "G4Box.hh"
#include "G4CutTubs.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4SubtractionSolid.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4VSolid.hh"

#include <cmath>
#include <utility>                         // for std::pair


BDSBeamPipeFactoryLHC* BDSBeamPipeFactoryLHC::instance = nullptr;

BDSBeamPipeFactoryLHC* BDSBeamPipeFactoryLHC::Instance()
{
  if (instance == nullptr)
    {instance = new BDSBeamPipeFactoryLHC();}
  return instance;
}

BDSBeamPipeFactoryLHC::BDSBeamPipeFactoryLHC()
{;}

BDSBeamPipeFactoryLHC::~BDSBeamPipeFactoryLHC()
{
  instance = nullptr;
}

BDSBeamPipe* BDSBeamPipeFactoryLHC::CreateBeamPipe(G4String    nameIn,
						   G4double    lengthIn,
						   G4double    aper1In,
						   G4double    aper2In,
						   G4double    aper3In,
						   G4double    /*aper4In*/,
						   G4Material* vacuumMaterialIn,
						   G4double    beamPipeThicknessIn,
						   G4Material* beamPipeMaterialIn)
{
#ifdef BDSDEBUG
  G4cout << __METHOD_NAME__ << G4endl;
#endif
   // clean up after last usage
  CleanUp();
  
  // build the solids
  //vacuum cylindrical solid (circular cross-section)
  G4VSolid* vacCylSolid = new G4Tubs(nameIn + "_vacuum_cylinder", // name
				     0,                           // inner radius
				     aper3In,                     // outer radius
				     lengthIn*0.5-2*lengthSafety, // half length
				     0,                           // rotation start angle
				     CLHEP::twopi);               // rotation finish angle
  //vacuum box solid (rectangular cross-section)
  G4VSolid* vacRectSolid = new G4Box(nameIn + "_vacuum_box", // name
				     aper1In,                // x half width
				     aper2In,                // y half width
				     lengthIn); // z full width (long for unambiguous intersection)
  allSolids.push_back(vacCylSolid);
  allSolids.push_back(vacRectSolid);
  //intersection of both of these gives the desired shape
  vacuumSolid = new G4IntersectionSolid(nameIn + "_vacuum_solid", // name
					vacCylSolid,              // solid 1
					vacRectSolid);            // solid 2

  //beampipe cylindrical solid (circular cross-section)
  //beampipe inner edge for subtraction (actually just like vacuum + lengthSafety)x
  G4VSolid* bpInnerCylSolid = new G4Tubs(nameIn + "_pipe_inner_cylinder", // name
					 0,                               // inner radius
					 aper3In + lengthSafety,          // outer radius
					 1.5*lengthIn,   // length big for unambiguous subtraction (but < outerlength)
					 0,                               // rotation start angle
					 CLHEP::twopi);                   // rotation finish angle
  //beampipe inner edge box solid (rectangular cross-section)
  G4VSolid* bpInnerRectSolid = new G4Box(nameIn + "_pipe_inner_box", // name
					 aper1In + lengthSafety,     // x half width
					 aper2In + lengthSafety,     // y half width
					 1.7*lengthIn); // z long for unambiguous intersection
  //beampipe inner intersection - 1.5*length long which is > half length for unambiguous subtraction later
  G4VSolid* bpInnerSolid = new G4IntersectionSolid(nameIn + "_pipe_inner_solid", // name
						   bpInnerCylSolid,              // solid 1
						   bpInnerRectSolid);            // solid 2

  //beampipe outer edge for subtraction (actually just like vacuum + lengthSafety)x
  G4VSolid* bpOuterCylSolid = new G4Tubs(nameIn + "_pipe_inner_cylinder", // name
					 0,                               // inner radius (0 for unambiguous subtraction)
					 aper3In + beamPipeThicknessIn,   // outer radius
					 (lengthIn*0.5)-2*lengthSafety,   // half length
					 0,                               // rotation start angle
					 CLHEP::twopi);                   // rotation finish angle
  //beampipe outer edge box solid (rectangular cross-section)
  G4VSolid* bpOuterRectSolid = new G4Box(nameIn + "_pipe_inner_box",    // name
					 aper1In + beamPipeThicknessIn, // x half width
					 aper2In + beamPipeThicknessIn, // y half width
					 lengthIn); // z full width (long for unambiguous intersection)
  G4VSolid* bpOuterSolid = new G4IntersectionSolid(nameIn + "_pipe_inner_solid", // name
						   bpOuterCylSolid,              // solid 1
						   bpOuterRectSolid);            // solid 2
  allSolids.push_back(bpInnerCylSolid);
  allSolids.push_back(bpInnerRectSolid);
  allSolids.push_back(bpInnerSolid);
  allSolids.push_back(bpOuterCylSolid);
  allSolids.push_back(bpOuterRectSolid);
  allSolids.push_back(bpOuterSolid);

  beamPipeSolid = new G4SubtractionSolid(nameIn + "_pipe_solid",  // name
					 bpOuterSolid,            // this
  					 bpInnerSolid);           // minus this
  
  //container cylindrical solid (circular cross-section)
  G4VSolid* contCylSolid = new G4Tubs(nameIn + "_vacuum_cylinder", // name
				     0,                           // inner radius
				     aper3In + beamPipeThicknessIn + lengthSafety, // outer radius
				     lengthIn*0.5, // half length
				     0,                           // rotation start angle
				     CLHEP::twopi);               // rotation finish angle
  //vacuum box solid (rectangular cross-section)
  G4VSolid* contRectSolid = new G4Box(nameIn + "_vacuum_box", // name
				     aper1In + beamPipeThicknessIn + lengthSafety, // x half width
				     aper2In + beamPipeThicknessIn + lengthSafety, // y half width
				     lengthIn); // z full width (long for unambiguous intersection)

  allSolids.push_back(contCylSolid);
  allSolids.push_back(contRectSolid);
  //intersection of both of these gives the desired shape
  containerSolid = new G4IntersectionSolid(nameIn + "_vacuum_solid", // name
					   contCylSolid,              // solid 1
					   contRectSolid);            // solid 2

  G4double width  = aper3In + beamPipeThicknessIn + lengthSafety;
  G4double height = aper2In + beamPipeThicknessIn + lengthSafety;

  CreateContainerSubtractionSolid(nameIn, lengthIn, beamPipeThicknessIn, aper1In, aper2In, aper3In);
  
  return CommonFinalConstruction(nameIn, vacuumMaterialIn, beamPipeMaterialIn, lengthIn, width, height);
}

BDSBeamPipe* BDSBeamPipeFactoryLHC::CreateBeamPipe(G4String      nameIn,
						   G4double      lengthIn,
						   G4ThreeVector inputFaceNormalIn,
						   G4ThreeVector outputFaceNormalIn,
						   G4double      aper1In,
						   G4double      aper2In,
						   G4double      aper3In,
						   G4double      /*aper4In*/,
						   G4Material*   vacuumMaterialIn,
						   G4double      beamPipeThicknessIn,
						   G4Material*   beamPipeMaterialIn)
{
#ifdef BDSDEBUG
  G4cout << __METHOD_NAME__ << G4endl;
#endif
  // clean up after last usage
  CleanUp();

  inputFaceNormal  = inputFaceNormalIn;
  outputFaceNormal = outputFaceNormalIn;

  G4double width  = aper3In + beamPipeThicknessIn + lengthSafety;
  G4double height = aper2In + beamPipeThicknessIn + lengthSafety;
  
  CreateGeneralAngledSolids(nameIn, lengthIn, aper1In, aper2In, aper3In, beamPipeThicknessIn,
			    inputFaceNormal, outputFaceNormal);
  CreateContainerSubtractionSolid(nameIn, lengthIn, beamPipeThicknessIn,
				  aper1In, aper2In, aper3In);
  
  return CommonFinalConstruction(nameIn, vacuumMaterialIn, beamPipeMaterialIn,
				 lengthIn, width, height);
}

BDSBeamPipe* BDSBeamPipeFactoryLHC::CommonFinalConstruction(G4String    nameIn,
							    G4Material* vacuumMaterialIn,
							    G4Material* beamPipeMaterialIn,
							    G4double    lengthIn,
							    G4double    containerWidthIn,
							    G4double    containerHeightIn)
{
#ifdef BDSDEBUG
  G4cout << __METHOD_NAME__ << G4endl;
#endif

  BDSBeamPipeFactoryBase::CommonConstruction(nameIn,
					     vacuumMaterialIn,
					     beamPipeMaterialIn,
					     lengthIn);
  
  // record extents
  BDSExtent ext = BDSExtent(containerWidthIn, containerHeightIn, lengthIn*0.5);
  
  // build the BDSBeamPipe instance and return it
  return BuildBeamPipeAndRegisterVolumes(ext, containerWidthIn);
}


/// the angled ones have degeneracy in the geant4 solids they used so we can avoid code duplication
/// by grouping common construction tasks
void BDSBeamPipeFactoryLHC::CreateGeneralAngledSolids(G4String      nameIn,
						      G4double      lengthIn,
						      G4double      aper1In,
						      G4double      aper2In,
						      G4double      aper3In,
						      G4double      beamPipeThicknessIn,
						      G4ThreeVector inputfaceIn,
						      G4ThreeVector outputfaceIn)
{
  // build the solids
  //vacuum cylindrical solid (circular cross-section)
  G4VSolid* vacCylSolid = new G4CutTubs(nameIn + "_vacuum_cylinder", // name
					0,                           // inner radius
					aper3In,                     // outer radius
					lengthIn*0.5-2*lengthSafety, // half length
					0,                           // rotation start angle
					CLHEP::twopi,                // rotation finish angle
					inputfaceIn,                 // input face normal
					outputfaceIn);               // output face normal
  //vacuum box solid (rectangular cross-section)
  G4VSolid* vacRectSolid = new G4Box(nameIn + "_vacuum_box", // name
				     aper1In,                // x half width
				     aper2In,                // y half width
				     lengthIn); // z full width (long for unambiguous intersection)
  allSolids.push_back(vacCylSolid);
  allSolids.push_back(vacRectSolid);
  //intersection of both of these gives the desired shape
  vacuumSolid = new G4IntersectionSolid(nameIn + "_vacuum_solid", // name
					vacCylSolid,              // solid 1
					vacRectSolid);            // solid 2

  //beampipe cylindrical solid (circular cross-section)
  //beampipe inner edge for subtraction (actually just like vacuum + lengthSafety)x
  G4VSolid* bpInnerCylSolid = new G4CutTubs(nameIn + "_pipe_inner_cylinder", // name
					    0,                               // inner radius
					    aper3In + lengthSafety,          // outer radius
					    1.5*lengthIn,   // length big for unambiguous subtraction (but < outerlength)
					    0,                               // rotation start angle
					    CLHEP::twopi,                    // rotation finish angle
					    inputfaceIn,                     // input face normal
					    outputfaceIn);                   // output face normal
  
  //beampipe inner edge box solid (rectangular cross-section)
  G4VSolid* bpInnerRectSolid = new G4Box(nameIn + "_pipe_inner_box", // name
					 aper1In + lengthSafety,     // x half width
					 aper2In + lengthSafety,     // y half width
					 1.7*lengthIn); // z long for unambiguous intersection
  //beampipe inner intersection - 1.5*length long which is > half length for unambiguous subtraction later
  G4VSolid* bpInnerSolid = new G4IntersectionSolid(nameIn + "_pipe_inner_solid", // name
						   bpInnerCylSolid,              // solid 1
						   bpInnerRectSolid);            // solid 2

  //beampipe outer edge for subtraction (actually just like vacuum + lengthSafety)x
  G4VSolid* bpOuterCylSolid = new G4CutTubs(nameIn + "_pipe_inner_cylinder", // name
					    0,                               // inner radius (0 for unambiguous subtraction)
					    aper3In + beamPipeThicknessIn,   // outer radius
					    (lengthIn*0.5)-2*lengthSafety,   // half length
					    0,                               // rotation start angle
					    CLHEP::twopi,                    // rotation finish angle
					    inputfaceIn,                     // input face normal
					    outputfaceIn);                   // output face normal
  
  //beampipe outer edge box solid (rectangular cross-section)
  G4VSolid* bpOuterRectSolid = new G4Box(nameIn + "_pipe_inner_box",    // name
					 aper1In + beamPipeThicknessIn, // x half width
					 aper2In + beamPipeThicknessIn, // y half width
					 lengthIn); // z full width (long for unambiguous intersection)
  G4VSolid* bpOuterSolid = new G4IntersectionSolid(nameIn + "_pipe_inner_solid", // name
						   bpOuterCylSolid,              // solid 1
						   bpOuterRectSolid);            // solid 2

  allSolids.push_back(bpInnerCylSolid);
  allSolids.push_back(bpInnerRectSolid);
  allSolids.push_back(bpInnerSolid);
  allSolids.push_back(bpOuterCylSolid);
  allSolids.push_back(bpOuterRectSolid);
  allSolids.push_back(bpOuterSolid);

  beamPipeSolid = new G4SubtractionSolid(nameIn + "_pipe_solid",  // name
					 bpOuterSolid,            // this
  					 bpInnerSolid);           // minus this
  
  //container cylindrical solid (circular cross-section)
  G4VSolid* contCylSolid = new G4CutTubs(nameIn + "_vacuum_cylinder", // name
					 0,                           // inner radius
					 aper3In + beamPipeThicknessIn + lengthSafety, // outer radius
					 lengthIn*0.5, // half length
					 0,                           // rotation start angle
					 CLHEP::twopi,                // rotation finish angle
					 inputfaceIn,                 // input face normal
					 outputfaceIn);               // output face normal
  
  //vacuum box solid (rectangular cross-section)
  G4VSolid* contRectSolid = new G4Box(nameIn + "_vacuum_box", // name
				      aper1In + beamPipeThicknessIn + lengthSafety, // x half width
				      aper2In + beamPipeThicknessIn + lengthSafety, // y half width
				      lengthIn); // z full width (long for unambiguous intersection)

  allSolids.push_back(contCylSolid);
  allSolids.push_back(contRectSolid);
  //intersection of both of these gives the desired shape
  containerSolid = new G4IntersectionSolid(nameIn + "_vacuum_solid", // name
					   contCylSolid,              // solid 1
					   contRectSolid);            // solid 2
}

void BDSBeamPipeFactoryLHC::CreateContainerSubtractionSolid(G4String& nameIn,
							    G4double& lengthIn,
							    G4double& beamPipeThicknessIn,
							    G4double& aper1In,
							    G4double& aper2In,
							    G4double& aper3In)
{
  //container cylindrical solid (circular cross-section)
  G4VSolid* contSubCylSolid = new G4Tubs(nameIn + "_subtraction_cylinder", // name
					 0,                                // inner radius
					 aper3In + beamPipeThicknessIn + lengthSafety, // outer radius
					 2*lengthIn,                       // long length for unambiguous subtraction
					 0,                                // rotation start angle
					 CLHEP::twopi);                    // rotation finish angle
  //vacuum box solid (rectangular cross-section)
  G4VSolid* contSubRectSolid = new G4Box(nameIn + "_subtraction_box",                  // name
					 aper1In + beamPipeThicknessIn + lengthSafety, // x half width
					 aper2In + beamPipeThicknessIn + lengthSafety, // y half width
					 1.7*lengthIn); // z full width (long for unambiguous intersection)
  allSolids.push_back(contSubCylSolid);
  allSolids.push_back(contSubRectSolid);
  //intersection of both of these gives the desired shape
  containerSubtractionSolid = new G4IntersectionSolid(nameIn + "_subtraction_solid", // name
						      contSubCylSolid,               // solid 1
						      contSubRectSolid);             // solid 2
}
