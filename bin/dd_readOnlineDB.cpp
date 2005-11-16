#include "SealKernel/Exception.h"
#include "POOLCore/CommandOptions.h"

#include "SealBase/ShellEnvironment.h"

#include<iostream>
#include<istream>
#include<fstream>
#include<string>

#include "DetectorDescription/Parser/interface/DDLParser.h"
#include "DetectorDescription/Parser/interface/DDLConfiguration.h"

#include "DetectorDescription/DBReader/interface/DDRALReader.h"

#include "DetectorDescription/Core/interface/DDName.h"
#include "DetectorDescription/Core/interface/DDCompactView.h"
#include "DetectorDescription/Core/interface/DDExpandedView.h"

#include "DetectorDescription/Base/interface/DDException.h"

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>

namespace std { } using namespace std;

int  main( int argc, char** argv )
{
  std::vector<pool::CommandOptions::Option> ops;
  pool::CommandOptions::Option opt;
  opt.flag = "-s";
  opt.property = "specsConfig";
  opt.type = pool::CommandOptions::Option::STRING;
  opt.helpEntry = "the source XML configuration file name for SpecPar files.";
  opt.required = true;
  ops.push_back(opt);
  //   opt.flag = "-p";
  //   opt.property = "specParConfig";
  //   opt.type = pool::CommandOptions::Option::STRING;
  //   opt.helpEntry = "the specPar XML configuration file name";
  //   opt.required = true;
  //   ops.push_back(opt);
  opt.flag = "-c";
  opt.property = "connectionString";
  opt.type = pool::CommandOptions::Option::STRING;
  opt.helpEntry = "the connection string";
  opt.required = true;
  ops.push_back(opt);

  // NOT properly implemented.
  opt.flag = "-v";
  opt.property = "geometryVersion";
  opt.type = pool::CommandOptions::Option::STRING;
  opt.helpEntry = "version of online geometry being read";
  opt.required = false;
  ops.push_back(opt);

  opt.flag = "-a";
  opt.property = "authentication";
  opt.type = pool::CommandOptions::Option::STRING;
  opt.helpEntry = "authentication file needed by POOL for RAL access.";
  opt.required = true;
  ops.push_back(opt);

  opt.flag = "-r";
  opt.property = "rootNode";
  opt.type = pool::CommandOptions::Option::STRING;
  opt.helpEntry = "the root node of the DDCompactView you want to traverse";
  opt.required = true;
  ops.push_back(opt);

//   opt.flag = "-u";
//   opt.property = "user";
//   opt.type = pool::CommandOptions::Option::STRING;
//   opt.helpEntry = "the user name, required for DB servers (e.g. ORACLE)";
//   opt.required = false;
//   ops.push_back(opt);

//   opt.flag = "-p";
//   opt.property = "password";
//   opt.type = pool::CommandOptions::Option::STRING;
//   opt.helpEntry = "the user password, required for DB servers (e.g. ORACLE)";
//   opt.required = false;
//   ops.push_back(opt);

//   opt.flag = "-t";
//   opt.property = "geometryType";
//   opt.type = pool::CommandOptions::Option::STRING;
//   opt.helpEntry = "the type or table analogue, default: \"IdealGeometry\"";
//   opt.required = false;
//   ops.push_back(opt);
//   //not used right now.
//   opt.flag = "-debug";
//   opt.property = "debug";
//   opt.type = pool::CommandOptions::Option::BOOLEAN;
//   opt.required = false;
//   opt.helpEntry = "enable the verbose mode";
//   ops.push_back(opt);  
  // help (only) 
  opt.flag = "-h";
  opt.property = "help";
  opt.type = pool::CommandOptions::Option::BOOLEAN;
  opt.helpEntry = "display help";
  opt.required = false;  
  ops.push_back(opt);
  pool::CommandOptions cmd(ops);
  try{
    cmd.parse(argc,argv);
    const std::map<std::string,std::string>& ops = cmd.userOptions();
    std::map<std::string,std::string>::const_iterator iter = ops.find("help");
    if(iter!=ops.end() || ops.size()==0){
      cmd.help();
    } else {
    
      std::string theSpecsConfig;
      iter = ops.find("specsConfig");
      if(iter!=ops.end()){
        theSpecsConfig = (*iter).second;
      }      
      //       iter = ops.find("debug");
      //       if(iter!=ops.end()){

      //       }       
      //       iter = ops.find("file");
      //       if(iter==ops.end()){
      //         throw seal::Exception("Driver file not provided", "", seal::Status::error());
      //       } 
      //       std::string theDriverFile = (*iter).second;
      //       iter = ops.find("dictionary");
      //       if(iter!=ops.end()){
      // 	std::string theDictionaryLibraries = (*iter).second;

      //       } 
      std::string theConnectionString;
      iter = ops.find("connectionString");
      if(iter!=ops.end()){
	theConnectionString = (*iter).second;
      } 
      std::string theRootNode;
      iter = ops.find("rootNode");
      if(iter!=ops.end()){
	theRootNode = (*iter).second;
      } 
//       std::string theUserName;
//       iter = ops.find("user");
//       if(iter!=ops.end()){
// 	theUserName = (*iter).second;
//       } 
//       std::string thePassword;
//       iter = ops.find("password");
//       if(iter!=ops.end()){
//         thePassword = (*iter).second;
//       } 
      std::string theAuthentication;
      iter = ops.find("authentication");
      if(iter!=ops.end()){
        theAuthentication = (*iter).second;
      } 
      std::string theGeometryVersion;
      iter = ops.find("geometryVersion");
      if(iter!=ops.end()){
        theGeometryVersion = (*iter).second;
      } 
//       std::string theGeometryType;
//       iter = ops.find("geometryType");
//       if(iter!=ops.end()){
//         theGeometryType = (*iter).second;
//       } 

      DDName ddn(theRootNode);
      cout << "the root node ddname is " << ddn << endl;
      DDRALReader ddrr( ddn, theAuthentication, theConnectionString );
      bool result = ddrr.readDB();
      if ( !result ) {
	cout << "Failed to read the Geometry sources from DB using RAL" << endl;
      } else {
	DDLParser* myP = DDLParser::instance();
	DDLConfiguration dp;
	std::cout << "About to read configuration file: " << theSpecsConfig << std::endl; 
	int success = dp.readConfig(theSpecsConfig);
	if ( success != 0) {
	  throw DDException("Failed to read configuration.");
	}
	success = myP->parse(dp);
	if ( success != 0) {
	  throw DDException("Failed to parse whole geometry!");
	}
      }
      DDCompactView cpv;
      DDExpandedView epv(cpv);
      typedef DDExpandedView::nav_type nav_type;
      typedef map<nav_type,int> id_type;
      id_type idMap;
      int id=0;
      ofstream dump("dump");
      do {
	nav_type pos = epv.navPos();
	idMap[pos]=id;
	dump << id << " - " << epv.geoHistory() << endl;
	++id;    
      } while(epv.next());

      std::cout << "finished" << std::endl;
      return 0;
    }
  } catch (const DDException& de) { 
    std::cout << "ERROR: " << de.what() << endl;
  } catch (const seal::Exception& e){
    std::cout << "ERROR: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
