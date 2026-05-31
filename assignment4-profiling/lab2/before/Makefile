CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic
TARGET := main
SOURCE := main.cpp

.PHONY: all run test clean help build-perf run-perf run-perf-stat build-valgrind run-valgrind build-callgrind run-callgrind

all: $(SOURCE)
	$(CXX) $(CXXFLAGS) -O2 -g $< -o $(TARGET)

# Perf build: adds frame pointers for better call graph resolution
build-perf: $(SOURCE)
	$(CXX) $(CXXFLAGS) -O2 -g -fno-omit-frame-pointer $< -o $(TARGET)

run-perf-stat: build-perf
	taskset -c 0 perf stat -d ./$(TARGET)

run-perf: build-perf
	taskset -c 0 perf record --call-graph dwarf ./$(TARGET)

flamegraph: run-perf
	perf script -i perf.data > perf.script
	~/FlameGraph/stackcollapse-perf.pl perf.script > perf.folded
	~/FlameGraph/flamegraph.pl perf.folded > flamegraph.svg

# Valgrind build: lower optimization (-O1) to ensure accurate stack traces and line numbers
build-valgrind: $(SOURCE)
	$(CXX) $(CXXFLAGS) -O1 -g $< -o $(TARGET)

run-valgrind: build-valgrind
	valgrind --leak-check=full ./$(TARGET)

# Callgrind build (for kCacheGrind): optimized but block inlining
build-callgrind: $(SOURCE)
	$(CXX) $(CXXFLAGS) -O2 -g -fno-inline $< -o $(TARGET)

# Highly Optimized build: -O3, vectorization, and link-time optimizations
build-opt: $(SOURCE)
	$(CXX) $(CXXFLAGS) -O3 -march=native $< -o $(TARGET)

run-opt: build-opt
	time taskset -c 0 ./$(TARGET)

run-callgrind: build-callgrind
	valgrind --tool=callgrind --cache-sim=yes --branch-sim=yes --callgrind-out-file=callgrind.out ./$(TARGET)
	@echo "Run 'kcachegrind callgrind.out' to view the results."

run: all
	./$(TARGET)

run-time: all
	time taskset -c 0 ./$(TARGET)

test: all
	@test "$$(./$(TARGET))" = "6040578838"
	@echo "baseline output matched"

clean:
	rm -f $(TARGET) callgrind.out perf.data*

help:
	@echo "Targets:"
	@echo "  all             - build the baseline program (-O2 -g)"
	@echo "  run             - build and run the baseline program"
	@echo "  run-time        - run the program with execution time measurement"
	@echo "  test            - check the baseline output"
	@echo "  build-perf      - build with frame pointers for perf"
	@echo "  run-perf-stat   - run performance counter stats with perf"
	@echo "  run-perf        - record execution with perf"
	@echo "  flamegraph      - generate flamegraph.svg using perf"
	@echo "  build-valgrind  - build with -O1 for valgrind memcheck"
	@echo "  run-valgrind    - run with valgrind leak-check"
	@echo "  build-callgrind - build for callgrind profiling (-O2)"
	@echo "  run-callgrind   - run callgrind (view with kCacheGrind)"
	@echo "  build-opt       - build with maximum optimizations (-O3 -march=native)"
	@echo "  run-opt         - run the highly optimized build"
	@echo "  clean           - remove the binary and profiling data"
