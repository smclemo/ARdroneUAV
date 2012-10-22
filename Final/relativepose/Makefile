#Winston's generic makefile
#Builds objects in the same subfolders as the source

program_NAME := relativepose

SUB_DIRS = .

C_SRCS := $(foreach sdir,$(SUB_DIRS),$(wildcard $(sdir)/*.c))
CXX_SRCS := $(foreach sdir,$(SUB_DIRS),$(wildcard $(sdir)/*.cc)) \
	$(foreach sdir,$(SUB_DIRS),$(wildcard $(sdir)/*.cpp))
C_OBJS := $(addsuffix .o, $(basename $(C_SRCS)))
CXX_OBJS := $(addsuffix .o, $(basename $(CXX_SRCS)))
OBJS = $(C_OBJS) $(CXX_OBJS)
BUILD_DIRS = build

INCLUDE_DIRS += 
LIB_DIRS = 
LIBS = cvd lapack GLU

CPPFLAGS += $(foreach includedir, $(INCLUDE_DIRS), -I$(INCLUDE_DIRS))
CPPFLAGS += -Wall -fopenmp
OFLAGS := -O3
LDFLAGS += $(foreach librarydir, $(LIB_DIRS), -L$(librarydir))
LDFLAGS += $(foreach library, $(LIBS), -l$(library))

%.o: %.c
	$(CC) -c $(OFLAGS) $(CPPFLAGS) $< -o $@

%.o : %.cpp
	$(CXX) -c $(OFLAGS) $(CPPFLAGS) $< -o $@ 

%.o : %.cc
	$(CXX) -c $(OFLAGS) $(CPPFLAGS) $< -o $@

.PHONY: all clean distclean

all:$(program_NAME)
	
$(program_NAME): $(OBJS)
	$(CXX) $(CPPFLAGS) $(OFLAGS) -o $(program_NAME) $(OBJS) $(LDFLAGS)

#-----Other stuff----------------------------

depend:
	makedepend $(CXXFLAGS) -Y $(CXX_SRCS)

clean:
	rm -f $(OBJS) $(program_NAME)
	
distclean: clean

# DO NOT DELETE

#object file dependencies (for templated stuff in headers)
#functions.o: functions.h

