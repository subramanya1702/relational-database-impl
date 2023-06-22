#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <bits/stdc++.h>

#include "record_class.h"

using namespace std;

#define BUFFER_SIZE 22

#ifndef DEBUG
#define DEBUG false
#endif


// Constants and Global variables
const string deptRunFilePrefix = "DeptRun_";
const string empRunFilePrefix = "EmpRun_";

int deptRunCount = 0; // Stores the number of run files created for Dept relation
int empRunCount = 0; // Stores the number of run files created Emp relation
int bufferIndex = 0; // Tracks the current buffer index
Record buffers[BUFFER_SIZE]; // An array of buffers of BUFFER_SIZE


// Function to split a line using the given delimiter
vector<string> splitLine(string line, char delimiter) {
    vector<string> fields;
    std::stringstream ss(line);
    string substr;

    while (std::getline(ss, substr, delimiter)) {
        fields.push_back(substr);
    }

    return fields;
}

// Function to fetch the smallest eid from the map
int getSmallestEid(unordered_map<int, string> eidAddressMap) {
    int smallestEid = std::numeric_limits<int>::max();

    for (const auto& pair: eidAddressMap) {
        smallestEid = std::min(pair.first, smallestEid);
    }

    return smallestEid;
}

// Function to fetch the smallest managerid from the map
int getSmallestManagerId(unordered_map<int, string> managerIdAddressMap) {
    int smallestManagerId = std::numeric_limits<int>::max();

    for (const auto& pair: managerIdAddressMap) {
        smallestManagerId = std::min(pair.first, smallestManagerId);
    }

    return smallestManagerId;
}

// Function to print the joint record when there is a match between manegerid and eid
void printJoin(ofstream& joinCsv, string deptRecordAddress, string empRecordAddress) {
    string join = "";

    int delimiterPosition = deptRecordAddress.find(":");
    if (delimiterPosition != std::string::npos) {
        string runFileId = deptRecordAddress.substr(0, delimiterPosition);
        int filePointer = stoi(deptRecordAddress.substr(delimiterPosition + 1, deptRecordAddress.length() - runFileId.length() - 1));
        string deptRunFileName = deptRunFilePrefix + runFileId;

        fstream deptRunFile(deptRunFileName);
        deptRunFile.seekp(filePointer, std::ios::beg);
        string line;

        if (deptRunFile.is_open()) {
            if (std::getline(deptRunFile, line)) {
                join = line;
            }
        }

        deptRunFile.close();
    }

    delimiterPosition = empRecordAddress.find(":");
    if (delimiterPosition != std::string::npos) {
        string runFileId = empRecordAddress.substr(0, delimiterPosition);
        int filePointer = stoi(empRecordAddress.substr(delimiterPosition + 1, empRecordAddress.length() - runFileId.length() - 1));
        string empRunFileName = empRunFilePrefix + runFileId;

        fstream empRunFile(empRunFileName);
        empRunFile.seekp(filePointer, std::ios::beg);
        string line;

        if (empRunFile.is_open()) {
            if (std::getline(empRunFile, line)) {
                join += "," + line;
            }
        }

        empRunFile.close();
    }

    cout << join << endl;
    joinCsv << join << endl;
}

// Function that increments managerid from Dept
void incrementDept(unordered_map<int, string>& managerIdAddressMap, int& smallestManagerId) {
    string value = managerIdAddressMap[smallestManagerId];
    int delimiterPosition = value.find(":");

    if (delimiterPosition != std::string::npos) {
        string runFileId = value.substr(0, delimiterPosition);
        int filePointer = stoi(value.substr(delimiterPosition + 1, value.length() - runFileId.length() - 1));
        string deptRunFileName = deptRunFilePrefix + runFileId;

        fstream deptRunFile(deptRunFileName);
        deptRunFile.seekp(filePointer, std::ios::beg);
        string line;
        std::getline(deptRunFile, line);
        filePointer = deptRunFile.tellp();
        managerIdAddressMap.erase(smallestManagerId);

        if (std::getline(deptRunFile, line)) {
            int pos = line.find_last_of(",");
            int index = stoi(runFileId);

            if (pos != std::string::npos) {
                int managerId = stoi(line.substr(pos + 1, line.length()));
                string address = runFileId + ":" + to_string(filePointer);
                managerIdAddressMap[managerId] = address;
            }
        }
        smallestManagerId = getSmallestManagerId(managerIdAddressMap);
        deptRunFile.close();
    } 
}

