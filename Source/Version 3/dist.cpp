#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <sstream>
#include <unistd.h> 

using namespace std;

struct distributor
{
    int id;
    int startIdx;
    int endIdx;
    int fileCount;
    std::vector<int> todoList;
};

struct Server
{
    int childrenCount;
    std::vector<distributor> distributors;
};

void writeDistributorOutputToFile(vector< vector<int> > assignedIndexes, int idx);
void distributeFiles(int distCount,distributor &dist, vector<string> &files);
int getProcessNum(string filePath);

void writeDistributorOutputToFile(vector< vector<int> > assignedIndexes, int idx) {
    string fileName = "dist" + to_string(idx);
    ofstream file;
    file.open(fileName);
    
    for (int i = 0; i < assignedIndexes.size(); i++) { 
        // = to_string(assignedIndexes[i].size());
        string line;
        for (int j = 0; j < assignedIndexes[i].size(); j++) {
            line = line + to_string(assignedIndexes[i][j]) + " ";
        }
        file << line << '\n';
        file.flush();
    }
    file.close();
    exit(0);
}


void distributeFiles(int distCount,distributor &dist, vector<string> &files)
{
    // init vector and add empty vector arrays representing each distributor
    vector< vector<int> > distAssignedIndexs;
    for (int j = 0; j < distCount; j++) {
        vector<int> empty;
        distAssignedIndexs.push_back(empty);
    }

    //loop through files in distributor range
    for (int i = dist.startIdx; i <= dist.endIdx; i++)
    {
        //read from file and get the num of process it should be assigned to
        int processNum = getProcessNum(files[i]);
        int fileIdx = i;
        distAssignedIndexs[processNum].push_back(i);
    }

    //write vector of ints to file
    writeDistributorOutputToFile(distAssignedIndexs,dist.id);
}

int getProcessNum(string filePath)
{
    ifstream inFile(filePath);
    string procIdx;
    if (inFile.is_open())
    {
        inFile >> procIdx;
    }
    inFile.close();
    return stoi(procIdx);
}

int main(int argc, const char *argv[]) {
  int fart;

  return 0;
}