#include <iostream>
#include <random>
#include <utility>
#include <vector>
#include <tuple>
#include <random>
#include <utility>
#include <queue>

#include <bits/stdc++.h>

using namespace std;

// Split the record into a list with ',' as the delimitor
vector<string> splitLine(string line) {
    vector<string> fields;
    stringstream ss(line);
    string substr;

    while (std::getline(ss, substr, ',')) {
        fields.push_back(substr);
    }

    return fields;
}

// Class representing a tuple from the Employee.csv file
class Record
{
public:
    int id, manager_id;
    std::string bio, name;

    Record(vector<std::string> fields)
    {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }

    void print()
    {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }
};

class BPlusNode {
 public:
    BPlusNode(BPlusNode *parent = nullptr, bool isLeaf = false, BPlusNode *prev_ = nullptr, BPlusNode *next_ = nullptr)
        : parent(parent), isLeaf(isLeaf), prev(prev_), next(next_) {
        if (next_) {
            next_->prev = this;
        }

        if (prev_) {
            prev_->next = this;
        }
    }

    // List of keys
    vector<int> keys;

    // Pointer to the parent node
    BPlusNode *parent;

    // List of children nodes. For a leaf node, the children will be empty
    vector<BPlusNode *> children;

    // List of values for leaf node keys. The values here are pointers (memory address) to the data in data file
    vector<string> values;

    // next and prev form the doubly linked list
    // Pointer to the next leaf node
    BPlusNode *next;

    // Pointer to the previous leaf node
    BPlusNode *prev;

    // Denotes if a node is a leaf
    bool isLeaf;

    // Address of the current node in the index file
    int nodeAddress;

    // Function to find the index of a child from the list of keys given a key
    int indexOfChild(int key) {
        for (int i = 0; i < keys.size(); i++) {
            if (key < keys[i]) {
                return i;
            }
        }
        return keys.size();
    }

    // Function to the get the child node given a key
    BPlusNode *getChild(int key) { 
        return children[indexOfChild(key)]; 
    }

    // Function to insert the key and child nodes to the existing lists
    void setChild(int key, vector<BPlusNode *> value) {
        int i = indexOfChild(key);
        keys.insert(keys.begin() + i, key);
        children.erase(children.begin() + i);
        children.insert(children.begin() + i, value.begin(), value.end());
    }

    // Function to split a node when it reaches maximum capacity
    tuple<int, BPlusNode *, BPlusNode *> splitInternal() {
        BPlusNode *left = new BPlusNode(parent, false, nullptr, nullptr);
        int mid = keys.size() / 2;

        copy(keys.begin(), keys.begin() + mid, back_inserter(left->keys));
        copy(children.begin(), children.begin() + mid + 1,
            back_inserter(left->children));

        for (BPlusNode *child : left->children) {
            child->parent = left;
        }

        int key = keys[mid];
        keys.erase(keys.begin(), keys.begin() + mid + 1);
        children.erase(children.begin(), children.begin() + mid + 1);

        return make_tuple(key, left, this);
    }

    // Function to get the value of the given key.
    // It returns the memory address of the data file for the given Employee Id
    string get(int key) {
        int index = -1;

        for (int i = 0; i < keys.size(); ++i) {
            if (keys[i] == key) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            return "NOT_FOUND";
        }

        return values[index];
    }

    // Function to insert the key and value to their appropriate lists
    void set(int key, string value) {
        int i = indexOfChild(key);
        if (find(keys.begin(), keys.end(), key) == keys.end()) {
            keys.insert(keys.begin() + i, key);
            values.insert(values.begin() + i, value);
        } else {
            values[i - 1] = value;
        }
    }

