include ../Mk/local.Mk
OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJS+=$(patsubst %.c,%.o,$(wildcard *.c))

CXXINCLUDE+=-I./ -I../common/ 

all: $(OBJS)

.cpp.o:
	$(CXX)  $(CXXFLAGS) $(CXXDEFINE) -c  $(CXXINCLUDE) $< 

.c.o:
	$(CXX)  $(FLAGS) $(CXXDEFINE) -c  $(CXXFLAGS) $(CXXINCLUDE) $< 

clean:
	$(RM) $(OBJS)
