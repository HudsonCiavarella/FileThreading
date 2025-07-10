#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>

const int QUEUE_CONDITION_SIZE = 20; //condition for reading and writing queue

/*
*   Thread arguments for readLine + writeLine
*   Includes file stream info, queue for lines
*   and mutex locks and condition variables
*/
struct t_arg {
    std::ifstream *inFile;
    std::ofstream *outFile;

    std::queue<std::string> *lineQueue;
    pthread_mutex_t *mutex;
    pthread_cond_t *queueNotFull;
    pthread_cond_t *queueNotEmpty;
    bool *readingDone;
};

/*
*   readLine thread function:
*   Takes turns reading lines from a source file
*   and adds them to the line queue
*/
void* readLine(void* args) {
    struct t_arg *fileArgs = (struct t_arg *)args;
    std::ifstream *inFile = fileArgs->inFile;
    std::string line;

    if(inFile) {
        while(true) {
            pthread_mutex_lock(fileArgs->mutex);    //begins mutex lock

            //checks if there are no more lines left to read
            if (!std::getline(*inFile, line)) {
                *(fileArgs->readingDone) = true;    //used to notify writers that reading is done
                pthread_cond_broadcast(fileArgs->queueNotEmpty); //tells all writers to write everything in queue as reading is done
                pthread_mutex_unlock(fileArgs->mutex);  //branch mutex unlock (avoids deadlocking)
                break; 
            }

            //if queue condition is met, wait until writers free up space
            while(fileArgs->lineQueue->size() == QUEUE_CONDITION_SIZE) {
                pthread_cond_wait(fileArgs->queueNotFull, fileArgs->mutex);
            }

            fileArgs->lineQueue->push(line);   

            pthread_cond_signal(fileArgs->queueNotEmpty);   //tells writers queue is not empty

            pthread_mutex_unlock(fileArgs->mutex);  //ends mutex lock
        }

    }

    return nullptr;
}

/*
*   writeLine thread function:
*   Takes turns popping lines off the queue
*   and writing them into a destination file
*/
void* writeLine(void* args) {
    struct t_arg *fileArgs = (struct t_arg *)args;
    std::ofstream *outFile = fileArgs->outFile;
    std::string line;

    if(outFile) {
        while(true) {
            pthread_mutex_lock(fileArgs->mutex); //begins mutex lock

            //if queue is empty, wait until readers add to queue
            while (fileArgs->lineQueue->size() == 0 && !*(fileArgs->readingDone)) {
                pthread_cond_wait(fileArgs->queueNotEmpty, fileArgs->mutex);
            }

            //once reading is done and queue is empty, exit thread
            if (fileArgs->lineQueue->size() == 0 && *(fileArgs->readingDone)) {
                pthread_mutex_unlock(fileArgs->mutex);  //branch mutex unlock (avoids deadlocking)
                break; 
            }

            line = fileArgs->lineQueue->front();
            fileArgs->lineQueue->pop();

            pthread_cond_signal(fileArgs->queueNotFull);    //tells readers queue is not full

            *outFile << line << std::endl;

            pthread_mutex_unlock(fileArgs->mutex);  //ends mutex lock
        }
    }

    return nullptr;
}

/*
*   mscopier takes 4 arguments:
*   - ./mmcopier the program which is not used
*   - n the number of threads to read and write
*   - source_file the directory of the source file
*   - destination_file the directory of the destination file
*/
int main(int argc, char** argv) {
    //checks command line argument quantity
    if (argc != 4) {
        std::cout << "Incorrect amount of command line arguments" << std::endl;
        return 1;
    }

    //checks if amount of files is within scope
    int n = atoi(argv[1]);
    if (n < 2 || n > 10) {
        std::cout << "Invalid number of threads" << std::endl;
        return 1;
    }

    //file information
    std::string sourceFile = argv[2];
    std::string destinationFile = argv[3];
    std::ifstream inFile;
    std::ofstream outFile;

    inFile.open(sourceFile);
    if(inFile.fail()) {
        std::cout << "Source file error" << std::endl;
        return 1;
    }

    outFile.open(destinationFile);
    if(outFile.fail()) {
        std::cout << "Destination file error" << std::endl;
        return 1;
    }

    std::cout << "Copying file..." << std::endl;

    pthread_t *threads = new pthread_t[n * 2];  //creates n writers and n readers (n * 2)
    struct t_arg *threadArgs = new t_arg[n * 2];

    //sets up thread argument structure
    std::queue<std::string> lineQueue;
    pthread_mutex_t mutex;
    pthread_cond_t queueNotFull;
    pthread_cond_t queueNotEmpty;
    bool readingDone = false;
    int ret;

    ret = pthread_mutex_init(&mutex, nullptr);
    if (ret != 0) {
        std::cout << "Mutex fail error" << std::endl;
    }

    ret = pthread_cond_init(&queueNotFull, nullptr);
    if (ret != 0) {
        std::cout << "Condition variable fail error" << std::endl;
    }

    ret = pthread_cond_init(&queueNotEmpty, nullptr);
    if (ret != 0) {
        std::cout << "Condition variable fail error" << std::endl;
    }

    //halves the threads into readers and writers
    for(int i = 0; i < (n * 2); ++i) {
        threadArgs[i].inFile = &inFile;
        threadArgs[i].outFile = &outFile;
        threadArgs[i].lineQueue = &lineQueue;
        threadArgs[i].mutex = &mutex;
        threadArgs[i].queueNotFull = &queueNotFull;
        threadArgs[i].queueNotEmpty = &queueNotEmpty;
        threadArgs[i].readingDone = &readingDone;

        if (i % 2 == 0) {
            ret = pthread_create(&threads[i], nullptr, &writeLine, &threadArgs[i]);
        }
        else {
            ret = pthread_create(&threads[i], nullptr, &readLine, &threadArgs[i]);
        }

        if (ret != 0) {
            std::cout << "Thread creation error" << std::endl;
        }
    }

    //ensures threads finish after termination
    for(int i = 0; i < (n * 2); ++i) {
        ret = pthread_join(threads[i], nullptr);
        if (ret != 0) {
            std::cout << "Thread join error" << std::endl;
        }
    }

    std::cout << "File copying finished!" << std::endl;

    //mutex and condition variables clean up
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&queueNotFull);
    pthread_cond_destroy(&queueNotEmpty);

    //file clean up
    inFile.close();
    outFile.close();

    //memory clean up
    delete[] threads;
    delete[] threadArgs;

    return 0;
}