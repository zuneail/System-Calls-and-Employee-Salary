// Zuneail Wahid 
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h> //file permissions for creat and open... '0644'
#include <cstdlib> //atoi() "ascii ->> int"
#include <cstring>

using namespace std;

const int BUCKET_COUNT = 200;
const int NAME_SIZE = 16;             //Defining sizes for the buffer, names, and buckets
const int BUF_SIZE = 128;

struct Bucket {
    char name[NAME_SIZE + 1];  // Employee name (max 16 characters + null terminator)
    int salary;                // Employee salary; -1 indicates an empty bucket
};

int hashFunction(const char *name) {    //The function is the sum of the name's ascii % the bucket count
    int sum = 0;
    for (int i = 0; i < NAME_SIZE && name[i] != '\0'; i++) {
        sum += name[i];
    }
    return sum % BUCKET_COUNT;
}

// getLine reads one line up to a newline or BUF_SIZE-1 characters from file descriptor into buf.
// It returns the number of characters read.
int getLine(int fd, char *buf, int bufSize) {
    int i = 0;
    char c;
    int ret;
    while (i < bufSize - 1 && (ret = read(fd, &c, 1)) == 1) { //checks that the return value of the file its reading is 1, and so long as it is then it continues reading line by line into the buffer storing 128 chars
        if (c == '\n')
            break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

int main(int argc, const char *argv[]) {
    int fbin, femp;     //holds the file's identifier 
    char buf[BUF_SIZE];
    int n;

    // make sure theres atleast one arguement in the commnad line 
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <employee_name> [<employee_name> ...]" << endl;
        return 1;
    }

    //1: Create and initialize "employee.bin"
    fbin = creat("employee.bin", 0644); //owner read and write, group read, other read.
    if (fbin == -1) {
        cout << "Can't create employee.bin" << endl;
        return 1;
    }
    Bucket empty; //create empty bucket
    memset(empty.name, 0, sizeof(empty.name)); //sets every name value in the bucket as 0
    empty.salary = -1;
    for (int i = 0; i < BUCKET_COUNT; i++) {
        if (write(fbin, &empty, sizeof(Bucket)) != sizeof(Bucket)) { //makes sure that the full size of the bucket was written to employees.bin
            cout << "Error writing empty bucket" << endl;
            return 1;
        }
    }
    close(fbin);

   //2: Read employees.txt and insert records
    femp = open("employees.txt", O_RDONLY, 0); //opens the file with read perms 
    if (femp == -1) {
        cout << "Can't open employees.txt" << endl;
        return 1;
    }
    fbin = open("employee.bin", O_RDWR, 0); //opens fil for reading and writing
    if (fbin == -1) {
        cout << "Can't open employee.bin for update" << endl;
        close(femp);
        return 1;
    }
    while ((n = getLine(femp, buf, BUF_SIZE)) > 0) {
        if (buf[0] == '\0')
            continue;
        // Look for the comma separating name and salary.
        char *comma = strchr(buf, ','); //looks for the first comma
        if (comma == NULL)
            continue;   //if the line dosent have a comma then just go to the next line its malformed
        *comma = '\0'; //create partition
        char *name = buf;
        char *salStr = comma + 1; //uses the null char's partition to find the salary from the name 
        int salary = atoi(salStr); //conversion from ascii -> integer so it can be stored

        // Insert the record using linear probing.
        int index = hashFunction(name); //hash the name using our function
        int start = index;
        Bucket temp;//create a temp bucket
        while (1) {
            int offset = index * sizeof(Bucket); 
            if (lseek(fbin, offset, SEEK_SET) < 0) { //in the binary file, jump or "seek" to the offset calculated for reading
                cout << "lseek error during insertion" << endl;
                return 1;
            }
            if (read(fbin, &temp, sizeof(Bucket)) != sizeof(Bucket)) { //read at that offset 
                cout << "read error during insertion" << endl;
                return 1;
            }
            if (temp.name[0] == '\0') {  // Empty bucket found at offset
                Bucket rec;//create a record
                memset(rec.name, 0, sizeof(rec.name));//make sure its clear
                strncpy(rec.name, name, NAME_SIZE); //copies the name up to the max size
                rec.name[NAME_SIZE] = '\0'; //terminates name with a null 
                rec.salary = salary; //set the salary 
                if (lseek(fbin, offset, SEEK_SET) < 0) { //jump to the offset calculated for writing
                    cout << "lseek error before write" << endl;
                    return 1;
                }
                if (write(fbin, &rec, sizeof(Bucket)) != sizeof(Bucket)) { //write the record at that offset in its entirety
                    cout << "write error during insertion" << endl;
                    return 1;
                }
                break;
            }
            index = (index + 1) % BUCKET_COUNT; //lin probing if that offset was full
            if (index == start) {
                cout << "Hash table is full. Cannot insert " << name << endl;
                return 1;
            }
        }
    }
    close(femp);
    close(fbin);

    //3: Process searches from command line args
    fbin = open("employee.bin", O_RDONLY, 0); //open the binary file for reading
    if (fbin == -1) {
        cout << "Can't open employee.bin for searching" << endl;
        return 1;
    }
    for (int i = 1; i < argc; i++) { //go through each arguement given
        int index = hashFunction(argv[i]); //use our hash function to find the index in the file
        int start = index;
        int found = 0;
        int foundSalary = -1;
        Bucket temp; //create a temporary bucket
        while (1) {
            int offset = index * sizeof(Bucket);
            if (lseek(fbin, offset, SEEK_SET) < 0) { //seek or jmp to that offset in the file
                cout << "lseek error during search" << endl;
                return 1;
            }
            if (read(fbin, &temp, sizeof(Bucket)) != sizeof(Bucket)) { //read the contents in that offset
                cout << "read error during search" << endl;
                return 1;
            }
            if (temp.name[0] == '\0') //if nothings there then break the loop and continue to output to terminal
                break;
            if (strncmp(temp.name, argv[i], NAME_SIZE) == 0) { //if somethings there then update the salary in the temp bucket, and set that we found something to true and continue to terminal output
                foundSalary = temp.salary;
                found = 1;
                break;
            }
            index = (index + 1) % BUCKET_COUNT; //in the case of there being a collision
            if (index == start)
                break;
        }
        if (found)
            cout << "Name = " << argv[i] << "\t Salary = " << foundSalary << "\t Index = " << index << endl;
        else
            cout << "Name = " << argv[i] << "\t Salary = Not found\t Index = Not found" << endl;
    }
    close(fbin);
    return 0;
}
