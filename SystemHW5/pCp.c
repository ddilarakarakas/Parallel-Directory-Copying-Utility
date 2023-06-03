#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pCp.h"


int main(int argc, char* argv[]) {
    if (argc < 5) {
        printf("Usage: %s <buffer size> <number of consumers> <source directory> <destination directory>\n", argv[0]);
        return 1;
    }
    
    
    signal(SIGINT, int_handler);
    
    bufferSize = atoi(argv[1]);
    numConsumers = atoi(argv[2]);
    sourceDir = argv[3];
    destinationDir = argv[4];
    
    consumerThreadID = malloc(sizeof(pthread_t) * numConsumers);
    
    gettimeofday(&startTime, NULL);
    
    if (pthread_mutex_init(&bufferMutex, NULL) == -1) {
        printf("Mutex initialization failed\n");
        return 1;
    }

    if (pthread_cond_init(&bufferNotEmpty, NULL) == -1) {
        printf("Condition variable initialization failed\n");
        pthread_mutex_destroy(&bufferMutex);
        return 1;
    }

    buffer = (struct FileInfo*)malloc(sizeof(struct FileInfo) * bufferSize);

    
    
    char* directories[] = {strdup(sourceDir), strdup(destinationDir)};
    pthread_create(&producerThreadID, NULL, producerThread, directories);

    for (int i = 0; i < numConsumers; ++i)
        pthread_create(&consumerThreadID[i], NULL, consumerThread, NULL);

    pthread_join(producerThreadID, NULL);
    for (int i = 0; i < numConsumers; ++i)
        pthread_join(consumerThreadID[i], NULL);

    gettimeofday(&endTime, NULL);
    
    pthread_mutex_destroy(&bufferMutex);
    pthread_cond_destroy(&bufferNotEmpty);
    free(buffer);
    free(consumerThreadID);
    free(directories[0]);
    free(directories[1]);
    int i;
    for(i = 0;i<index_source ;i++){
        closedir(sourceDirArr[i]);
    }
    
    double totalTime = (endTime.tv_sec - startTime.tv_sec) +
                       (endTime.tv_usec - startTime.tv_usec) / 1000000.0;

    printf("Total time: %.6f seconds\n", totalTime);
    printf("Total files copied: %d\n", totalFiles);
    printf("Total bytes copied: %d\n", totalBytes);

    return 0;
}

void* producerThread(void* arg) {
    char** directories = (char**)arg;
    const char* srcDir = directories[0];
    const char* destDir = directories[1];

    // Open source directory
    sourceDirP = opendir(srcDir);
    sourceDirArr[index_source] = sourceDirP;
    index_source++;
    
    if (!sourceDirP) {
        printf("Error opening source directory: %s\n", srcDir);
        done = 1;
        return NULL;
    }

    struct dirent* entry;
    while ((entry = readdir(sourceDirP)) != NULL) {
        char filename[256];
        strcpy(filename, entry->d_name);

        // Skip . and ..
        if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
            continue;

        // Create source and destination file paths
        char srcPath[512];
        char destPath[512];
        sprintf(srcPath, "%s/%s", srcDir, filename);
        sprintf(destPath, "%s/%s", destDir, filename);

        // Get file information
        struct stat fileStat;
        if (lstat(srcPath, &fileStat) == -1) {
            printf("Error getting file information: %s\n", srcPath);
            continue;
        }

        // Check if the entry is a directory
        if (S_ISDIR(fileStat.st_mode)) {
            // Recursive call for subdirectory
            char* subdirs[] = {strdup(srcPath), strdup(destPath)};
            producerThread(subdirs);
            free(subdirs[0]);
            free(subdirs[1]);
        } else {
            // Add file info to the buffer
            pthread_mutex_lock(&bufferMutex);
            while (bufferCount == bufferSize) {
                pthread_cond_wait(&bufferNotEmpty, &bufferMutex);
            }
            struct FileInfo fileInfo;
            strcpy(fileInfo.sourcePath, srcPath);
            strcpy(fileInfo.destinationPath, destPath);
            fileInfo.fileType = fileStat.st_mode & S_IFMT;
            buffer[bufferCount] = fileInfo;
            bufferIndex = (bufferIndex + 1);
            bufferCount++;
            totalFiles++;
            totalBytes += fileStat.st_size;
            pthread_mutex_unlock(&bufferMutex);
            pthread_cond_signal(&bufferNotEmpty);
        }
    }
    //bufferIndex = 0;
    done = 1;
    control = 1;

    return NULL;
}


