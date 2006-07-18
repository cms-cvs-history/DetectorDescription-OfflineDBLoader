#include "DetectorDescription/OfflineDBLoader/interface/ReadWriteORA.h"

#include "DataSvc/RefException.h"

#include "CondCore/DBCommon/interface/DBSession.h"
#include "CondCore/DBCommon/interface/Exception.h"
#include "CondCore/DBCommon/interface/ServiceLoader.h"
#include "CondCore/DBCommon/interface/ConnectMode.h"
#include "CondCore/DBCommon/interface/MessageLevel.h"
#include "CondCore/MetaDataService/interface/MetaData.h"
#include "CondCore/DBCommon/interface/DBWriter.h"
#include "CondCore/IOVService/interface/IOV.h"

#include "FWCore/Framework/interface/IOVSyncValue.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DetectorDescription/Parser/interface/DDLParser.h"
#include "DetectorDescription/Parser/interface/FIPConfiguration.h"
#include "DetectorDescription/PersistentDDDObjects/interface/PersistentDDDObjects.h"
#include "DetectorDescription/DBReader/interface/DDORAReader.h"
#include "DetectorDescription/PersistentDDDObjects/interface/DDDToPersFactory.h"
#include "DetectorDescription/Core/interface/DDCompactView.h"
#include "DetectorDescription/Core/interface/DDRoot.h"
#include "DetectorDescription/Core/interface/DDSolid.h"
#include "DetectorDescription/Core/interface/DDSpecifics.h"
#include "DetectorDescription/Core/interface/DDsvalues.h"
#include "DetectorDescription/Core/interface/DDPartSelection.h"
#include "DetectorDescription/Core/interface/DDTransform.h"

#include "POOLCore/POOLContext.h"
#include "CoralBase/Exception.h"

#include<string>

using cond::MetaData;

ReadWriteORA::ReadWriteORA ( const std::string& dbConnectString
			     , const std::string& metaName 
			     , const std::string& userName
			     , const std::string& password )
  : dbConnectString_(dbConnectString)
  , metaName_(metaName)
  , userName_(userName)
  , password_(password)
{ 
  //FIXME: check DB and xml for existence, nothing more.
}

ReadWriteORA::~ReadWriteORA () { }

