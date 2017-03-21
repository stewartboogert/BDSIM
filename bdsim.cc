/**
 * @file bdsim.cc
 *
 * \mainpage
 * BDSIM © 2001-@CURRENT_YEAR@
 *
 * version @BDSIM_VERSION@
 */

#include "BDSDebug.hh" 
#include "BDSExecOptions.hh"     // executable command line options 
#include "BDSGlobalConstants.hh" //  global parameters

#include <cstdlib>      // standard headers 
#include <cstdio>
#include <signal.h>

#include "G4EventManager.hh" // Geant4 includes
#include "G4GeometryManager.hh"
#include "G4ParallelWorldPhysics.hh"
#include "G4TrackingManager.hh"
#include "G4SteppingManager.hh"
#include "G4GeometryTolerance.hh"
#include "G4GenericBiasingPhysics.hh"

#include "BDSAcceleratorModel.hh"
#include "BDSBunch.hh"
#include "BDSColours.hh"
#include "BDSDetectorConstruction.hh"   
#include "BDSEventAction.hh"
#include "BDSFieldLoader.hh"
#include "BDSGeometryWriter.hh"
#include "BDSMaterials.hh"
#include "BDSModularPhysicsList.hh"
#include "BDSOutputBase.hh" 
#include "BDSOutputFactory.hh"
#include "BDSParallelWorldCurvilinear.hh"
#include "BDSParallelWorldSampler.hh"
#include "BDSParser.hh" // Parser
#include "BDSPrimaryGeneratorAction.hh"
#include "BDSRandom.hh" // for random number generator from CLHEP
#include "BDSRunAction.hh"
#include "BDSRunManager.hh"
#include "BDSSamplerRegistry.hh"
#include "BDSSDManager.hh"
#include "BDSSteppingAction.hh"
#include "BDSStackingAction.hh"
#include "BDSTrackingAction.hh"
#include "BDSUtilities.hh"
#include "BDSVisManager.hh"

//=======================================================
// Global variables 
BDSOutputBase* bdsOutput=nullptr;     ///< output interface
//=======================================================

