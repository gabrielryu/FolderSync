CPPC=g++
PROG=FolderSync
CPP_FLAGS = -std=c++17
OBJS = md5.o main.o

md5.o : md5.h md5.cpp
		$(CPPC) $(CPP_FLAGS) -c md5.cpp

main.o: main.h md5.h main.cpp
		$(CPPC) $(CPP_FLAGS) -c main.cpp

main : $(OBJS)
		$(CPPC) $(CPP_FLAGS) $(OBJS) -o main

clean:
	@echo "clean up"
	@rm -rf *.o