/// Write DB from compactview
bool ReadWriteORA::writeDB ( const DDCompactView & cpv ) {

  std::string token;
  PIdealGeometry* pgeom = new PIdealGeometry;

  try {
    
    DDCompactView::graph_type gra = cpv.graph();
    
    DDMaterial::iterator<DDMaterial> it(DDMaterial::begin()), ed(DDMaterial::end());
    PMaterial* pm;
    for (; it != ed; ++it) {
      if (! it->isDefined().second) continue;
      pm = DDDToPersFactory::material ( *it );
      pgeom->pMaterials.push_back ( *pm );
      delete pm;
    }

    DDRotation::iterator<DDRotation> rit(DDRotation::begin()), red(DDRotation::end());
    PRotation* pr;
    for (; rit != red; ++rit) {
      if (! rit->isDefined().second) continue;  
      pr = DDDToPersFactory::rotation( *rit );
      pgeom->pRotations.push_back ( *pr );
    } 

    DDSolid::iterator<DDSolid> sit(DDSolid::begin()), sed(DDSolid::end());
    PSolid* ps;
    for (; sit != sed; ++sit) {
      if (! sit->isDefined().second) continue;  
      ps = DDDToPersFactory::solid( *sit );
      pgeom->pSolids.push_back( *ps );
      delete ps;
    }

    // Discovered during validation:  If a user declares
    // a LogicalPart in the DDD XML and does not position it
    // in the graph, it will NOT be stored to the database
    // subsequently SpecPars (see below) that use wildcards
    // that may have selected the orphaned LogicalPart node
    // will be read into the DDD from the DB (not by this
    // code, but in the system) and so DDSpecifics will
    // throw a DDException.
    typedef graph_type::const_adj_iterator adjl_iterator;
    adjl_iterator git = gra.begin();
    adjl_iterator gend = gra.end();    
    
    graph_type::index_type i=0;
    PLogicalPart* plp;
    for (; git != gend; ++git) 
      {
	const DDLogicalPart & ddLP = gra.nodeData(git);
	//	std::cout << ddLP << std::endl;
	plp = DDDToPersFactory::logicalPart ( ddLP  );
	pgeom->pLogicalParts.push_back( *plp );
	delete plp;
	++i;
	if (git->size()) 
	  {
	    // ask for children of ddLP  
	    graph_type::edge_list::const_iterator cit  = git->begin();
	    graph_type::edge_list::const_iterator cend = git->end();
	    PPosPart* ppp;
	    for (; cit != cend; ++cit) 
	      {
		const DDLogicalPart & ddcurLP = gra.nodeData(cit->first);
		ppp = DDDToPersFactory::position ( ddLP, ddcurLP, gra.edgeData(cit->second), *pgeom );
		pgeom->pPosParts.push_back( *ppp );
		delete ppp;
	      } // iterate over children
	  } // if (children)
      } // iterate over graph nodes  
  
    std::vector<std::string> partSelections;
    std::map<std::string, std::vector<std::pair<std::string, double> > > values;
    std::map<std::string, int> isEvaluated;
    PSpecPar* psp;

    DDSpecifics::iterator<DDSpecifics> spit(DDSpecifics::begin()), spend(DDSpecifics::end());

    // ======= For each DDSpecific...
    for (; spit != spend; ++spit) {
      if ( !spit->isDefined().second ) continue;  
      const DDSpecifics & sp = *spit;

      std::vector<DDPartSelection>::const_iterator sit(sp.selection().begin()), sed(sp.selection().end());

      // ========... copy all the selection strings out as strings by using the DDPartSelection's ostream function...
      for (; sit != sed; ++sit) {
	std::ostringstream selStringStream;
	selStringStream << *sit;
	partSelections.push_back ( selStringStream.str() );
      }
      // =========  ... and iterate over all DDValues...
      DDsvalues_type::const_iterator vit(sp.specifics().begin()), ved(sp.specifics().end());
      for (; vit != ved; ++vit) {
	const DDValue & v = vit->second;
	std::vector<std::pair<std::string, double> > vpvp;
	if ( v.isEvaluated() ) {
	  size_t s=v.size();
	  size_t i=0;
	  // ============  ... and copy all actual values with the same name
	  for (; i<s; ++i) {
	    vpvp.push_back(v[i]);
	  }
	  isEvaluated[v.name()] = 1;
	  //	  std::cout << sp.toString() << " variable name " << v.name() << " set evaluated to 1 (true) " << std::endl;
	}
	else {
	  size_t s=v.size();
	  size_t i=0;
	  const std::vector<std::string> & vs = v.strings();
	  // ============  ... and copy all actual values with the same name
	  for (; i<s; ++i) {
	    vpvp.push_back(make_pair(vs[i], 0.0));
	  }
	  isEvaluated[v.name()] = 0;
	  //	  std::cout << sp.toString() << " variable name " << v.name() << " set evaluated to 0 (false) " << std::endl;
	}
	values[v.name()] = vpvp;
      }
      psp = DDDToPersFactory::specpar( spit->toString()
				       , partSelections
				       , values
				       , isEvaluated );

      pgeom->pSpecPars.push_back( *psp );
      values.clear();
      partSelections.clear();
      isEvaluated.clear();
      delete psp;
    } 

    pgeom->pStartNode = DDRootDef::instance().root().toString();
    pool::POOLContext::loadComponent( "SEAL/Services/MessageService" );
    pool::POOLContext::setMessageVerbosityLevel( seal::Msg::Error );
    cond::ServiceLoader* loader=new cond::ServiceLoader;
    std::string usr = "CORAL_AUTH_USER="+userName_;
    std::string pass = "CORAL_AUTH_PASSWORD="+password_;
    ::putenv(usr.c_str());
    ::putenv(pass.c_str());
//     std::cout << ::getenv("CORAL_AUTH_USER") << std::endl;
//     std::cout << ::getenv("CORAL_AUTH_PASSWORD") << std::endl;
    loader->loadAuthenticationService( cond::Env );
    loader->loadMessageService( cond::Error );
    cond::DBSession* session=new cond::DBSession(dbConnectString_);
    session->setCatalog("file:PoolFileCatalog.xml");
    session->connect(cond::ReadWriteCreate);
    cond::DBWriter pw(*session, "PIdealGeometry");
    cond::DBWriter iovw(*session, "IOV");
    cond::IOV* initiov=new cond::IOV;
    session->startUpdateTransaction();
    std::string tok=pw.markWrite<PIdealGeometry>(pgeom);
    unsigned long long myTime=(unsigned long long)edm::IOVSyncValue::endOfTime().eventID().run();
    //    std::cout << "The end-of-time is " << myTime << std::endl;
    initiov->iov.insert(std::make_pair(myTime,tok));
    std::string iovtok = iovw.markWrite<cond::IOV>(initiov);
    session->commit();
    session->disconnect();
    delete session;

    cond::MetaData metadata_svc(dbConnectString_, *loader);
    metadata_svc.connect(cond::ReadWriteCreate);
    metadata_svc.addMapping(metaName_, iovtok);

    metadata_svc.disconnect();
    edm::LogInfo ("DDDReadWriteORA") << "Done with save, token " << tok << " as metaName " << metaName_  << std::endl;

    delete loader;

  } catch (DDException& e) {
    std::cout << "ReadWriteORA::writeDB caught DDException: " << e.what() << std::endl;
    return false;
  } catch (const coral::Exception& e) {
    std::cout << "ReadWriteORA::writeDB caught coral::Exception: " << e.what() << std::endl;
    return false;
  } catch( const pool::RefException& er){
    std::cerr<<"ReadWriteORA::writeDB caught pool::RefException "<<er.what()<<std::endl;
    return false;
  } catch ( pool::Exception& e ) {
    std::cout << "ReadWriteORA::writeDB caught pool::Exception -> " << e.what() << std::endl;
    return false;
  } catch ( std::exception& e ) {
    std::cout << "ReadWriteORA::writeDB caught std::exception -> " << e.what() << std::endl;
    return false;
  } catch ( ... ) {
    std::cout << "ReadWriteORA::writeDB caught ( ... ) , i.e. unknown, exception. " << std::endl;
    return false;
  }

  return true;
}
