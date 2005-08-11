#include "SealKernel/Exception.h"
#include "POOLCore/CommandOptions.h"

#include "SealBase/ShellEnvironment.h"

#include<iostream>
#include<istream>
#include<fstream>
#include<string>




#include "DetectorDescription/OfflineDBLoader/interface/ReadWriteORA.h"
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
  opt.property = "sourceConfig";
  opt.type = pool::CommandOptions::Option::STRING;
  opt.helpEntry = "the source XML configuration file name";
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
  opt.flag = "-n";
  opt.property = "geometryName";
  opt.type = pool::CommandOptions::Option::STRING;
  opt.helpEntry = "name of IdealGeometry being stored";
  opt.required = true;
  ops.push_back(opt);
  opt.flag = "-u";
  opt.property = "user";
  opt.type = pool::CommandOptions::Option::STRING;
  opt.helpEntry = "the user name, required for DB servers (e.g. ORACLE)";
  opt.required = false;
  ops.push_back(opt);
  opt.flag = "-p";
  opt.property = "password";
  opt.type = pool::CommandOptions::Option::STRING;
  opt.helpEntry = "the user password, required for DB servers (e.g. ORACLE)";
  opt.required = false;
  ops.push_back(opt);
  opt.flag = "-t";
  opt.property = "geometryType";
  opt.type = pool::CommandOptions::Option::STRING;
  opt.helpEntry = "the type or table analogue, default: \"IdealGeometry\"";
  opt.required = false;
  ops.push_back(opt);
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
    
      string theSourceConfig;
      iter = ops.find("sourceConfig");
      if(iter!=ops.end()){
        theSourceConfig = (*iter).second;
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
      std::string theUserName;
      iter = ops.find("user");
      if(iter!=ops.end()){
	theUserName = (*iter).second;
      } 
      std::string thePassword;
      iter = ops.find("password");
      if(iter!=ops.end()){
        thePassword = (*iter).second;
      } 
      std::string theGeometryName;
      iter = ops.find("geometryName");
      if(iter!=ops.end()){
        theGeometryName = (*iter).second;
      } 
      std::string theGeometryType;
      iter = ops.find("geometryType");
      if(iter!=ops.end()){
        theGeometryType = (*iter).second;
      } 

      //       if(onlyPrint) {
      //         utility.printSetUpStructure(theDriverFile);
      //       } else {
      //         utility.setUpFromExistingData(theDriverFile);
      // 	std::cout << "DONE!..."<<std::endl;
      //       }

      seal::ShellEnvironment senv;
      senv.set( "POOL_AUTH_USER", theUserName );
      senv.set( "POOL_AUTH_PASSWORD", thePassword );
      if ( senv.has("DDWriteConnectString") ) {
	theConnectionString = senv.get("DDWriteConnectString");
	cout << "Environment variable DDWriteConnectString was used for connection. ";
      }
      cout << "Connection String is: "  << theConnectionString << endl;
      
      ReadWriteORA rwo( theConnectionString
			, theSourceConfig
			, theGeometryName
			, theGeometryType
			, theUserName
			, thePassword );
      bool result = rwo.readFromXML();

      if ( !result ) {
	cout << "Failed to read XML Geometry sources from configuration.xml" << endl;
      } else {
	result = rwo.writeDB();
	if ( !result ) {
	  cout << "Failed to write DB." << endl;
	}
      }
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
