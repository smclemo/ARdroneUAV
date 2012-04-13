OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

include ../Mk/local.Mk
CXXINCLUDE+=-I./  -I../common -I../camera -I../map -I/usr/local/include
CXXFLAGS+=-Wall

all: $(OBJS)

.cpp.o:
	$(CXX)  $(CXXFLAGS) $(CXXDEFINE) -c  $(CXXINCLUDE) $< 

.c.o:
	$(CXX)  $(FLAGS) $(CXXDEFINE) -c  $(CXXFLAGS) $(CXXINCLUDE) $< 

clean:
	$(RM) $(OBJS)
