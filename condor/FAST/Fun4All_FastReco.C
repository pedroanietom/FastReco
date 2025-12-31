#include <G4_ActsGeom.C>
#include <G4_Global.C>
#include <G4_Magnet.C>
#include <GlobalVariables.C>
#include <Trkr_Clustering.C>
#include <Trkr_Reco.C>
#include <Trkr_RecoInit.C>
#include <Trkr_TpcReadoutInit.C>
#include <QA.C>

#include <ffamodules/CDBInterface.h>

#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllUtils.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllServer.h>

#include <phool/recoConsts.h>

#include <trackreco/PHActsTrackProjection.h>

#include <trackbase_historic/SvtxTrack.h>

#include <mbd/MbdReco.h>
#include <zdcinfo/ZdcReco.h>
#include <globalvertex/GlobalVertexReco.h>
#include <calovalid/CaloValid.h>
#include <caloreco/CaloTowerBuilder.h>
#include <caloreco/CaloTowerCalib.h>
#include <caloreco/CaloTowerStatus.h>
#include <caloreco/CaloWaveformProcessing.h>
#include <caloreco/DeadHotMapLoader.h>
#include <caloreco/RawClusterBuilderTemplate.h>
#include <caloreco/RawClusterDeadHotMask.h>
#include <caloreco/RawClusterPositionCorrection.h>
#include <caloreco/TowerInfoDeadHotMask.h>
#include </sphenix/u/pnietomar/install/include/fastreco/FastHadronReco.h>
#include <Calo_Calib.C>

#include <stdio.h>
#include <iostream>
#include <filesystem>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libphool.so)
R__LOAD_LIBRARY(libcdbobjects.so)
R__LOAD_LIBRARY(libtrack_reco.so)
R__LOAD_LIBRARY(libcalo_reco.so)
R__LOAD_LIBRARY(libcalotrigger.so)
R__LOAD_LIBRARY(libcentrality.so)
R__LOAD_LIBRARY(libmbd.so)
R__LOAD_LIBRARY(libepd.so)
R__LOAD_LIBRARY(libzdcinfo.so)
R__LOAD_LIBRARY(libTrackingDiagnostics.so)