int main(int argc,char** argv)
{
  /// Print header & program information
  G4cout<<"bdsim : version @BDSIM_VERSION@"<<G4endl;
  G4cout<<"        (C) 2001-@CURRENT_YEAR@ Royal Holloway University London"<<G4endl;
  G4cout<<"        http://www.pp.rhul.ac.uk/bdsim"<<G4endl;
  G4cout<<G4endl;

  /// Initialize executable command line options reader object
  const BDSExecOptions* execOptions = new BDSExecOptions(argc,argv);
  execOptions->Print();
  G4bool ignoreSIGINT = execOptions->IgnoreSIGINT(); // different sig catching for cmake
  
  /// Check geant4 exists in the current environment
  if (!BDS::Geant4EnvironmentIsSet())
    {G4cout << "No Geant4 environmental variables found - please source geant4.sh environment" << G4endl; exit(1);}

#ifdef BDSDEBUG
  G4cout << __FUNCTION__ << "> DEBUG mode is on." << G4endl;
#endif

  /// Parse lattice file
  auto fileName = execOptions->InputFileName();
  G4cout << __FUNCTION__ << "> Using input file : "<< fileName << G4endl;
  BDSParser::Instance(fileName);

  /// Update options generated by parser with those from executable options.
  BDSParser::Instance()->AmalgamateOptions(execOptions->Options());
  /// Check options for consistency
  BDSParser::Instance()->CheckOptions();
  
  /// Explicitly initialise materials to construct required materials before global constants.
  BDSMaterials::Instance()->PrepareRequiredMaterials(execOptions->Options().verbose);

  /// No longer needed. Everything can safely use BDSGlobalConstants from now on.
  delete execOptions; 

  /// Force construction of global constants after parser has been initialised (requires materials first).
  /// This uses the options from BDSParser.
  const BDSGlobalConstants* globalConstants = BDSGlobalConstants::Instance();

  /// Initialize random number generator
  BDSRandom::CreateRandomNumberGenerator();
  BDSRandom::SetSeed(); // set the seed from options
  
  /// Instantiate the specific type of bunch distribution (class),
  /// get the corresponding parameters from the gmad parser info
  /// and attach to the initialised random number generator.
#ifdef BDSDEBUG
  G4cout << __FUNCTION__ << "> Instantiating chosen bunch distribution." << G4endl;
#endif
  BDSBunch* bdsBunch = new BDSBunch();
  bdsBunch->SetOptions(BDSParser::Instance()->GetOptions());

  /// Optionally generate primaries only and exit
  if (globalConstants->GeneratePrimariesOnly())
    {
      // output creation is duplicated below but with this if loop, we exit so ok.
      bdsOutput = BDSOutputFactory::CreateOutput(globalConstants->OutputFormat());
      bdsOutput->Initialise();
      G4double x0=0.0, y0=0.0, z0=0.0, xp=0.0, yp=0.0, zp=0.0, t=0.0, E=0.0, weight=1.0;
      const G4int nToGenerate = globalConstants->NGenerate();
      const G4int printModulo = BDSGlobalConstants::Instance()->PrintModulo();
      for (G4int i = 0; i < nToGenerate; i++)
      {
	if (i%printModulo == 0)
	  {G4cout << "\r Primary> " << std::fixed << i << " of " << nToGenerate << G4endl;}
        bdsBunch->GetNextParticle(x0,y0,z0,xp,yp,zp,t,E,weight);
        bdsOutput->WritePrimary(E, x0, y0, z0, xp, yp, zp, t, weight, 1, i, 1);
        bdsOutput->FillEvent();
      }
      bdsOutput->Close();
      G4cout << G4endl;
      delete bdsBunch;
      delete bdsOutput;
      exit(0);
    }

  /// Construct mandatory run manager (the G4 kernel) and
  /// register mandatory initialization classes.
#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> Constructing run manager"<<G4endl;
#endif
  BDSRunManager * runManager = new BDSRunManager;
#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> Registering user action - detector construction"<<G4endl;
#endif
  /// Register the geometry and parallel world construction methods with run manager.
  BDSDetectorConstruction* realWorld    = new BDSDetectorConstruction();
  BDSParallelWorldSampler* samplerWorld = new BDSParallelWorldSampler();
  BDSParallelWorldCurvilinear* curvilinearWorld = new BDSParallelWorldCurvilinear();
  realWorld->RegisterParallelWorld(samplerWorld);
  realWorld->RegisterParallelWorld(curvilinearWorld);
  runManager->SetUserInitialization(realWorld);  

  /// For geometry sampling, phys list must be initialized before detector.
  /// BUT for samplers we use a parallel world and this HAS to be before the physcis
#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> Constructing physics processes" << G4endl;
#endif
  G4String physicsListName = BDSParser::Instance()->GetOptions().physicsList;
  // Note, we purposively don't create a parallel world process for the curvilinear
  // world as we don't need the track information from it - unreliable that way. We
  // query the geometry directly using our BDSAuxiliaryNavigator class.
  G4ParallelWorldPhysics* sampWorld = new G4ParallelWorldPhysics(samplerWorld->GetName());
  BDSModularPhysicsList*  physList  = new BDSModularPhysicsList(physicsListName);
  physList->RegisterPhysics(sampWorld);
  physList->BuildAndAttachBiasWrapper(BDSParser::Instance()->GetBiasing());
  runManager->SetUserInitialization(physList);
  
  /// Print the geometry tolerance
  G4GeometryTolerance* theGeometryTolerance = G4GeometryTolerance::GetInstance();
  G4cout << __FUNCTION__ << "> Geometry Tolerances: "     << G4endl;
  G4cout << __FUNCTION__ << ">" << std::setw(22) << "Surface: " << std::setw(10) << theGeometryTolerance->GetSurfaceTolerance() << " mm"   << G4endl;
  G4cout << __FUNCTION__ << ">" << std::setw(22) << "Angular: " << std::setw(10) << theGeometryTolerance->GetAngularTolerance()          << " rad" << G4endl;
  G4cout << __FUNCTION__ << ">" << std::setw(22) << "Radial: "  << std::setw(10) << theGeometryTolerance->GetRadialTolerance()  << " mm"   << G4endl;

  /// Set user action classes
#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> Registering user action - Run Action"<<G4endl;
#endif
  runManager->SetUserAction(new BDSRunAction);

#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> Registering user action - Event Action"<<G4endl;
#endif
  runManager->SetUserAction(new BDSEventAction());

#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> Registering user action - Stepping Action"<<G4endl;
#endif
  // Only add steppingaction if it is actually used, so do check here (for cpu reasons)
  G4bool verboseStepping = globalConstants->VerboseStep();
  if (globalConstants->ThresholdCutPhotons() > 0 || globalConstants->ThresholdCutCharged() > 0
      || verboseStepping)
    {
      G4int verboseEventNumber = globalConstants->VerboseEventNumber();
      runManager->SetUserAction(new BDSSteppingAction(verboseStepping, verboseEventNumber));
    }
  
#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> Registering user action - Tracking Action"<<G4endl;
#endif
  runManager->SetUserAction(new BDSTrackingAction(globalConstants->Batch()));

#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> Registering user action - Stacking Action"<<G4endl;
#endif
  runManager->SetUserAction(new BDSStackingAction);

#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> Registering user action - Primary Generator"<<G4endl;
#endif
  runManager->SetUserAction(new BDSPrimaryGeneratorAction(bdsBunch));

  /// Initialize G4 kernel
#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> Initialising Geant4 kernel"<<G4endl;
#endif
  runManager->Initialize();

  /// Implement bias operations on all volumes only after G4RunManager::Initialize()
  realWorld->BuildPhysicsBias();

#ifdef BDSDEBUG
  auto physics = runManager->GetUserPhysicsList();
  if (const BDSModularPhysicsList* modPhysics = dynamic_cast<const BDSModularPhysicsList*>(physics))
    {
      modPhysics->PrintPrimaryParticleProcesses();
      modPhysics->PrintDefinedParticles();
    }
#endif

  /// Set verbosity levels
  runManager->SetVerboseLevel(globalConstants->VerboseRunLevel());
  G4EventManager::GetEventManager()->SetVerboseLevel(globalConstants->VerboseEventLevel());
  G4EventManager::GetEventManager()->GetTrackingManager()->SetVerboseLevel(globalConstants->VerboseTrackingLevel());
  G4EventManager::GetEventManager()->GetTrackingManager()->GetSteppingManager()->SetVerboseLevel(globalConstants->VerboseSteppingLevel());
  
  /// Close the geometry in preparation for running - everything is now fixed.
  G4bool bCloseGeometry = G4GeometryManager::GetInstance()->CloseGeometry();
  if(!bCloseGeometry)
    { 
      G4cerr << __FUNCTION__ << "> error - geometry not closed." << G4endl;
      return 1;
    }

  if (globalConstants->ExportGeometry())
    {
      BDSGeometryWriter geometrywriter;
      geometrywriter.ExportGeometry(globalConstants->ExportType(),
				    globalConstants->ExportFileName());
    }
  else
    {
      /// Construct output
#ifdef BDSDEBUG
      G4cout << __FUNCTION__ << "> Setting up output." << G4endl;
#endif
      bdsOutput = BDSOutputFactory::CreateOutput(globalConstants->OutputFormat());
      G4cout.precision(10);

      /// Catch aborts to close output stream/file. perhaps not all are needed.
      struct sigaction act;
      act.sa_handler = &BDS::HandleAborts;
      sigemptyset(&act.sa_mask);
      act.sa_flags = 0;
      if (!ignoreSIGINT)
	{sigaction(SIGINT,  &act, 0);}
      sigaction(SIGABRT, &act, 0);
      sigaction(SIGTERM, &act, 0);
      sigaction(SIGSEGV, &act, 0);
      
      /// Run in either interactive or batch mode
      if(!globalConstants->Batch())   // Interactive mode
	{
	  BDSVisManager visManager;
	  visManager.StartSession(argc,argv);
	}
      else           // Batch mode
	{runManager->BeamOn(globalConstants->NGenerate());}
    }

  /// Termination & clean up.
  G4GeometryManager::GetInstance()->OpenGeometry();

#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> BDSOutput deleting..."<<G4endl;
#endif
  delete bdsOutput;
  
#ifdef BDSDEBUG 
  G4cout << __FUNCTION__ << "> instances deleting..."<<G4endl;
#endif
  delete BDSAcceleratorModel::Instance();
  delete globalConstants;
  delete BDSMaterials::Instance();
  delete BDSParser::Instance();

  // instances not used in this file, but no other good location for deletion
  delete BDSColours::Instance();
  delete BDSFieldLoader::Instance();
  delete BDSSDManager::Instance();
  delete BDSSamplerRegistry::Instance();
  
#ifdef BDSDEBUG 
  G4cout<< __FUNCTION__ << "> BDSRunManager deleting..."<<G4endl;
#endif
  delete runManager;
  delete samplerWorld;
  delete bdsBunch;

  G4cout << __FUNCTION__ << "> End of Run, Thank you for using BDSIM!" << G4endl;

  return 0;
}
