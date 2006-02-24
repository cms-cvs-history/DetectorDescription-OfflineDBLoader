#         dd_xml2db [option]
#                 -s ( --sourceConfig ) : the source XML configuration file name
#                 -c ( --connectionString ) : the connection string
#                 -u ( --user ) : the user name
#                 -p ( --password ) : the user password
#                 -n ( --geometryName ) : name of Geometry (version) in DB
#                 -debug ( --debug ) : enable the verbose mode
#                 -h ( --help ) : display help
dd_xml2db -s Geometry/CMSCommonData/data/configuration.xml -c sqlite_file:IdealGeometry.db -n IdealGeometry01 -t IdealGeometry