void Fun4All_FastReco(
    const int runnumber = 53877,
    const int segment = 0,
    const int nEvents = 1,
    const char* caloFile = "",
    const char* trackingFiles = "",
    const char* clusterFiles = "",
    const std::string outfilename = "tracks_calos")
{


  TString outfileheader = outfilename + "_" + runnumber + "-" + segment;
  std::cout<<"outfile header: "<<outfileheader<<std::endl;
  std::string theOutfileheader = outfileheader.Data();


  Enable::MVTX_APPLYMISALIGNMENT = true;
  ACTSGEOM::mvtx_applymisalignment = Enable::MVTX_APPLYMISALIGNMENT;
  TRACKING::pp_mode = true;

  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  int verbosity = 0;

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", runnumber);
  rc->set_IntFlag("RUNSEGMENT", segment);

  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG", "newcdbtag");
  rc->set_uint64Flag("TIMESTAMP", runnumber);
  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");

  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);

  TpcReadoutInit( runnumber );
  std::cout << " run: " << runnumber
            << " samples: " << TRACKING::reco_tpc_maxtime_sample
            << " pre: " << TRACKING::reco_tpc_time_presample
            << " vdrift: " << G4TPC::tpc_drift_velocity_reco
            << std::endl;

  G4TPC::ENABLE_MODULE_EDGE_CORRECTIONS = true;

  G4TPC::ENABLE_STATIC_CORRECTIONS = true;
  G4TPC::USE_PHI_AS_RAD_STATIC_CORRECTIONS = false;

  G4TPC::ENABLE_AVERAGE_CORRECTIONS = true;
  G4TPC::USE_PHI_AS_RAD_AVERAGE_CORRECTIONS = false;

  G4TPC::average_correction_filename = CDBInterface::instance()->getUrl("TPC_LAMINATION_FIT_CORRECTION");

  G4MAGNET::magfield_rescale = 1;
  TrackingInit();

  Global_Reco();

  std::cout << "Adding Geometry file" << std::endl;
  Fun4AllInputManager *intrue2 = new Fun4AllRunNodeInputManager("DST_GEO");
  std::string geoLocation = CDBInterface::instance()->getUrl("calo_geo");
  intrue2->AddFile(geoLocation);
  se->registerInputManager(intrue2);

  Process_Calo_Calib();

  CaloValid *ca = new CaloValid("CaloValid");
  ca->set_timing_cut_width(200);
  se->registerSubsystem(ca);

  MbdReco *mbdreco = new MbdReco();
  se->registerSubsystem(mbdreco);

  ZdcReco *zdcreco = new ZdcReco();
  zdcreco->set_zdc1_cut(0.0);
  zdcreco->set_zdc2_cut(0.0);
  se->registerSubsystem(zdcreco);

  GlobalVertexReco *gvertex = new GlobalVertexReco();
  se->registerSubsystem(gvertex);

  auto projection = new PHActsTrackProjection("CaloProjection");
  float new_cemc_rad = 102.9; // from DetailedCalorimeterGeometry, project to inner surface
  bool doEMcalRadiusCorr = true;
  if (doEMcalRadiusCorr)
  {
    projection->setLayerRadius(SvtxTrack::CEMC, new_cemc_rad);
  }
  se->registerSubsystem(projection);

  TString ananame = theOutfileheader + "_ana.root";
  std::string anaOutputFileName(ananame.Data());

  /*
  sPHAnalysis *ana = new sPHAnalysis("sPHAnalysis",anaOutputFileName);
  ana->set_whattodo(1);
  ana->set_motherid(443);
  ana->set_runnumber(runnumber);
  ana->set_segment(segment);
  se->registerSubsystem(ana);
  se->Verbosity(0);
  */

  std::string faststring = outfilename + "_fastreco.root";
  auto fastreco = new FastHadronReco();
  fastreco->Verbosity(0);
  fastreco->setTrackQualityCut(1000);
  fastreco->setPtCut(0.2);
  fastreco->setPairDCACut(5.0);
  fastreco->setRequireMVTX(false);
  fastreco->setTrackDCACut(0.0);  // requires fabs(dca) > this
  fastreco->setDecayMass1(0.000511);    //(muons decaymass = 0.1057) (pions = 0.13957) (electron = 0.000511) (kaons = 0.49367)
  fastreco->setDecayMass2(0.000511);  //(muons decaymass = 0.1057) (pions = 0.13957) (electron = 0.000511) (kaons = 0.49367)
  fastreco->setRunNumber(runnumber);
  fastreco->setSegment(segment);
  fastreco->set_output_file(faststring);
  se->registerSubsystem(fastreco);

  std::string trackingFilename = std::string(trackingFiles);
  cout << "Loading TRACKING file: " << trackingFilename << endl;
  auto trackin = new Fun4AllDstInputManager("TrackInManager");
  trackin->fileopen(trackingFilename);
  se->registerInputManager(trackin);

  std::string clusterFilename = std::string(clusterFiles);
  cout << "Loading CLUSTER file: " << clusterFilename << endl;
  auto cluster = new Fun4AllDstInputManager("ClusterInManager");
  cluster->fileopen(clusterFilename);
  se->registerInputManager(cluster);

  std::string caloFilename = std::string(caloFile);
  cout << "Loading CALOFITTING file: " << caloFilename << endl;
  auto caloin = new Fun4AllDstInputManager("CaloInManager");
  caloin->AddListFile(caloFilename);
  se->registerInputManager(caloin);

  se->run(nEvents);
  se->End();
  se->PrintTimer();

  CDBInterface::instance()->Print();
  delete se;
  std::cout << "Finished" << std::endl;
  gSystem->Exit(0);

  return;
}


std::string GetFirstLine(std::string listname)
{
  std::ifstream file(listname);

  std::string firstLine = "";
  if (file.is_open()) {
      if (std::getline(file, firstLine)) {
          std::cout << "First Line: " << firstLine << std::endl;
      } else {
          std::cerr << "Unable to read first line of file" << std::endl;
      }
      file.close();
  } else {
      std::cerr << "Unable to open file" << std::endl;
  }
  return firstLine;
}
