# makefile
# author: Pavel Studeník

GXX=g++
LIBS= -lm #-lX11 -lXi -lXmu -lglut -lGL -lGLU
CFLAGS= -Wall -g
CFLAGS  += `pkg-config gtk+-2.0 --cflags`
LIBS += `pkg-config gtk+-2.0 --libs`

CVCFLAGS  += -Wall -g `pkg-config opencv --cflags`

CVLIBS += -I ./lib/h/ `pkg-config opencv --libs`

all: cvblob poloc detec
	
poloc: main.o detection_class.o ./lib/libcvblob.a
	$(CXX) $(LIBS) $(CFLAGS) $(CVCFLAGS) $(CVLIBS) $^ -o $@

detec: detec.o detection_class.o ./lib/libcvblob.a
	$(CXX) $(CVCFLAGS) $(CVLIBS) $^ -o $@
	
cvblob:
	cd lib/cvblob/; cmake . ; make
	@echo "-----------------------"
	@echo "copy library cvblob ..."
	cp lib/cvblob/lib/libcvblob.a lib/
	@echo "-----------------------"
	@echo "copy header file ..."
	@echo "-----------------------"
	cp lib/cvblob/cvBlob/*.h lib/h/


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
	
