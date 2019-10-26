
FLAGS += \
	-DTEST \
	-I./parasites \
	-Wno-unused-local-typedefs

SOURCES += src/Southpole.cpp
SOURCES += parasites/stmlib/utils/random.cc
SOURCES += parasites/stmlib/dsp/units.cc
SOURCES += parasites/stmlib/dsp/atan.cc

SOURCES += src/Smoke.cpp
SOURCES += parasites/clouds/dsp/correlator.cc
SOURCES += parasites/clouds/dsp/granular_processor.cc
SOURCES += parasites/clouds/dsp/mu_law.cc
SOURCES += parasites/clouds/dsp/pvoc/frame_transformation.cc
SOURCES += parasites/clouds/dsp/pvoc/phase_vocoder.cc
SOURCES += parasites/clouds/dsp/pvoc/stft.cc
SOURCES += parasites/clouds/resources.cc

SOURCES += src/Splash.cpp
SOURCES += parasites/tides/generator.cc
SOURCES += parasites/tides/resources.cc

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk
