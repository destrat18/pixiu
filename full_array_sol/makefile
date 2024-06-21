CFLAGS = -g -pthread -fsanitize=address -ftrapv -fsanitize=undefined -std=c++17
CFLAGS_FAST = -O2 -pthread -std=c++17 -lstdc++
CFLAGS_FAST_OMP = -O2 -fopenmp -pthread -std=c++17 -lstdc++
CC = g++
WORKDIR = src

REQUIRED_TO_RUN = user2.cpp utils.cpp parsetd.cpp tdlib_wrapper.cpp nicify.cpp my_graph.cpp parse_gr.cpp ./flow-cutter/*.cpp

release:
	cd $(WORKDIR); $(CC) $(CFLAGS_FAST_OMP) -o run $(REQUIRED_TO_RUN)
test:
	cd $(WORKDIR); $(CC) $(CFLAGS) -o run $(REQUIRED_TO_RUN)

