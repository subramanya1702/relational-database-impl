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


// Constants and Global variables
const string runFilePrefix = "Run_";
int runCount = 0; // Stores the number of run files created
int bufferIndex = 0; // Tracks the current buffer index
Record buffers[BUFFER_SIZE]; // An array of buffers of BUFFER_SIZE
vector<int> runFilesPagePointers; // Stores the current file pointers of all the run files


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

// Function to compare record eids. Used as a comparator while sorting buffers
bool compareByEid(Record a, Record b) {
    return a.emp_record.eid < b.emp_record.eid;
}

// Function to fetch the smallest eid from the map
int getSmallestEid(unordered_map<int, string> idAddressMap) {
    int smallestEid = std::numeric_limits<int>::max();

    for (const auto& pair: idAddressMap) {
        if (pair.first < smallestEid) {
            smallestEid = pair.first;
        }
    }

    return smallestEid;
}

// Sort the buffers using eid as the sort key in ascending order
// Write the sorted buffers to a run file
void sortBuffers(){
    std::sort(buffers, buffers + bufferIndex, compareByEid);
    string runFileName = runFilePrefix + to_string(runCount);
    ofstream runFile(runFileName);

    for (int i = 0; i < bufferIndex; i++) {
        runFile << buffers[i].toString() << endl;
    }

    runFilesPagePointers.push_back(0);
    runFile.close();
}

// Function to merge all the runs and create a final sorted file
// At any given moment, the merge function will at most have BUFFER_SIZE - 1 pages (Actually BUFFER_SIZE - 1 eids and their file addresses) in memory
void mergeRuns(ofstream& empCsvSorted, unordered_map<int, string> idAddressMap){

    // Keep merging until only one page/record is left in the map
    while (idAddressMap.size() > 1) {
        int smallestEid = getSmallestEid(idAddressMap);
        string value = idAddressMap[smallestEid];
        int delimiterPosition = value.find(":");

        // Insert the smallest record to the output buffer and then write it to the disk (EmpSorted.csv)
        if (delimiterPosition != std::string::npos) {
            string runFileId = value.substr(0, delimiterPosition);
            int filePointer = stoi(value.substr(delimiterPosition + 1, value.length() - runFileId.length() - 1));
            string runFileName = runFilePrefix + runFileId;

            fstream runFile(runFileName);
            runFile.seekp(filePointer, std::ios::beg);
            string line;

            if (runFile.is_open()) {
                if (std::getline(runFile, line)) {
                    empCsvSorted << line << endl;
                    idAddressMap.erase(smallestEid);
                }
            }

            // Replace the smallestEid with the next record from the file
            if (std::getline(runFile, line)) {
                int pos = line.find(",");
                int index = stoi(runFileId);

                if (pos != std::string::npos) {
                    int eid = stoi(line.substr(0, pos));
                    string address = runFileId + ":" + to_string(runFilesPagePointers[index]);
                    idAddressMap[eid] = address;
                }

                runFilesPagePointers[index] = runFile.tellp();
            }
        } 
    }

    // Write the left over records from the run file
    unordered_map<int, std::string>::iterator it = idAddressMap.begin();
    int remainingKey = it->first;
    string remainingValue = it->second;
    int delimiterPosition = remainingValue.find(":");

    if (delimiterPosition != std::string::npos) {
        string runFileId = remainingValue.substr(0, delimiterPosition);
        int filePointer = stoi(remainingValue.substr(delimiterPosition + 1, remainingValue.length() - runFileId.length() - 1));
        string runFileName = runFilePrefix + runFileId;

        fstream runFile(runFileName);
        runFile.seekp(filePointer, std::ios::beg);
        string line;

        while (std::getline(runFile, line)) {
            empCsvSorted << line << endl;
            idAddressMap.erase(remainingKey);
        }
    }

    empCsvSorted.close();
}

// Function to execute Pass One of TPMMS
void executePassOne() {

    // Initialize and create files
    fstream empCsv("Emp.csv");
    fstream empCsvSortedFile("EmpSorted.csv");
    empCsvSortedFile.close();

    // Read records from the Emp.csv file and fill up the buffers
    if (empCsv.is_open()) {
        string line;

        while(std::getline(empCsv, line)) {
            Record emp = Record(splitLine(line, ','));
            buffers[bufferIndex] = emp;
            bufferIndex++;

            // When size of the buffers has reached the limit, sort them all and then reset the variables
            if (bufferIndex == BUFFER_SIZE) {
                sortBuffers();
                bufferIndex = 0;
                runCount++;
            }
        }
    } else {
        cout << "ERROR: Unable to open Emp.csv file!!!" << endl;
        exit(0);
    }

    // Sort the remanining buffers
    sortBuffers();
    runCount++;

    empCsv.close();

    // Check if the number of run files is less than the buffer size to make sure it meets TPMMS requirements
    if (runCount >= BUFFER_SIZE) {
        cout << "ERROR: Number of runs have exceeded the allowed threshold. Cannot perform TPMMS operation!!!" << endl;
    }
}

// Function to execute Pass Two of TPMMS
void executePassTwo() {
    ofstream empCsvSorted("EmpSorted.csv");

    // A map to store eid as a key and its respective "file index/name + file pointer" of the record as value 
    // At any given moment, the map will have atmost BUFFER_SIZE - 1 elements
    unordered_map<int, string> idAddressMap;
    
    // Start the merge process by picking one page from each of the BUFFER_SIZE buffers
    // Initial set up before the actual merging begins
    for (int i = 0; i < runCount; i++) {
        string runFileName = runFilePrefix + to_string(i);
        string line;
        fstream runFile(runFileName);
        runFile.seekp(runFilesPagePointers[i], std::ios::beg);

        if (runFile.is_open()) {
            if (std::getline(runFile, line)) {
                int pos = line.find(",");

                if (pos != std::string::npos) {
                    int eid = stoi(line.substr(0, pos));
                    string address = to_string(i) + ":" + to_string(runFilesPagePointers[i]);
                    idAddressMap[eid] = address;
                }

                runFilesPagePointers[i] = runFile.tellp();
            }
        }

        runFile.close();
    }

    // Actually begin merging pages from all the run files
    mergeRuns(empCsvSorted, idAddressMap);
}

// Function to print the sorted records
void printSortedRecords(){
    fstream empCsvSorted("EmpSorted.csv");
    string line;

    if (empCsvSorted.is_open()) {
        while (std::getline(empCsvSorted, line)) {
            cout << line << endl;
        }
    } else {
        cout << "ERROR: Unable to open EmpSorted.csv file!!!" << endl;
    }
    return;
}

// Function to delete the run files
void deleteRunFiles() {
    for (int i = 0; i < runCount; i++) {
        string runFileName = runFilePrefix + to_string(i);
        if (remove(runFileName.c_str()) != 0) {
            cout << "ERROR: Unable to delete run file " + runFileName << endl;
        }
    }
}

int main() {
    // Execute pass one
    executePassOne();

    // Execute pass two
    executePassTwo();

    // Print the sorted records
    printSortedRecords();

    // Delete the run files
    deleteRunFiles();

    return 0;
}
