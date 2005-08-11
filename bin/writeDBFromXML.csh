#         dd_xml2db [option]
#                 -s ( --sourceConfig ) : the source XML configuration file name
#                 -c ( --connectionString ) : the connection string
#                 -u ( --user ) : the user name
#                 -p ( --password ) : the user password
#                 -n ( --geometryName ) : name of Geometry (version) in DB
#                 -debug ( --debug ) : enable the verbose mode
#                 -h ( --help ) : display help
ln -s /afs/cern.ch/cms/Releases/Geometry/Geometry_1_16_1/src/* .
ln -s /afs/cern.ch/cms/Releases/ORCA/ORCA_8_10_1/src/Data/* .
ln -s /afs/cern.ch/cms/Releases/OSCAR/OSCAR_3_9_5/src/Data/* .
dd_xml2db -s ORCAconfiguration.xml -c sqlite_file:IdealGeometry.db -n IdealGeometry01 -t IdealGeometry

