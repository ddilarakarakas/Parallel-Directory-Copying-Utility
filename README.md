# Parallel Directory Copying Utility (pCp) - Project Description

## _Project Overview_
The Parallel Directory Copying Utility (pCp) project aims to create a multithreaded utility that allows efficient copying of files and subdirectories from a source directory to a destination directory. The utility leverages a producer-consumer pattern and a worker thread pool to achieve parallelism and optimized resource utilization.

## _Implementation Details_

##Producer Thread

The producer thread function takes an array of source and destination directory paths as parameters. For each file in the source directory, the producer thread:

- Opens the source file for reading and the destination file for writing.
- If the destination file already exists, it is truncated.
- If an error occurs during file operations, both files are closed, and an error message is displayed.
- The opened file descriptors and file names are placed in a buffer for consumption by consumer threads.

##Consumer Threads

Consumer threads retrieve items from the buffer created by the producer thread. Each consumer thread:

- Reads file descriptors and file names from the buffer.
- Copies data from the source file descriptor to the destination file descriptor.
- Closes both files after copying is complete.
- Writes a message to standard output indicating the file name and completion status.

##Main Program

The main program takes command-line arguments for buffer size, number of consumer threads, source directory, and destination directory. It performs the following steps:

- Initializes the buffer and other necessary data structures.
- Creates the producer thread with the source and destination directory paths.
- Creates the specified number of consumer threads.
- Waits for all threads (producer and consumers) to complete using pthread_join.
- Measures the total time taken for the copying operation using gettimeofday.
- Handles error scenarios, such as file opening errors and exceeding the open file descriptor limit.
- Manages memory to prevent memory leaks.
- Collects statistics about the number and types of files copied, as well as the total number of bytes copied.

##Experimentation and Analysis

The project requires experimentation to determine optimal combinations of buffer size and consumer threads. The following steps should be taken:

- Run the utility with different buffer sizes and consumer thread counts.
- Measure and record the execution times for each combination.
- Analyze the results to identify the best-performing combinations.
- Write a report that presents the experimental setup, execution times, and insights gained from the analysis.

## _Conclusion_

The pCp utility demonstrates the effective use of multithreading and a producer-consumer pattern to achieve parallel directory copying. By experimenting with different parameters, the optimal performance configurations can be identified and leveraged for efficient file copying operations.
