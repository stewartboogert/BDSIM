#include <include/BDSGlobalConstants.hh>
#include "BDSDebug.hh"
#include "BDSIntegratorDipoleFringe.hh"
#include "BDSMagnetStrength.hh"
#include "BDSStep.hh"

#include "G4AffineTransform.hh"
#include "G4Mag_EqRhs.hh"
#include "G4MagIntegratorStepper.hh"
#include "G4ThreeVector.hh"

BDSIntegratorDipoleFringe::BDSIntegratorDipoleFringe(BDSMagnetStrength const* strength,
						     G4Mag_EqRhs*             eqOfMIn,
						     G4double                 minimumRadiusOfCurvature):
  BDSIntegratorDipole2(eqOfMIn, minimumRadiusOfCurvature),
  polefaceAngle((*strength)["polefaceangle"]),
  fringeCorr((*strength)["fringecorr"])
{;}

void BDSIntegratorDipoleFringe::Stepper(const G4double yIn[],
					const G4double dydx[],
					const G4double h,
					G4double       yOut[],
					G4double       yErr[])
{
  // do the dipole kick using base class
  BDSIntegratorDipole2::Stepper(yIn, dydx, h, yOut, yErr);

  // don't do fringe kick if we're sampling the field  for a long step
  if (h > 1*CLHEP::cm)
    {return;}

  G4ThreeVector pos = G4ThreeVector(yIn[0], yIn[1], yIn[2]);
  G4ThreeVector mom = G4ThreeVector(yIn[3], yIn[4], yIn[5]);
  
  // global to local (curvilinear) - 'true' = use local volume for transform
  BDSStep    localPosMom = ConvertToLocal(pos, mom, 0.5*CLHEP::um, true);
  G4ThreeVector localPos = localPosMom.PreStepPoint();
  G4ThreeVector localMom = localPosMom.PostStepPoint();

  // check for forward going paraxial particles - only 
  if (localMom.unit().z() < 0.9)
    {return;}
  
  // get radius of curvature already calculated in base class step
  G4double rho = GetRadHelix();

  G4double momMag = localMom.mag();
  G4ThreeVector normMom = localMom.unit();

  // fraction of kick applied in case of multiple steps in element
  G4double fraction = h / BDSGlobalConstants::Instance()->ThinElementLength();
  // prevent overkicking
  if (fraction > 1){
    fraction = 1;
  }

  // calculate fractional fringe field kick
  G4double          y0 = localPos[1] / CLHEP::m;
  G4double ymatElement = tan(polefaceAngle - fringeCorr) / (rho / CLHEP::m);
  G4double       ykick = fraction * y0 * ymatElement;

  // apply kick to unit momentum and recalculate zp to conserve.
  normMom[1] += ykick;
  normMom[2] = sqrt(1.0 - pow(normMom[0],2) - pow(normMom[1],2));
  normMom *= momMag;
  G4ThreeVector globalMom = ConvertAxisToGlobal(normMom, true);

  // update output momentum coordinates - i+3 for momentum
  for (G4int i = 0; i < 3; i++)
    {yOut[i+3] = globalMom[i];}
}
