# makefile
# author: Pavel Studeník

GXX=g++
LIBS= -lm #-lX11 -lXi -lXmu -lglut -lGL -lGLU
CFLAGS= -Wall -g -O3
CFLAGS+= `pkg-config gtk+-2.0 --cflags gtkglext-1.0 gtkglext-x11-1.0`
LIBS+= `pkg-config --cflags --libs gtk+-2.0 gmodule-export-2.0 --libs gthread-2.0 gtkglext-1.0 gtkglext-x11-1.0`

CVCFLAGS  += $(CFLAGS) `pkg-config opencv --cflags`
CVLIBS += -I ./lib/h/ `pkg-config opencv --libs`

PROGRAM= fingerpaint

all: cvblob $(PROGRAM) detec $(PROGRAM)_gui
	
$(PROGRAM): main.o detection_class.o ./lib/libcvblob.a
#compitle aplication
	$(CXX) $(LIBS) $(CFLAGS) $(CVCFLAGS) $(CVLIBS) $^ -o $@
#create dir for saving image
	@mkdir -p paints

$(PROGRAM)_gui: gui.o detection_class.o ./lib/libcvblob.a
	$(CXX) $(LIBS) $(CFLAGS) $(CVCFLAGS) $(CVLIBS) $^ -o $@

detec: detec.o detection_class.o ./lib/libcvblob.a
	$(CXX) $(CVCFLAGS) $(CVLIBS) $^ -o $@

#compile external library cvblob
cvblob:
	cd lib/cvblob/; cmake . ; make
	@echo "-----------------------"
	@echo "copy library cvblob ..."
	cp lib/cvblob/lib/libcvblob.a lib/
	@echo "-----------------------"
	@echo "copy header file ..."
	@echo "-----------------------"
	cp lib/cvblob/cvBlob/*.h lib/h/

gui.o:  src/gui.cpp
	$(CXX) $(CFLAGS) $(LIBS) $(CVCFLAGS) $(CVLIBS) -c $^ -o $@

#objects
main.o: src/main.cpp
	$(CXX) $(CFLAGS) $(CVCFLAGS) $(CVLIBS) $(CVCFLAGS) -c $^ -o $@

detec.o: src/detection.cpp  
	$(CXX) $(CVCFLAGS) $(CVLIBS)  -c $^ -o $@

detection_class.o: src/detection_class.cpp
	$(CXX) $(CVCFLAGS) $(CVLIBS)  -c $^ -o $@

		
clean:
	rm *.o
	
count: 
	find ./src/ -name "*.cpp" -print | xargs wc -l 
	