    // Function to split the leaf when it reaches maximum capacity
    tuple<int, BPlusNode *, BPlusNode *> splitLeaf() {
        BPlusNode *left = new BPlusNode(parent, true, prev, this);
        int mid = keys.size() / 2;

        left->keys = vector<int>(keys.begin(), keys.begin() + mid);
        left->values = vector<string>(values.begin(), values.begin() + mid);

        keys.erase(keys.begin(), keys.begin() + mid);
        values.erase(values.begin(), values.begin() + mid);

        return make_tuple(keys[0], left, this);
    }
    
    // Function to serialize the B+tree
    std::string serialize() {
        std::stringstream ss;

        int currentAddress = ss.tellp();
        ss << currentAddress << "\n";
        ss << isLeaf << "\n";
        ss << keys.size() << "\n";
        for (int i = 0; i < keys.size(); i++) {
            ss << keys[i] << " ";
        }

        ss << "\n";
        ss << children.size() << "\n";

        for (int i = 0; i < children.size(); i++) {
            ss << children[i]->serialize();
        }

        ss << values.size() << "\n";
        for (int i = 0; i < values.size(); i++) {
            ss << values[i] << "\n";
        }

        return ss.str();
    }

    // Function to deserialize the B+tree
    void deserialize(std::stringstream& ss) {
        ss >> nodeAddress;
        ss >> isLeaf;
        int n_keys;
        ss >> n_keys;
        keys.resize(n_keys);
        for (int i = 0; i < n_keys; i++) {
            ss >> keys[i];
        }

        int n_children;
        ss >> n_children;
        children.resize(n_children);
        for (int i = 0; i < n_children; i++) {
            children[i] = new BPlusNode();
            children[i]->parent = this;
            children[i]->deserialize(ss);
        }

        int n_values;
        ss >> n_values;
        values.resize(n_values);
        for (int i = 0; i < n_values; i++) {
            ss >> values[i];
        }
    }

    std::string serializeUsingBFS(std::stringstream& ss) {
        // Node Address
        ss << ss.tellp() << "\n";
        ss << isLeaf << "\n";
        ss << keys.size() << "\n";
        for (int i = 0; i < keys.size(); i++) {
            ss << keys[i] << " ";
        }

        ss << "\n";
        ss << children.size() << "\n";

        if (isLeaf) {
            ss << values.size() << "\n";
            for (int i = 0; i < values.size(); i++) {
                ss << values[i] << "\n";
            }
        }

        return ss.str();
    }

    void deserializeUsingBFS(std::stringstream& ss, vector<BPlusNode*>& listOfBPlusNodes) {
        ss >> nodeAddress;
        ss >> isLeaf;

        int n_keys;
        ss >> n_keys;
        keys.resize(n_keys);
        for (int i = 0; i < n_keys; i++) {
            ss >> keys[i];
        }

        int n_children;
        ss >> n_children;
        children.resize(n_children);

        if (isLeaf) {
            int n_values;
            ss >> n_values;
            values.resize(n_values);

            for (int i = 0; i < n_values; i++) {
                ss >> values[i];
            }
        }

        listOfBPlusNodes.push_back(this);
    }

    void deserializeOnlyRoot(std::stringstream& ss, 
                             vector<BPlusNode*>& listOfBPlusNodes,
                             int& numOfDeserializations) {
        ss >> isLeaf;

        int n_keys;
        ss >> n_keys;
        keys.resize(n_keys);
        for (int i = 0; i < n_keys; i++) {
            ss >> keys[i];
        }

        int n_children;
        ss >> n_children;
        children.resize(n_children);

        if (isLeaf) {
            int n_values;
            ss >> n_values;
            values.resize(n_values);

            for (int i = 0; i < n_values; i++) {
                ss >> values[i];
            }
        }

        listOfBPlusNodes.push_back(this);
    }
};

class BPlusPageNode {
public:
    bool isLeaf;
    int numKeys;
    vector<int> keys;
    int numChildren;
    vector<int> childAddresses;
    vector<string> values;
    vector<BPlusPageNode*> children;

    BPlusPageNode() {}

