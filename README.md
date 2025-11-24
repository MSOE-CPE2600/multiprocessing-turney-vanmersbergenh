# System Programming Lab 11 Multiprocessing

## Mandel
The program forks the parent process into child processes, each creating one image. When the child process is finished, it exits and the parent creates a new process for a new image. This process repeats until 50 images are created and all processes finish. The maximum number of child processes is specified using the argument `-n %d`

Ex: `$ ./mandel -n 12`

## Lab 12 Update
The program can use threads for spliting the work for creating a single image across multiple threads. The image is split by rows, and any remaining rows are given to the last thread. The number of threads to use can be set using the argument `-t %d`

Ex: `$ ./mandel -t 12`

Both threads and processors can be used at the same time, with runtime data for different combinations shown in the **Results** section.

#### Compile
Use the included Makefile: `$ make`

## Results
![Graph of Time vs Number of Processors](graph.png) 

Time vs the number of processors results in an exponential curve showing a large difference between 1 and 2 processors, but a small difference between 10 and 20 processors. This is likely due to the laptop having only 12 logical processors.

|            | 1       | 2      | 5      | 10     | 20     | Threads  |
|-----------:|--------:|-------:|-------:|-------:|-------:|:---------|
| 1          | 138.392 | 68.922 | 53.018 | 35.397 | 30.485 |          |
| 2          | 69.410  | 46.009 | 34.022 | 28.675 | 27.900 |          |
| 5          | 40.017  | 29.046 | 28.325 | 26.525 | 27.127 |          |
| 10         | 28.259  | 27.053 | 26.910 | 27.323 | 26.660 |          |
| 20         | 27.033  | 26.758 | 27.589 | 26.028 | 26.735 |          |
| Processors |         |        |        |        |        | Time [s] |

Processors seem to impact runtime more, with a clear difference appearing at 5 processors 1 thread vs 1 processor 5 threads. This is likely due to the extra work of joining the threads at the end of creating the image. There was no "sweet spot" for optimal runtime, the program plateaus whenever processors times threads is greater than 12 because of the physical limitation of the laptop having only 12 logical processors. The fastest runtime was achieved with 20 processors and 10 threads. The runtime for 1 thread is faster than the runtime data used to create the graph because the laptop was plugged in while collecting data, so the processors were not power limited.