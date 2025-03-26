# System-Calls-and-Employee-Salary

This C++ Project for my CSCI 340 Class implements a hash table for storing and retrieving employee salary records using system calls. It simulates a persistent hash table saved to disk in a binary file (employee.bin) and populates it from a text file (employees.txt) using linear probing for collision resolution.

🚀 Features
Creates a fixed-size hash table with 200 buckets, stored in employee.bin
Parses and inserts employee name-salary pairs from a plain-text file
Uses a simple hash function based on ASCII sum modulo bucket count
Handles collisions with linear probing
Supports search for employee records via command line arguments
Outputs matching salaries and hash table indices

📁 Input Files
employees.txt: A text file with each line formatted as Name,Salary

🧾 Output
Creates a binary file employee.bin with hashed employee records
On running with arguments, prints salary and hash index for each searched employee

🛠️ Dependencies
Standard C++ libraries (<iostream>, <fcntl.h>, <unistd.h>, etc.)
Unix-like environment for POSIX system calls (open, creat, read, write, lseek)

🧠 Concepts Used
File-based hashing
Linear probing for collision resolution
Low-level file operations
Buffer manipulation and ASCII-to-integer conversion


