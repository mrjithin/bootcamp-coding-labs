# Intro Profiling Lab Report

## 1. Optimizations Made

- TODO

## 2. Methodology Walkthrough

Include before/after evidence from:

- `time`
- `perf stat`
- FlameGraph
- Callgrind/KCachegrind
- Valgrind leak summary

## 3. Correctness Evidence

Include:

- `make test`
- Final normal run output
- Checksum comparison before and after optimization

## 4. Conceptual Questions

A1.1: user+sys computes the time spent by the CPU actually running the program while real calculates the real time elapsed from the start of the program till the end. Since the program may wait for processes like I/O or for other programs to run in between, the real time may not match user+sys time. 

A2.1: These event counts are read directly from CPU's Performance Monitoring Units (PMUs) which are present as hardware. These event counts are used by perf program to calculate derived metrics.

A2.2: Since the CPU has limit counting units, if we try to measure more parameters than available units, the CPU has to rotate among the parameters. So the percentage denotes the active percentage of time the specific parameter was measured. 

A2.3: Due to the above rotating of PMUs, the count is not exact and is approximated. 