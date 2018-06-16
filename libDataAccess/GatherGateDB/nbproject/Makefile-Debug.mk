#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/262068057/cppmysql.o \
	${OBJECTDIR}/cbasicdataaccess.o \
	${OBJECTDIR}/tinyXML/tinystr.o \
	${OBJECTDIR}/tinyXML/tinyxml.o \
	${OBJECTDIR}/tinyXML/tinyxmlerror.o \
	${OBJECTDIR}/tinyXML/tinyxmlparser.o \
	${OBJECTDIR}/tinyXML/xml_config.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=/usr/lib/mysql/libmysqlclient_r.so

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libGatherGateDB.${CND_DLIB_EXT}

/root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libGatherGateDB.${CND_DLIB_EXT}: /usr/lib/mysql/libmysqlclient_r.so

/root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libGatherGateDB.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libGatherGateDB.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/_ext/262068057/cppmysql.o: ../include/cppmysql.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/262068057
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/mysql -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/262068057/cppmysql.o ../include/cppmysql.cpp

${OBJECTDIR}/cbasicdataaccess.o: cbasicdataaccess.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/mysql -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cbasicdataaccess.o cbasicdataaccess.cpp

${OBJECTDIR}/tinyXML/tinystr.o: tinyXML/tinystr.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/mysql -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinystr.o tinyXML/tinystr.cpp

${OBJECTDIR}/tinyXML/tinyxml.o: tinyXML/tinyxml.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/mysql -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinyxml.o tinyXML/tinyxml.cpp

${OBJECTDIR}/tinyXML/tinyxmlerror.o: tinyXML/tinyxmlerror.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/mysql -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinyxmlerror.o tinyXML/tinyxmlerror.cpp

${OBJECTDIR}/tinyXML/tinyxmlparser.o: tinyXML/tinyxmlparser.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/mysql -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/tinyxmlparser.o tinyXML/tinyxmlparser.cpp

${OBJECTDIR}/tinyXML/xml_config.o: tinyXML/xml_config.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyXML
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/mysql -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyXML/xml_config.o tinyXML/xml_config.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libGatherGateDB.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