    void set(BPlusNode node) {
        isLeaf = node.isLeaf;
        numKeys = node.keys.size();
        keys = node.keys;
        numChildren = node.children.size();
        children.resize(numChildren);
        childAddresses.resize(numChildren);
    }

    void setChild(BPlusPageNode* childNode, int address, int childIndex) {
        children[childIndex] = childNode;
        childAddresses[childIndex] = address;
    }

    string search(int key) {
        BPlusPageNode* node = this;

        while (!node->isLeaf) {
            node = node->getChild(key);
        }

        return node->get(key);
    }

private:
    int indexOfChild(int key) {
        for (int i = 0; i < keys.size(); i++) {
            if (key < keys[i]) {
                return i;
            }
        }
        return keys.size();
    }

    int getChildAddress(int key) { 
        return childAddresses[indexOfChild(key)]; 
    }

    BPlusPageNode* getChild(int key) { 
        return children[indexOfChild(key)]; 
    }

    string get(int key) {
        int index = -1;

        for (int i = 0; i < keys.size(); ++i) {
            if (keys[i] == key) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            return "NOT_FOUND";
        }

        return values[index];
    }
};

class BPlusTree {
public:
    BPlusTree(int _degree = 4) {
        root = new BPlusNode(nullptr, true, nullptr, nullptr);
        degree = _degree > 2 ? _degree : 2;
    }

    BPlusNode *root;
    int degree;
    queue<BPlusNode> nodeQueue;

    // Function to find the leaf node given a key
    BPlusNode *findLeaf(int key) {
        BPlusNode *node = root;
        while (!node->isLeaf) {
            node = node->getChild(key);
        }
        return node;
    }

    // Function to search for a given key or Employee Id in this context
    string get(int key) {
        return findLeaf(key)->get(key); 
    }

    // Function to insert the key and value to the appropriate leaf node
    void set(int key, string value) {
        BPlusNode *leaf = findLeaf(key);
        leaf->set(key, value);

        if (leaf->keys.size() == degree) {
            insert(leaf->splitLeaf());
        }
    }

    // Function to insert the results of splitLeaf() into the tree
    void insert(tuple<int, BPlusNode *, BPlusNode *> result) {
        int key = std::get<0>(result);
        BPlusNode *left = std::get<1>(result);
        BPlusNode *right = std::get<2>(result);
        BPlusNode *parent = right->parent;

        if (parent == nullptr) {
            left->parent = right->parent = root =
                new BPlusNode(nullptr, false, nullptr, nullptr);
            root->keys = {key};
            root->children = {left, right};
            return;
        }

        parent->setChild(key, {left, right});

        if (parent->keys.size() == degree) {
            insert(parent->splitInternal());
        }
    }

    // Read csv file and add records to the index_file and data_file
    void createFromFile(string csvFileName) {
        fstream csvFile(csvFileName);
        string line;

        ofstream dataFile("DataFile");
        
        char* page = new char[4096];
        int offset = 0;
        int globalOffset = 0;
        int index = 0;

        if (csvFile.is_open()) {
            while (std::getline(csvFile, line)) {
                Record record(splitLine(line));
                int recordSize = calculateRecordSize(record) + 4;

                // If the page size is more than 4096 bytes, write the page content to relation file
                // And reset the page buffer and offset
                if (offset + recordSize > 4096) {
                    dataFile.write(page, offset);
                    
                    page = new char[4096];
                    offset = 0;
                }

                writeRecordToPage(record, page, offset);

                // Create a string out of the start and end addresses of a record in data file
                string address = to_string(globalOffset) + ":" + to_string(globalOffset + recordSize);
                globalOffset += recordSize;
                set(record.id, address);
            }

            // Serialize the B+tree index
            this->serializeUsingBFS("EmployeeIndex");

            if (offset > 0) {
                dataFile.write(page, 4096);
            }
        }

        delete[] page;
        csvFile.close();
        dataFile.close();
    }

