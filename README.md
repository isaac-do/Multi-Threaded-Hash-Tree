# Project Report

This is a program that computes a hash value for an input file using multi-threading to improve the execution time. The program allows the user to input a file name and a desired number of threads to assist in the hashing of the file. 

The experimental set-up was on a server with the following specs: x86_64 architecture, with the Intel(R) Xeon(R) CPU E5-3695 vs2 @ 2.40GHz CPU, made by GenuineIntel, which consisted of 48 CPUs (2 sockets, 12 cores per socket, and 2 threads per core).

The main point of interest was to see how the runtime is affected as the number of threads is increased on the same file input. The program was executed with five test files, each progressively bigger. The experiment tested it out at these increments of threads: 1, 4, 16, 32, 64, 128 and 256. Each increment was tested around 3-5 times and took the average of those runs to ensure an accurate execution time (I did not have an independent use of the server where the test ran, so other programs executing simultaneously could cause ours to slow).
What was discovered was that the execution time does consistently go down as threads are increased, with the exception of the running time of threads between 128 and 256, in some test cases their execution times were extremely close and only marginally faster, but in some cases the time was marginally slower. Overall, the speedup shows diminishing returns as the number of threads increases. The biggest spike of improved execution time happens from 4 to 32 threads, with threads 1-16 seeing an approximate doubling of speedup (this is the only time in which the speedup is close to proportional to the number of threads increased), this slows down slightly from 32-128 threads and significantly less improved (in some cases worse) from 128-256 threads. 

Below is a table of each test case (labeled p2_tc0-p2_tc4), with execution time in seconds and speedup (time taken for a single thread/ time taken for n threads).

![Screenshot 2024-04-27 232239](https://github.com/isaac-do/Multi-Threaded-Hash-Tree/assets/74117874/07b72414-0d53-4ae4-8d1e-66605df35a10)

Below, each of the test cases is graphed to show the execution time vs. number of threads and the speedup vs. number of threads.

![image](https://github.com/isaac-do/Multi-Threaded-Hash-Tree/assets/74117874/272ff07f-7924-4c78-9b61-40582a8d8916)
![image](https://github.com/isaac-do/Multi-Threaded-Hash-Tree/assets/74117874/3d285f19-9183-48ec-b457-a4b3284f4397)
![image](https://github.com/isaac-do/Multi-Threaded-Hash-Tree/assets/74117874/c668a984-f75c-47da-9004-de8fd83f6a5b)
![image](https://github.com/isaac-do/Multi-Threaded-Hash-Tree/assets/74117874/d52d4461-6055-497b-880c-e5fa24c37fdd)
![image](https://github.com/isaac-do/Multi-Threaded-Hash-Tree/assets/74117874/35719d1f-5f77-4c01-8177-b6d87a6544f1)

In conclusion, the speedup achieved by adding threads was a large improvement to the program’s ability to hash files efficiently. Depending on the number of threads added (minimum 4, maximum 256) showed an improvement of a minimum of ~3.53x speedup and a maximum of ~36.35x speedup.
