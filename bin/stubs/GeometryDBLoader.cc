#include <memory>

#include <FWCore/Framework/interface/Frameworkfwd.h>
#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/Framework/interface/MakerMacros.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>

#include <DetectorDescription/Core/interface/DDCompactView.h>
#include <DetectorDescription/Core/interface/DDValue.h>
#include <DetectorDescription/Core/interface/DDsvalues.h>
#include <DetectorDescription/Core/interface/DDExpandedView.h>
#include <DetectorDescription/Core/interface/DDFilteredView.h>
#include <DetectorDescription/Core/interface/DDSpecifics.h>
#include "DetectorDescription/Core/interface/DDName.h"
#include "DetectorDescription/Core/interface/DDCompactView.h"
#include "DetectorDescription/Core/interface/DDExpandedView.h"
#include "DetectorDescription/OfflineDBLoader/interface/ReadWriteORA.h"
#include "DetectorDescription/OfflineDBLoader/interface/GeometryInfoDump.h"

#include <Geometry/Records/interface/IdealGeometryRecord.h>
#include <MagneticField/Records/interface/IdealMagneticFieldRecord.h>

#include <iostream>
#include <istream>
#include <fstream>
#include <string>


class GeometryDBLoader : public edm::EDAnalyzer {

public:
 
  explicit GeometryDBLoader( const edm::ParameterSet& );
  ~GeometryDBLoader();
  
  virtual void analyze( const edm::Event&, const edm::EventSetup& );
  virtual void beginJob( const edm::EventSetup& );
  
  const std::string& myName() { return myName_;}
  
private: 
  
  const int dashedLineWidth_;
  const std::string dashedLine_;
  const std::string myName_;
  std::string label_;
  std::string user_;
  std::string pass_;
  std::string conn_;
  std::string meta_;
  std::string recType_;
  bool dumpHistory_;
  bool dumpSpecs_;
  bool dumpPosInfo_;
  int rotNumSeed_;
};

GeometryDBLoader::GeometryDBLoader( const edm::ParameterSet& iConfig )
 : dashedLineWidth_(194), dashedLine_( std::string(dashedLineWidth_, '-') ), 
  myName_( "GeometryDBLoader" )
{
  user_=iConfig.getParameter<std::string>("userName");
  pass_=iConfig.getParameter<std::string>("password");
  conn_=iConfig.getParameter<std::string>("connection");
  meta_=iConfig.getParameter<std::string>("metaName");
  recType_=iConfig.getParameter<std::string>("recType");
  rotNumSeed_=iConfig.getParameter<int>("rotationNumberingSeed");
  dumpHistory_=iConfig.getUntrackedParameter<bool>("dumpGeoHistory");
  dumpSpecs_=iConfig.getUntrackedParameter<bool>("dumpSpecs");
  dumpPosInfo_=iConfig.getUntrackedParameter<bool>("dumpPosInfo");
}


GeometryDBLoader::~GeometryDBLoader()
{
}

void GeometryDBLoader::analyze( const edm::Event& iEvent, const edm::EventSetup& iSetup ) { 
  std::cout << "analyze does nothing" << std::endl;
}

void GeometryDBLoader::beginJob( const edm::EventSetup& iSetup ) {

  std::cout << myName() << ": Analyzer..." << std::endl;
  std::cout << "start " << dashedLine_ << std::endl;

  edm::ESHandle<DDCompactView> pDD;

  if ( recType_ == "geometry" ) {
    iSetup.get<IdealGeometryRecord>().get( label_, pDD );
  } else if ( recType_ == "magfield" ) {
    iSetup.get<IdealMagneticFieldRecord>().get(recType_, pDD );
  }

  ::setenv( "POOL_AUTH_USER", user_.c_str(), 1 );
  ::setenv( "POOL_AUTH_PASSWORD", pass_.c_str(), 1 );
  std::cout << "Connection String is: "  << conn_ << std::endl;
  
  ReadWriteORA rwo( conn_
		    , meta_
		    , user_
		    , pass_ 
		    , rotNumSeed_);
  bool result = rwo.writeDB( *pDD );
  if ( !result ) {
    std::cout << "Failed to write DB." << std::endl;
  }
  
  GeometryInfoDump gidump;
  gidump.dumpInfo( dumpHistory_, dumpSpecs_, dumpPosInfo_, *pDD );

  std::cout << dashedLine_ << " end" << std::endl;
}

//define this as a plug-in
DEFINE_FWK_MODULE(GeometryDBLoader);