    // Function to serialize the B+tree index and write it to an index file
    // NOT BEING USED
    void serialize(const std::string& filename) {
        std::stringstream ss;
        std::ofstream outfile(filename);

        ss << root->serialize();

        string serialized_tree = ss.str();
        outfile << serialized_tree;
        outfile.close();
    }

    // Function to deserialize the B+tree index from the index file
    // NOT BEING USED
    void deserialize(const std::string& filename) {
        std::ifstream infile(filename);
        std::stringstream ss;
        ss << infile.rdbuf();

        root = new BPlusNode();
        root->parent = nullptr;
        root->deserialize(ss);
    }

    // Function to serialize the B+tree index and write it to an index file using BFS
    void serializeUsingBFS(const std::string& filename) {
        std::stringstream ss;
        std::ofstream outfile(filename);

        nodeQueue.push(*root);

        while (nodeQueue.size() > 0) {
            BPlusNode node = nodeQueue.front();
            nodeQueue.pop();
            
            node.serializeUsingBFS(ss);

            for (int c = 0; c < node.children.size(); c++) {
                nodeQueue.push(*node.children[c]);
            }
        }

        string serialized_tree = ss.str();
        outfile << serialized_tree;
        outfile.close();
    }

    // Function to deserialize the B+tree index from the index file using BFS
    BPlusPageNode* deserializeUsingBFS(const std::string& filename) {
        std::ifstream infile(filename);
        std::stringstream ss;
        ss << infile.rdbuf();

        vector<BPlusNode*> listOfBPlusNodes;

        while (!ss.eof()) {
            BPlusNode* node = new BPlusNode();
            node->deserializeUsingBFS(ss, listOfBPlusNodes);
        }
        
        root = listOfBPlusNodes[0];
        root->parent = nullptr;
        int pointer = 1;

        queue<BPlusNode*> desQueue;
        desQueue.push(root);
        
        BPlusPageNode* pageRootNode = new BPlusPageNode();
        queue<BPlusPageNode*> prnQueue;
        prnQueue.push(pageRootNode);

        while (desQueue.size() > 0) {
            BPlusNode* node = desQueue.front();
            desQueue.pop();

            BPlusPageNode* prNode = prnQueue.front();
            prnQueue.pop();

            prNode->set(*node);

            if (node->isLeaf) {
                prNode->values = node->values;
            }
            
            // Declare two variables for prev and next pointers to link all the leaves
            for (int i = 0; i < node->children.size(); i++) {
                node->children[i] = listOfBPlusNodes[pointer++];
                node->children[i]->parent = node;
                desQueue.push(node->children[i]);

                BPlusPageNode* childNode = new BPlusPageNode();
                childNode->set(*node->children[i]);
                prNode->setChild(childNode, node->children[i]->nodeAddress, i);
                prnQueue.push(childNode);
            }
        }

        return pageRootNode;
    }

    // Calculate the size of record object
    int calculateRecordSize(Record record) {
        return 8 + record.name.length() + record.bio.length() + 8;
    }

    // Write a record to the page
    void writeRecordToPage(Record record, char* page, int& offset) {
        // ID
        memcpy(page + offset, std::to_string(record.id).c_str(), 8);
        offset += 8;
        memcpy(page + offset, ",", 1);
        offset += 1;

        // Name
        memcpy(page + offset, record.name.c_str(), record.name.length());
        offset += record.name.length();
        memcpy(page + offset, ",", 1);
        offset += 1;

        // Bio
        memcpy(page + offset, record.bio.c_str(), record.bio.length());
        offset += record.bio.length();
        memcpy(page + offset, ",", 1);
        offset += 1;

        // Manager Id
        memcpy(page + offset, std::to_string(record.manager_id).c_str(), 8);
        offset += 8;
        memcpy(page + offset, "\n", 1);
        offset += 1;
    }
};