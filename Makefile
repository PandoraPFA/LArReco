ifndef PANDORA_LARCONTENT_DIR
    PANDORA_LARCONTENT_DIR = $(PANDORA_DIR)/LArContent
endif

CC = g++
CFLAGS = -c -g -fPIC -O2 -Wall -Wextra -Werror -pedantic -Wno-long-long -Wno-sign-compare -Wshadow -fno-strict-aliasing -std=c++11
ifdef BUILD_32BIT_COMPATIBLE
    CFLAGS += -m32
endif

LIBS  = -L$(PANDORA_LARCONTENT_DIR)/lib -lLArContent
LIBS += -L$(PANDORA_DIR)/lib -lPandoraSDK
ifdef MONITORING
    LIBS += $(shell root-config --glibs --evelibs)
    LIBS += -lPandoraMonitoring
endif
ifdef BUILD_32BIT_COMPATIBLE
    LIBS += -m32
endif

PROJECT_BINARY = $(PROJECT_DIR)/bin/PandoraInterface

INCLUDES  = -I $(PROJECT_DIR)/include/
INCLUDES += -I $(PANDORA_DIR)/PandoraSDK/include/
INCLUDES += -I $(PANDORA_LARCONTENT_DIR)/
ifdef MONITORING
    INCLUDES += -I $(shell root-config --incdir)
    INCLUDES += -I $(PANDORA_DIR)/PandoraMonitoring/include/
endif

ifdef MONITORING
    DEFINES = -DMONITORING=1
endif

SOURCES =  $(wildcard $(PROJECT_DIR)/test/*.cxx)
OBJECTS = $(SOURCES:.cxx=.o)
DEPENDS = $(OBJECTS:.o=.d)

all: binary

binary: $(OBJECTS) 
	$(CC) $(OBJECTS) $(LIBS) -o $(PROJECT_BINARY)

-include $(DEPENDS)

%.o:%.cxx
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -MP -MMD -MT $*.o -MT $*.d -MF $*.d -o $*.o $*.cxx

clean:
	rm -f $(OBJECTS)
	rm -f $(DEPENDS)
	rm -f $(PROJECT_BINARY)
