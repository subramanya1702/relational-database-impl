#include <string>
#include <ios>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include "classes.h"

using namespace std;

// Function to create B+Tree Index and set up everything
void initializeBPlusTreeIndex() {
    BPlusTree b;
    b.createFromFile("Employee.csv");
}

// Deserialize the B+Tree index file
BPlusPageNode* getPageNode() {
    BPlusTree loaded_tree;
    return loaded_tree.deserializeUsingBFS("EmployeeIndex");
}

// Entrypoint
int main(int argc, char *const argv[]) {
    initializeBPlusTreeIndex();

    BPlusPageNode* pageNode = getPageNode();
    
    // Initialize the data file
    fstream file("DataFile");

    while (1) {
        // Read user input
        string id;
        cout << "Enter employee Id:" << endl;
        cin >> id;

        // Search for the Employee Id in the B+Tree index and get the memory address range of the record in data file
        string memoryAddresses = pageNode->search(stoi(id));

        // If the Id is not found, print an apprioriate message and continue to read user input
        if (memoryAddresses == "NOT_FOUND") {
            cout << "Employee Id not found!!!" << endl;
            continue;
        }

        // Extract the start and end addresses
        int delimiterPosition = memoryAddresses.find(":");
        int startAddress = stoi(memoryAddresses.substr(0, delimiterPosition));
        int endAddress = stoi(memoryAddresses.substr(delimiterPosition + 1, memoryAddresses.length() - 1)) - 1;
        
        // Seek the data file between start and end addresses and read its contents
        file.seekg(startAddress);
        char content[endAddress - startAddress + 1];
        file.read(content, endAddress - startAddress + 1);
        content[endAddress - startAddress] = '\0';
        
        // Convert the content to a Record object
        Record record(splitLine(content));

        // Print the record object
        record.print();
    }   

    return 0;
}
