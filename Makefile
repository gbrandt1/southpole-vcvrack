SLUG = Southpole
VERSION = 0.5.1

# FLAGS will be passed to both the C and C++ compiler
FLAGS += \
	-DTEST -DPARASITES \
	-I./parasites \
	-Wno-unused-local-typedefs

CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
# SOURCES += $(wildcard src/*.cpp)
SOURCES += src/Southpole.cpp
SOURCES += src/Balaclava.cpp	
SOURCES += src/Bandana.cpp
SOURCES += src/But.cpp
SOURCES += src/Abr.cpp

SOURCES += src/Etagere.cpp
SOURCES += src/VAStateVariableFilter.cpp
SOURCES += src/DSPUtilities.cpp

#SOURCES += src/Sns.cpp
SOURCES += src/Bounce.cpp

SOURCES += src/Annuli.cpp
SOURCES += parasites/rings/dsp/fm_voice.cc
SOURCES += parasites/rings/dsp/string_synth_part.cc
SOURCES += parasites/rings/dsp/string.cc
SOURCES += parasites/rings/dsp/resonator.cc
SOURCES += parasites/rings/resources.cc

SOURCES += src/Smoke.cpp
SOURCES += parasites/clouds/dsp/correlator.cc
SOURCES += parasites/clouds/dsp/granular_processor.cc
SOURCES += parasites/clouds/dsp/mu_law.cc
SOURCES += parasites/clouds/dsp/pvoc/frame_transformation.cc
SOURCES += parasites/clouds/dsp/pvoc/phase_vocoder.cc
SOURCES += parasites/clouds/dsp/pvoc/stft.cc
SOURCES += parasites/clouds/resources.cc 
#SOURCES += parasites/tides/generator.cc 
#SOURCES += parasites/tides/resources.cc 

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

# Include the VCV plugin Makefile framework
include ../../plugin.mk