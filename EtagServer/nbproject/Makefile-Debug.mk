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
	${OBJECTDIR}/common.o \
	${OBJECTDIR}/config.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/moxa_setting.o \
	${OBJECTDIR}/msg.o \
	${OBJECTDIR}/net_handle.o \
	${OBJECTDIR}/reader1000api.o


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
LDLIBSOPTIONS=-lpthread -lxml2

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk /root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/etagserver

/root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/etagserver: ${OBJECTFILES}
	${MKDIR} -p /root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o /root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/etagserver ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/common.o: common.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Ilib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/common.o common.c

${OBJECTDIR}/config.o: config.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Ilib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/config.o config.c

${OBJECTDIR}/main.o: main.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Ilib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/moxa_setting.o: moxa_setting.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Ilib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/moxa_setting.o moxa_setting.c

${OBJECTDIR}/msg.o: msg.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Ilib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/msg.o msg.c

${OBJECTDIR}/net_handle.o: net_handle.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Ilib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/net_handle.o net_handle.c

${OBJECTDIR}/reader1000api.o: reader1000api.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Ilib -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/reader1000api.o reader1000api.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} /root/jiezisoft/bin/${CND_CONF}/${CND_PLATFORM}/etagserver

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
