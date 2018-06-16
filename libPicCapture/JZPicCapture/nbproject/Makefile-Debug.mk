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
	${OBJECTDIR}/CJZCapture.o \
	${OBJECTDIR}/MutexGuard.o \
	${OBJECTDIR}/SysMutex.o


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
LDLIBSOPTIONS=/usr/lib/libpthread.so ../../../tools/NetCamera_SDK_v2.3.6_SDK/linux/x32/libNetCamera.so ../../../tools/NetCamera_SDK_v2.3.6_SDK/linux/x32/libNetCamera_SnapServer.so

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libJZPicCapture.${CND_DLIB_EXT}

/root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libJZPicCapture.${CND_DLIB_EXT}: /usr/lib/libpthread.so

/root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libJZPicCapture.${CND_DLIB_EXT}: ../../../tools/NetCamera_SDK_v2.3.6_SDK/linux/x32/libNetCamera.so

/root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libJZPicCapture.${CND_DLIB_EXT}: ../../../tools/NetCamera_SDK_v2.3.6_SDK/linux/x32/libNetCamera_SnapServer.so

/root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libJZPicCapture.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libJZPicCapture.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/CJZCapture.o: CJZCapture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CJZCapture.o CJZCapture.cpp

${OBJECTDIR}/MutexGuard.o: MutexGuard.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MutexGuard.o MutexGuard.cpp

${OBJECTDIR}/SysMutex.o: SysMutex.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysMutex.o SysMutex.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} /root/jiezisoft/libs/${CND_CONF}/${CND_PLATFORM}/libJZPicCapture.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
