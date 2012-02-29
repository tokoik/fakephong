CXXFLAGS	= -I/usr/X11R6/include -DX11 -Wall
LDLIBS	= -L/usr/X11R6/lib -lglut -lGLU -lGL -lm
OBJECTS	= $(subst .cpp,.o,$(wildcard *.cpp))
TARGET	= fakephong

$(TARGET): $(OBJECTS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	-rm -f $(TARGET) *.o *~ core

depend:
	$(CXX) $(CXXFLAGS) -MM *.cpp > $(TARGET).d

include $(wildcard *.d)
