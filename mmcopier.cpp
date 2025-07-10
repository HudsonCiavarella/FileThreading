#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>

/*
*   Thread Arguments for copyFile thread function:
*   Gives file information for each thread
*/
struct t_arg {
    std::string sourceDir;
    std::string destinationDir;
};

/*
*   CopyFile thread function:
*   Each thread uses a file input and output stream to
*   copy its allocated file
*/
void* copyFile(void* args) {
    struct t_arg *fileArgs = (struct t_arg *)args;
    std::ifstream inFile;
    std::ofstream outFile;
    std::string line;

    //opens checks if source file opened correctly
    inFile.open(fileArgs->sourceDir);
    if(inFile.fail()) {
        std::cout << "Source file error" << std::endl;
        return (void*)1;
    }

    //opens checks if destination file opened correctly
    outFile.open(fileArgs->destinationDir);
    if(outFile.fail()) {
        std::cout << "Destination file error" << std::endl;
        return (void*)1;
    }

    //reads from source file and writes to destination file
    if(inFile && outFile) {
        while (std::getline(inFile, line)) {
            outFile << line << std::endl;
        }
    }

    inFile.close();
    outFile.close();

    return nullptr;
}

/*
*   mmcopier takes 4 arguments:
*   - ./mmcopier the program which is not used
*   - n the number of threads / files to read
*   - source_dir the source directory of files
*   - destination_dir the destination directory for files
*   Note: mmcopier assumes file names begin with "source" and dynamically increment by 1
*/
int main(int argc, char** argv) {
    //checks command line argument quantity
    if (argc != 4) {
        std::cout << "Incorrect amount of command line arguments" << std::endl;
        return 1;
    }

    //checks if amount of files is within scope
    int n = atoi(argv[1]);
    if (n <= 0 || n > 10) {
        std::cout << "Invalid number of files" << std::endl;
        return 1;
    }

    std::string sourceDir = argv[2];
    std::string destinationDir = argv[3];

    std::cout << "Copying files..." << std::endl;

    pthread_t *threads = new pthread_t[n]; //threads reference array (heap)
    struct t_arg *threadArgs = new t_arg[n]; //thread arguments array (heap)
    int ret; 
    bool fileFail = false;

    //creates each thread with incrementing directory
    for(int i = 0; i < n; ++i) {
        std::string fileName = "/source" + std::to_string(i + 1) + ".txt";
        threadArgs[i].sourceDir = sourceDir + fileName;
        threadArgs[i].destinationDir = destinationDir + fileName;
        ret = pthread_create(&threads[i], nullptr, &copyFile, &threadArgs[i]);
        if (ret != 0) {
            std::cout << "Thread creation error" << std::endl;
            fileFail = true;
        }
    }

    //Ensures threads finish after program termination
    for(int i = 0; i < n; ++i) {
        void* threadOutcome;
        ret = pthread_join(threads[i], &threadOutcome);
        if (ret != 0) {
            std::cout << "Thread join error" << std::endl;
            fileFail = true;
        }
        else if (threadOutcome == (void*)1) {
            fileFail = true;
        }
    }

    if (fileFail) {
        std::cout << "Failed to copy files." << std::endl;
    }
    else {
        std::cout << "Files copied successfully!" << std::endl;
    }
    
    delete[] threads;
    delete[] threadArgs;

    return 0;
}
