	 

#Makefile for 
# AIX version

.SUFFIXES:
.SUFFIXES: .cpp .obj .o

CPP = g++

KERNEL_INC=-I$(HOME)/include -I../Utility -I../NetMgr -I../CommFramwork -I../FramworkNm

CPPFLAGS =  -g -Wall  -O2 
CPPDEFS = -DLINUX -DUNIX -DNDEBUG -D_THREAD_SAFE -D_MT -DSA_USE_PTHREAD -D_AIX32_THREADS 

.cpp.o:
	$(CPP) $(CPPFLAGS) -c  $(CPPDEFS) $(KERNEL_INC) $< -o $@

OBJL = ArrayListMsg.o HashtableMsg.o BroadcastPacket.o TradePacket.o IpcPacket.o LinePacket.o PairPacket.o ProcessInterfaceCmd.o ProcessInterfaceH1C.o ProcessInterfaceH2C.o ProcessInterfaceKC.o ProcessinterfaceKS.o

#ALL:  libframworkap.a  clean
ALL:  libframworkap.a

libframworkap.a: $(OBJL)
	ar rv $@ $(OBJL)
	mv $@ $(HOME)/lib
clean:
	rm -f *.o