void* consumerThread(void* arg) {
    while (1) {
        pthread_mutex_lock(&bufferMutex);
        while (bufferCount == 0 && done == 0) {
            pthread_cond_wait(&bufferNotEmpty, &bufferMutex);
        }
        if(done && control){
            bufferIndex=0;
            control = 0;
        }
        else if(done == 0){
            bufferIndex--;
        }
        // Check if there are no more entries in the buffer
        if (bufferCount == 0 && done) {
            pthread_mutex_unlock(&bufferMutex);
            break;
        }
        // Get file info from the buffer
        struct FileInfo fileInfo = buffer[bufferCount-1];
        bufferIndex = (bufferIndex + 1);
        bufferCount--;
        pthread_mutex_unlock(&bufferMutex);
        pthread_cond_signal(&bufferNotEmpty);

        // Open source file for reading
        sourceFd = open(fileInfo.sourcePath, O_APPEND | O_CREAT | O_RDWR,0666);
        
        if (sourceFd == -1) {
            printf("Error opening source file: %s\n", fileInfo.sourcePath);
            continue;
        }

        // Open destination file for writing
        destinationFd = open(fileInfo.destinationPath, O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (destinationFd == -1) {
            getParentOfLastIndex(fileInfo.destinationPath);
            destinationFd = open(fileInfo.destinationPath, O_RDWR | O_CREAT | O_TRUNC, 0666);
            continue;
        }

        // Copy file content
        char bufferC[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(sourceFd, bufferC, sizeof(bufferC))) > 0) {
            ssize_t bytesWritten = write(destinationFd, bufferC, bytesRead);
            if (bytesWritten == -1) {
                printf("Error writing to destination file: %s\n", fileInfo.destinationPath);
                break;
            }
        }

        close(sourceFd);
        close(destinationFd);

        printf("Copied file: %s\n", fileInfo.sourcePath);
        switch (fileInfo.fileType) {
            case S_IFREG:
                printf("File type: Regular file\n");
                break;
            case S_IFDIR:
                printf("File type: Directory\n");
                break;
            case S_IFLNK:
                printf("File type: Symbolic link\n");
                break;
            case S_IFCHR:
                printf("File type: Character device\n");
                break;
            case S_IFBLK:
                printf("File type: Block device\n");
                break;
            case S_IFIFO:
                printf("File type: FIFO (named pipe)\n");
                break;
            case S_IFSOCK:
                printf("File type: Socket\n");
                break;
            default:
                printf("File type: Unknown\n");
                break;
        }
    }

    return NULL;
}

void
int_handler(int dummy) {
    fprintf(stdout, "\n================================\n");
    fprintf(stdout,   "UNEXPECTED CANCELLATION OF COPY!\n");
    fprintf(stdout,   "================================\n");
    int i;

    pthread_cancel(producerThreadID);

    for(i = 0; i < numConsumers; i++) {
        pthread_cancel(consumerThreadID[i]);
    }

    
    free(consumerThreadID);
    free(buffer);
    closedir(sourceDirP);
    

    gettimeofday(&endTime, NULL);
    long elapsed = (endTime.tv_sec-startTime.tv_sec) * 1000000 + endTime.tv_usec-startTime.tv_usec;
    fprintf(stdout, "Sucessfully Copied %d files containing %d bytes in %ld milliseconds. There might be more files copied but they are most likely damaged files.\n",
        totalFiles, totalBytes, elapsed / 1000);

    exit(EXIT_SUCCESS);
}

void getParentOfLastIndex(const char* path) {
    char* pathCopy = strdup(path);  // Create a copy of the path
    char* parent = NULL;

    char* token = strtok(pathCopy, "/");  // Split the path using '/'
    char* prevToken = NULL;

    while (token != NULL) {
        prevToken = token;
        token = strtok(NULL, "/");
    }

    if (prevToken != NULL) {
        int parentLength = strlen(path) - strlen(prevToken) - 1;  // Length of the parent
        parent = (char*)malloc((parentLength + 1) * sizeof(char));  // Allocate memory for the parent
        strncpy(parent, path, parentLength);  // Copy the parent path
        parent[parentLength] = '\0';  // Null-terminate the parent string
    }

    free(pathCopy);  // Free the memory allocated for the copy
    int result = mkdir(parent, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    free(parent);
    close(result);
}
