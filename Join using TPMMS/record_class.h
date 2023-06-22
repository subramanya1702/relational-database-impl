#include <iostream>
#include <vector>
#include <string>
#include <bits/stdc++.h>

using namespace std;

enum relationType { DEPT, EMP };

class Record {
public:
    struct EmpRecord {
        int eid;
        string ename;
        int age;
        double salary;
    } emp_record;

    struct DeptRecord {
        int did;
        string dname;
        double budget;
        int managerid;
    } dept_record;

    Record() {}

    Record(vector<std::string> fields, int type) {
        if (type == DEPT) {
            dept_record.did = stoi(fields[0]);
            dept_record.dname = fields[1];
            dept_record.budget = stod(fields[2]);
            dept_record.managerid = stoi(fields[3]);
        } else if (type == EMP) {
            emp_record.eid = stoi(fields[0]);
            emp_record.ename = fields[1];
            emp_record.age = stoi(fields[2]);
            emp_record.salary = stod(fields[3]);
        }
    }
    
    string toString(int type) const {
        if (type == DEPT) {
            return to_string(dept_record.did) + "," + 
                                dept_record.dname + "," +
                                to_string(dept_record.budget) + "," +
                                to_string(dept_record.managerid); 
        } else if (type == EMP) {
            return to_string(emp_record.eid) + "," + 
                                emp_record.ename + "," +
                                to_string(emp_record.age) + "," +
                                to_string(emp_record.salary);
        } else {
            return "";
        }
    }

};