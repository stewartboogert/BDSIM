#include "BDSScreen.hh"

#include "BDSDebug.hh"
#include "BDSGlobalConstants.hh"
#include "BDSMultilayerScreen.hh"
#include "BDSScreenLayer.hh"

#include "G4Colour.hh"
#include "G4VisAttributes.hh"

#include <list>
#include <sstream>

BDSScreen::BDSScreen(G4String aName,  
		     G4double aLength,
		     BDSBeamPipeInfo* beamPipeInfo, 
		     G4TwoVector sizeIn, //X Y dimensions of screen
		     G4double screenAngleIn):
  BDSDrift(aName,aLength,beamPipeInfo),
  size(sizeIn), 
  screenAngle(screenAngleIn)
{
  screenRot = new G4RotationMatrix();
  screenRot->rotateY(screenAngle);
  screenPos.setX(0);
  screenPos.setY(0);
  screenPos.setZ(0);
  mlScreen = new BDSMultilayerScreen(size, name+"_mlscreen");
  nLayers=0;
}

BDSScreen::~BDSScreen(){
  delete screenRot;
  delete mlScreen;
}

void BDSScreen::Build(){
  BDSDrift::Build(); //Build the beam pipe geometry.
  G4VisAttributes* VisAtt1 = new G4VisAttributes(G4Colour(0.4, 0.4, 0.4, 0.3));
  VisAtt1->SetForceWireframe(true);
  VisAtt1->SetVisibility(true);
  containerLogicalVolume->SetVisAttributes(VisAtt1);

  PlaceScreen(); //Place the screen in the beam pipe
}

// TODO
// void BDSScreen::BuildFieldAndStepper(){
//   G4cout << __METHOD_NAME__ << " - building bmap field and stepper." << G4endl;
//   BuildBmapFieldAndStepper();
// }

void BDSScreen::screenLayer(G4double thickness, G4String material, G4int isSampler){
  std::stringstream ss;
  ss << nLayers;
  G4String lNum = ss.str();
  G4String lName = name+"_"+lNum;
  mlScreen->screenLayer(thickness,material,lName, isSampler);
  if(!isSampler) RegisterSensitiveVolume(mlScreen->lastLayer()->log());
  nLayers++;
}

void BDSScreen::PlaceScreen(){
  mlScreen->build();//Build the screen.
  mlScreen->place(screenRot, screenPos, containerLogicalVolume); //Place the screen in the beampipe centre. // TODO check if containerlogical volume is correct here
}
