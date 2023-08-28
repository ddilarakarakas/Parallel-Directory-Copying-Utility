//
//  pCp.h
//  SystemHW5
//
//  Created by Dilara Karakaş on 3.06.2023.
//

#ifndef pCp_h
#define pCp_h


struct FileInfo {
    char sourcePath[256];
    char destinationPath[256];
    int fileType;
};

struct timeval startTime, endTime;
pthread_mutex_t bufferMutex;
pthread_cond_t bufferNotEmpty;
struct FileInfo* buffer;
int bufferCount = 0;
int bufferIndex = 0;
int done = 0;
int totalFiles = 0;
int totalBytes = 0;
int bufferSize = 0;
int consumerBufferIndex = 0;
int control = 1;
int numConsumers;
char* sourceDir;
char* destinationDir;
pthread_t producerThreadID;
pthread_t* consumerThreadID;
DIR* sourceDirArr[50000];
int index_source = 0;

/* Producer Global Variable */
// Source directory
DIR* sourceDirP;

/* Consumer Global Variable */
int sourceFd;
int destinationFd;


void* producerThread(void* arg);
void* consumerThread(void* arg);
void int_handler(int dummy);
void getParentOfLastIndex(const char* path);


#endif /* pCp_h */
