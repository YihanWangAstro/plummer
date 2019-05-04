
all : plummer

CXX=g++
CXXFLAGS=-std=c++17 -O3 -Wall 
OBJS = plummer.o 
INCL = config-reader.h

plummer : $(OBJS)
	$(CXX) -std=c++17 -O3 -o  plummer $(OBJS) 
	

$(OBJS) : $(INCL)

.PHONY : clean	
clean:
	-rm $(OBJS) 
