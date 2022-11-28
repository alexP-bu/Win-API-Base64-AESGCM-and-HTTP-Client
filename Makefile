CC=gcc
CXX=g++


ifeq ($(OS),Windows_NT)
	RM=powershell /c rm
else	
	RM=rm
endif

all: base64.exe aestest.exe http.exe 


.PHONY: all clean 

base64.exe:
	$(CXX) -Wall base64.cpp   -lcrypt32  -static   -o bin/base64.exe 

aestest.exe:
	$(CXX) -Wall aesgcm.cpp  test_aesgcm.cpp  -lcrypt32 -lbcrypt -static  -o bin/aesgcm.exe

http.exe:
	$(CXX) -Wall http.cpp -lwinhttp -static   -o bin/http.exe

submission.zip:
	zip -r submission.zip Makefile base64 http aesgcm 

clean:
	$(RM)  bin/*.exe 