To compile - make all
To clean - make clean

Task 1 Instructions

./mmcopier n source_dir destination_dir

Example:

./mmcopier 5 /Users/hudsonciavarella/Desktop/OperatingSystemPrinciples/Assignment1/Task1/source_dir /Users/hudsonciavarella/Desktop/OperatingSystemPrinciples/Assignment1/Task1/destination_dir

Task 2 Instructions

./mscopier n source_file destination_file

Example: 

./mscopier 5 /Users/hudsonciavarella/Desktop/OperatingSystemPrinciples/Assignment1/Task2/input.txt /Users/hudsonciavarella/Desktop/OperatingSystemPrinciples/Assignment1/Task2/output3.txt

Locks + Condition Variables (mscopier.cpp):

Readers - Lock begins at 37 and ends at 56, however can be unlocked if reading finished at 43.
        - Condition wait within lock at 49 which waits if queue is full.
        - Condition signal for writers at 54 after pushing aline to queue.

Writers - Lock begins at 76 and ends at 96, however can be unlocked if reading is finished and queue is empty at 85.
        - Condition wait within lock at 80 which waits if queue is empty.
        - Condition signal for readers at 92 after removing a line from queue.