// Function that increments eid from Emp
void incrementEmp(unordered_map<int, string>& eidAddressMap, int& smallestEid) {
    string value = eidAddressMap[smallestEid];
    int delimiterPosition = value.find(":");

    if (delimiterPosition != std::string::npos) {
        string runFileId = value.substr(0, delimiterPosition);
        int filePointer = stoi(value.substr(delimiterPosition + 1, value.length() - runFileId.length() - 1));
        string empRunFileName = empRunFilePrefix + runFileId;

        fstream empRunFile(empRunFileName);
        empRunFile.seekp(filePointer, std::ios::beg);
        string line;
        std::getline(empRunFile, line);
        filePointer = empRunFile.tellp();
        eidAddressMap.erase(smallestEid);

        if (std::getline(empRunFile, line)) {
            int pos = line.find(",");
            int index = stoi(runFileId);

            if (pos != std::string::npos) {
                int eid = stoi(line.substr(0, pos));
                string address = runFileId + ":" + to_string(filePointer);
                eidAddressMap[eid] = address;
            }
        }
        smallestEid = getSmallestEid(eidAddressMap);
        empRunFile.close();
    } 
}

// Function to merge the results from Dept and Emp relations
// The end result will be a csv file Join.csv which contains the joint records from Dept and Emp
void mergeJoinRuns(unordered_map<int, string>& managerIdAddressMap, unordered_map<int, string>& eidAddressMap) {
    ofstream joinCsv("Join.csv");
    int smallestManagerId = getSmallestManagerId(managerIdAddressMap);
    int smallestEid = getSmallestEid(eidAddressMap);

    // Fall back file pointers for Emp relation
    // It captures the file pointers of Emp relation when dept.managerid = emp.eid
    // Used as a fall back mechanism when there is a mismatch
    unordered_map<int, string> markFilePointers;

    // Loop until there are no elements left in either of the maps
    while (managerIdAddressMap.size() > 0 && eidAddressMap.size() > 0) {
        if (markFilePointers.size() == 0) {
            // Increment managerid from dept until it is equal to or greater than the eid from Emp
            while (smallestManagerId < smallestEid) {
                incrementDept(managerIdAddressMap, smallestManagerId);
            }
            
            // Increment eid from Emp until it is equal to or greater than the managerid from Dept
            while (smallestManagerId > smallestEid) {
                incrementEmp(eidAddressMap, smallestEid);
            }

            // Set the marker file pointers
            markFilePointers = eidAddressMap;
        }

        // If the managerid is equal to eid, print the joined record and increment eid from Emp
        if (smallestManagerId == smallestEid) {
            printJoin(joinCsv, managerIdAddressMap[smallestManagerId], eidAddressMap[smallestEid]);
            incrementEmp(eidAddressMap, smallestEid);
        } else {
            // Reset empAddressMap to marker file pointers in case of a mismatch
            eidAddressMap = markFilePointers;
            smallestEid = getSmallestEid(eidAddressMap);

            // Increment managerid from Dept
            incrementDept(managerIdAddressMap, smallestManagerId);

            // Clear the mark file pointers
            markFilePointers.clear();
        }
    }

    joinCsv.close();
}

// Function to compare records by eid. Used as a comparator while sorting buffers
bool compareByEid(Record a, Record b) {
    return a.emp_record.eid < b.emp_record.eid;
}

// Function to compare records by managerid. Used as a comparator while sorting buffers
bool compareByManagerId(Record a, Record b) {
    return a.dept_record.managerid < b.dept_record.managerid;
}

// Sort the buffers using eid as the sort key in ascending order
// Write the sorted buffers to a run file
void sortBuffers(int type){
    if (type == DEPT) {
        std::sort(buffers, buffers + bufferIndex, compareByManagerId);
        string deptRunFileName = deptRunFilePrefix + to_string(deptRunCount);
        ofstream deptRunFile(deptRunFileName);

        for (int i = 0; i < bufferIndex; i++) {
            deptRunFile << buffers[i].toString(type) << endl;
        }

        deptRunFile.close();
    } else if (type == EMP) {
        std::sort(buffers, buffers + bufferIndex, compareByEid);
        string empRunFileName = empRunFilePrefix + to_string(empRunCount);
        ofstream empRunFile(empRunFileName);

        for (int i = 0; i < bufferIndex; i++) {
            empRunFile << buffers[i].toString(type) << endl;
        }

        empRunFile.close();
    }
}

