CC = gcc

all: clean ptrace_profiler

clean:
	rm -rf ptrace_profiler
		
ptrace_profiler:
	$(CC) -o ptrace_profiler src/*.c -lunwind-x86_64 -lunwind-ptrace


