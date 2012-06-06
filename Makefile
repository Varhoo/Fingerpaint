# makefile
# author: Pavel Studeník

GXX=g++
LIBS= -lm #-lX11 -lXi -lXmu -lglut -lGL -lGLU
CFLAGS= -Wall -g -O3
CFLAGS+= `pkg-config gtk+-2.0 --cflags` # gtkglext-1.0 gtkglext-x11-1.0`
LIBS+= `pkg-config --cflags --libs gtk+-2.0 gmodule-export-2.0 --libs gthread-2.0 gtkglext-1.0 gtkglext-x11-1.0`
LIBS+= `pkg-config libfreenect --libs --cflags` -lpthread -lm -lusb-1.0 -lfreenect -Wunknown-pragmas

CVCFLAGS  += $(CFLAGS) `pkg-config opencv  --cflags`
CVLIBS += $(LIBS) -I/usr/local/include/ `pkg-config opencv  --libs`

PROGRAM= fingerpaint
OBJECTS= freenect.o utils.o detect.o main.o
CVOBJECTS=cvaux.o cvcontour.o cvtrack.o cvblob.o cvcolor.o cvlabel.o

all: $(PROGRAM) 
# detec $(PROGRAM)_gui

$(PROGRAM): $(OBJECTS) $(CVOBJECTS) 
	#compitle aplication
	$(CXX) $(CFLAGS)  $^ -o $@ $(CVLIBS)
	#create dir for saving image
	@mkdir -p paints

$(PROGRAM)_gui: gui.o detection_class.o 
	$(CXX) $(LIBS) $(CFLAGS) $(CVCFLAGS) $(CVLIBS) $^ -o $@

detec: /usr/local/lib/libcvblob.so detec.o detection_class.o
	$(CXX) $(CVCFLAGS) $(CVLIBS) $^ -o $@

#objects
gui.o:  src/gui.cpp
	$(CXX) $(CFLAGS) $(LIBS) $(CVCFLAGS) $(CVLIBS) -c $^ -o $@

main.o: src/main.cpp
	$(CXX) $(CFLAGS) $(CVCFLAGS) $(CVLIBS) $(CVCFLAGS) -c $^ -o $@

detection_webcam.o: src/detection_webcam.cpp
	$(CXX) $(CVCFLAGS) $(CVLIBS)  -c $^ -o $@

%.o: src/%.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) -c $<  $(LIBS) 

# compile cvblob
cvlabel.o: src/lib/cvlabel.cpp  
	$(CXX) $(CVLIBS) $(CVCFLAGS) -c $^ -o $@
cvtrack.o: src/lib/cvtrack.cpp
	$(CXX) $(CFLAGS) $(CVCFLAGS) -c $^ -o $@
cvcontour.o:  src/lib/cvcontour.cpp 
	$(CXX) $(CFLAGS) $(CVCFLAGS) -c $^ -o $@
cvaux.o: src/lib/cvaux.cpp  
	$(CXX) $(CVLIBS) $(CVCFLAGS) -c $^ -o $@
cvblob.o: src/lib/cvblob.cpp
	$(CXX) $(CFLAGS) $(CVCFLAGS) -c $^ -o $@
cvcolor.o:  src/lib/cvcolor.cpp 
	$(CXX) $(CFLAGS) $(CVCFLAGS) -c $^ -o $@

clean:
	rm *.o
	
count: 
	find ./src/ -name "*.cpp" -print | xargs wc -l 
	
