OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

include ../Mk/local.Mk
CXXINCLUDE+=-I./ -I../common -I../control 

all: $(OBJS)

.cpp.o:
	$(CXX)  $(CXXFLAGS) $(CXXDEFINE) -c  $(CXXINCLUDE) $< 

.c.o:
	$(CXX)  $(FLAGS) $(CXXDEFINE) -c  $(CXXFLAGS) $(CXXINCLUDE) $< 

clean:
	$(RM) $(OBJS)
