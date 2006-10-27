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

#include <Geometry/Records/interface/IdealGeometryRecord.h>

#include <iostream>
#include <istream>
#include <fstream>
#include <string>

#include <cmath>
#include <iomanip>
#include <vector>
#include <map>
#include <sstream>

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
  bool dumpHistory_;
  bool dumpSpecs_;
};

GeometryDBLoader::GeometryDBLoader( const edm::ParameterSet& iConfig )
 : dashedLineWidth_(194), dashedLine_( std::string(dashedLineWidth_, '-') ), 
  myName_( "GeometryDBLoader" )
{
  user_=iConfig.getParameter<std::string>("userName");
  pass_=iConfig.getParameter<std::string>("password");
  conn_=iConfig.getParameter<std::string>("connection");
  meta_=iConfig.getParameter<std::string>("metaName");
  dumpHistory_=iConfig.getUntrackedParameter<bool>("dumpGeoHistory");
  dumpSpecs_=iConfig.getUntrackedParameter<bool>("dumpSpecs");
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
  iSetup.get<IdealGeometryRecord>().get(label_, pDD );

  try {
    ::setenv( "POOL_AUTH_USER", user_.c_str(), 1 );
    ::setenv( "POOL_AUTH_PASSWORD", pass_.c_str(), 1 );
    std::cout << "Connection String is: "  << conn_ << std::endl;
   
    ReadWriteORA rwo( conn_
		      , meta_
		      , user_
		      , pass_ );
    bool result = rwo.writeDB( *pDD );
    if ( !result ) {
      std::cout << "Failed to write DB." << std::endl;
    }
    if ( dumpHistory_ ) {
      const DDCompactView cpv = *pDD;
      DDExpandedView epv(cpv);
      typedef DDExpandedView::nav_type nav_type;
      typedef std::map<nav_type,int> id_type;
      id_type idMap;
      int id=0;
      std::ofstream dump("dumpGeoHistoryOnWrite");
      do {
	nav_type pos = epv.navPos();
	idMap[pos]=id;
	dump << id << " - " << epv.geoHistory() << std::endl;
	++id;    
      } while(epv.next()); 
    }
    if ( dumpSpecs_ ) {
      DDSpecifics::iterator<DDSpecifics> spit(DDSpecifics::begin()), spend(DDSpecifics::end());
      // ======= For each DDSpecific...
      std::ofstream dump("dumpSpecsOnWrite");
      for (; spit != spend; ++spit) {
	if ( !spit->isDefined().second ) continue;  
	const DDSpecifics & sp = *spit;
	dump << sp << std::endl;
      }
      dump.close();
    }
    std::cout << "finished" << std::endl;

  } catch (const DDException& de) { 
    std::cout << "ERROR: " << de.what() << std::endl;
  } catch (const std::exception& e){
    std::cout << "ERROR: " << e.what() << std::endl;
  }  
  std::cout << dashedLine_ << " end" << std::endl;
}

//define this as a plug-in
DEFINE_FWK_MODULE(GeometryDBLoader);
