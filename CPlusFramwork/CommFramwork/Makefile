#Makefile for 
# AIX version

.SUFFIXES:
.SUFFIXES: .cpp .obj .o

CPP = g++

KERNEL_INC=-I$(HOME)/include -I../Utility -I../NetMgr 

CPPFLAGS =  -g -Wall  -O2 
CPPDEFS = -DLINUX -DUNIX -DNDEBUG -D_THREAD_SAFE -D_MT -DSA_USE_PTHREAD -D_AIX32_THREADS 

.cpp.o:
	$(CPP) $(CPPFLAGS) -c  $(CPPDEFS) $(KERNEL_INC) $< -o $@

OBJL = Connector.o Listener.o ProtocolComm.o ProtocolProcess.o Selector.o

#ALL:   libcommframwork.a  clean
ALL:   libcommframwork.a

libcommframwork.a : $(OBJL)
	ar rv $@ $(OBJL)
	mv $@ $(HOME)/lib
clean:
	rm -f *.o
