#Path to project directory
ifndef 
    PROJECT_DIR = YOUR_PATH_HERE
endif

#Paths to project dependencies
ifndef PANDORA_DIR
    PANDORA_DIR = YOUR_PATH_HERE
endif

ifndef PANDORA_LARCONTENT_DIR
    PANDORA_LARCONTENT_DIR = YOUR_PATH_HERE
endif

PROJECT_INCLUDE_DIR = $(PROJECT_DIR)/include/
PROJECT_SOURCE_DIR  = $(PROJECT_DIR)/src/
PROJECT_TEST_DIR = $(PROJECT_DIR)/test/
PROJECT_BINARY_DIR  = $(PROJECT_DIR)/bin/

INCLUDES  = -I $(PROJECT_INCLUDE_DIR) -I $(PROJECT_SOURCE_DIR)
INCLUDES += -I $(PROJECT_DIR)/Calculators/
INCLUDES += -I $(PANDORA_DIR)/PandoraSDK/include/
INCLUDES += -I $(PANDORA_LARCONTENT_DIR)/include/
INCLUDES += -I $(shell $(ROOTSYS)/bin/root-config --incdir)
INCLUDES += -I $(PANDORA_DIR)/PandoraMonitoring/include/

CC = g++
CFLAGS = -c -Wall -g -w -fPIC -O2 -DMONITORING=1
ifdef BUILD_32BIT_COMPATIBLE
    CFLAGS += -m32
endif

SOURCES  =  $(wildcard $(PROJECT_SOURCE_DIR)/*.cxx)
SOURCES +=  $(wildcard $(PROJECT_TEST_DIR)/*.cxx)
SOURCES +=  $(wildcard $(PROJECT_DIR)/Calculators/MicroBooNE*.cxx)

OBJECTS = $(SOURCES:.cxx=.o)
DEPENDS = $(OBJECTS:.o=.d)

LIBS  = -L$(PANDORA_LARCONTENT_DIR)/lib -lLArContent
LIBS += -L$(PANDORA_DIR)/lib -lPandoraSDK
LIBS += $(shell $(ROOTSYS)/bin/root-config --glibs)
LIBS += $(shell $(ROOTSYS)/bin/root-config --evelibs)
LIBS += -lPandoraMonitoring
ifdef BUILD_32BIT_COMPATIBLE
    LIBS += -m32
endif

LDFLAGS = $(LIBS) -Wl,-rpath

all: $(OBJECTS) PandoraInterface

PandoraInterface:
	@echo Creating binary: $(PROJECT_BINARY_DIR)/PandoraInterface
	$(CC) $(LIBS) $(OBJECTS) -o $(PROJECT_BINARY_DIR)/PandoraInterface
	@echo Created binary: $(PROJECT_BINARY_DIR)/PandoraInterface

-include $(DEPENDS)

%.o:%.cxx
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -MP -MMD -MT $*.o -MT $*.d -MF $*.d -o $*.o $*.cxx

clean:
	rm -f $(OBJECTS)
	rm -f $(DEPENDS)
	rm -f $(PROJECT_BINARY_DIR)/PandoraInterface
