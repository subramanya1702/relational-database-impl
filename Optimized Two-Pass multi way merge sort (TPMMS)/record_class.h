#include <iostream>
#include <vector>
#include <string>
#include <bits/stdc++.h>

using namespace std;

class Record {
public:
    struct EmpRecord {
        int eid;
        string ename;
        int age;
        double salary;
    }emp_record;

    Record() {}

    Record(vector<std::string> fields) {
        emp_record.eid = stoi(fields[0]);
        emp_record.ename = fields[1];
        emp_record.age = stoi(fields[2]);
        emp_record.salary = stod(fields[3]);
    }

    string toString() const {
        string recordString = to_string(emp_record.eid) + "," + 
                                emp_record.ename + "," +
                                to_string(emp_record.age) + "," +
                                to_string(emp_record.salary);
        return recordString;
    }
};