// Function to execute Pass One of TPMMS and create run files
void executePassOne(string fileName, int type, int& runCount) {
    fstream csv(fileName);

    // Read records from csv file and fill up the buffers
    if (csv.is_open()) {
        string line;

        while(std::getline(csv, line)) {
            Record record = Record(splitLine(line, ','), type);
            buffers[bufferIndex] = record;
            bufferIndex++;

            // When size of the buffers has reached the limit, sort them all and then reset the variables
            if (bufferIndex == BUFFER_SIZE) {
                sortBuffers(type);
                bufferIndex = 0;
                runCount++;
            }
        }
    } else {
        cout << "ERROR: Unable to open " << fileName << "!!!" << endl;
        exit(0);
    }

    // Sort the remanining buffers
    sortBuffers(type);
    bufferIndex = 0;
    runCount++;

    csv.close();
}

// Function to execute pass two/merge phase of the join algorithm
void executePassTwo() {
    // Initialize and populate an address map for the smallest managerids from all dept run files
    // At any given time, the size of the map will be at most # of Dept Runs (< BUFFER_SIZE)
    unordered_map<int, string> managerIdAddressMap;
    
    for (int i = 0; i < deptRunCount; i++) {
        string deptRunFileName = deptRunFilePrefix + to_string(i);
        string line;
        fstream deptRunFile(deptRunFileName);
        deptRunFile.seekp(0, std::ios::beg);

        if (deptRunFile.is_open()) {
            if (std::getline(deptRunFile, line)) {
                int pos = line.find_last_of(",");

                if (pos != std::string::npos) {
                    int eid = stoi(line.substr(pos + 1, line.length()));
                    string address = to_string(i) + ":0";
                    managerIdAddressMap[eid] = address;
                }
            }
        }

        deptRunFile.close();
    }

    // Initialize and populate an address map for the smallest eids from all emp run files
    // At any given time, the size of the map will be at most # of Emp Runs (< BUFFER_SIZE)
    unordered_map<int, string> eidAddressMap;

    for (int i = 0; i < empRunCount; i++) {
        string empRunFileName = empRunFilePrefix + to_string(i);
        string line;
        fstream empRunFile(empRunFileName);
        empRunFile.seekp(0, std::ios::beg);

        if (empRunFile.is_open()) {
            if (std::getline(empRunFile, line)) {
                int pos = line.find(",");

                if (pos != std::string::npos) {
                    int eid = stoi(line.substr(0, pos));
                    string address = to_string(i) + ":0";
                    eidAddressMap[eid] = address;
                }
            }
        }

        empRunFile.close();
    }

    mergeJoinRuns(managerIdAddressMap, eidAddressMap);
}

// Function to delete run files
void deleteRunFiles() {
    for (int i = 0; i < std::max(deptRunCount, empRunCount); i++) {
        if (i < deptRunCount) {
            string deptRunFileName = deptRunFilePrefix + to_string(i);
            if (remove(deptRunFileName.c_str()) != 0) {
                cout << "ERROR: Unable to delete run file " + deptRunFileName << endl;
            }
        }

        if (i < empRunCount) {
            string empRunFileName = empRunFilePrefix + to_string(i);
            if (remove(empRunFileName.c_str()) != 0) {
                cout << "ERROR: Unable to delete run file " + empRunFileName << endl;
            }
        }
    }
}

int main() {
    clock_t start, end;
    start = clock();
    
    // Execute pass one for Dept and Emp Relations
    executePassOne("Dept.csv", DEPT, deptRunCount);
    executePassOne("Emp.csv", EMP, empRunCount);

    // Check if the total number of run files is less than the buffer size to make sure it meets the optimzed sort merge join requirements
    if (deptRunCount + empRunCount >= BUFFER_SIZE) {
        cout << "ERROR: Total number of runs exceeded the allowed threshold. Cannot perform sort merge join operation!!!" << endl;
        return 0;
    }

    // Execute pass two
    executePassTwo();

    // Delete Dept and Emp run files
    deleteRunFiles();

    end = clock();

    if (DEBUG) {
        double elapsedTime = ((end - start) / (double) CLOCKS_PER_SEC) * 1000;
        cout << "Time taken: " << fixed << elapsedTime << "ms" << endl;
    }

    return 0;
}
