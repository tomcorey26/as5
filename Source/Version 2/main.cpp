//
//  main.cpp
//  Directory Operations, C++ version
//
//  Created by Jean-Yves Hervé on 2017-02-15.
//	Edited 2018-02-25, 2019-11-04
//
//	This C++ program takes as argument the path to a folder and
//	outputs the name of all the files found in that folder
//
//	For reference:  file types recognized by dirent
//		#define	DT_UNKNOWN	 0
//		#define	DT_FIFO		 1
//		#define	DT_CHR		 2
//		#define	DT_DIR		 4
//		#define	DT_BLK		 6
//		#define	DT_REG		 8
//		#define	DT_LNK		10
//		#define	DT_SOCK		12
//		#define	DT_WHT		14

#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <sys/wait.h>

using namespace std;
void writeDistributorOutputToFile(vector<vector<int>> assignedIndexes, int idx);

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
//TODO document functions and add doxygen
//TODO fix incorrect process number problem
//TODO Figure out the end of line problem
//TODO test if this works on linux
//TODO figure out how to get rid of spaces in beginning
vector<string> getFiles(const char *dataRootPath)
{
    DIR *directory = opendir(dataRootPath);
    if (directory == NULL)
    {
        cout << "data folder " << dataRootPath << " not found" << endl;
        exit(1);
    }

    struct dirent *entry;
    vector<string> fileName;
    while ((entry = readdir(directory)) != NULL)
    {
        const char *name = entry->d_name;
        if ((name[0] != '.') && (entry->d_type == DT_REG))
        {
            string filePath = string(dataRootPath) + string(entry->d_name);
            fileName.push_back(filePath);
        }
    }
    closedir(directory);

    return fileName;
}

vector<distributor> assignDistributers(int fileCount, int distCount)
{
    int filePerDist = fileCount / distCount;
    int remainder = 0;
    if (fileCount % distCount != 0)
    {
        remainder = fileCount % distCount;
    }

    vector<distributor> distributors;
    int start = 0;
    // minus one bc start at 0
    int end = filePerDist - 1;
    for (int i = 0; i < distCount; i++)
    {
        //init distributor
        distributor dist;
        //evenly spread out remainder
        if (remainder != 0)
        {
            end++;
            remainder--;
        }

        //assign values to dist and add to vector
        // printf("distIdx: %d , start: %d, end: %d \n", i, start, end);
        dist.id = i;
        dist.startIdx = start;
        dist.endIdx = end;
        dist.fileCount = end - start + 1;
        distributors.push_back(dist);

        //change the markers
        start = end + 1;
        end += filePerDist;
    }
    return distributors;
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

int getOrderIdx(string filePath)
{
    ifstream inFile(filePath);
    string procIdx;
    if (inFile.is_open())
    {
        inFile >> procIdx;
        inFile >> procIdx;
    }
    inFile.close();
    return stoi(procIdx);
}

string getFileContent(string filePath)
{
    ifstream inFile(filePath);
    string junk;
    string fileText;
    if (inFile.is_open())
    {
        //remove numbers in file
        inFile >> junk;
        inFile >> junk;
        //get the rest
        getline(inFile, fileText);
    }
    inFile.close();
    return fileText + '\n';
}

void distributeFiles(int distCount, distributor &dist, vector<string> &files)
{
    // init vector and add empty vector arrays representing each distributor
    vector<vector<int>> distAssignedIndexs;
    for (int j = 0; j < distCount; j++)
    {
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
    writeDistributorOutputToFile(distAssignedIndexs, dist.id);
}

void writeDistributorOutputToFile(vector<vector<int>> assignedIndexes, int idx)
{
    string fileName = "distributors/dist" + to_string(idx);
    ofstream file;
    file.open(fileName);

    for (int i = 0; i < assignedIndexes.size(); i++)
    {
        // = to_string(assignedIndexes[i].size());
        string line;
        for (int j = 0; j < assignedIndexes[i].size(); j++)
        {
            line = line + to_string(assignedIndexes[i][j]) + " ";
        }
        file << line << '\n';
        file.flush();
    }
    file.close();
}

struct fileData
{
    int fileIdx;
    int orderIdx;

    bool operator()(fileData i, fileData j) { return (i.orderIdx < j.orderIdx); }
} fileD;

vector<fileData> getIdxArray(int distCount, vector<string> &files, int distIdx)
{

    vector<fileData> sortedTodoList;
    //loop the dist files
    //open each dist file, get array of ints corresponding to which distriubtor this is (loop get line disidx times)
    for (int i = 0; i < distCount; i++)
    {
        string path = "distributors/dist" + to_string(i);
        ifstream inFile(path);
        string fileText;

        //skip lines to get to correct dist vector
        for (int j = 0; j < distIdx; j++)
        {
            getline(inFile, fileText);
        }
        //get correct dist line and assign it to string
        getline(inFile, fileText);

        stringstream dataLine(fileText);
        string number;

        while (getline(dataLine, number, ' '))
        {
            fileData currfile;
            currfile.fileIdx = stoi(number);
            currfile.orderIdx = getOrderIdx(files[currfile.fileIdx]);
            sortedTodoList.push_back(currfile);
        }
        inFile.close();
    }
    sort(sortedTodoList.begin(), sortedTodoList.end(), fileD);
    return sortedTodoList;
}

void processData(int distCount, int distIdx, vector<string> &files)
{

    vector<fileData> sortedTodoList = getIdxArray(distCount, files, distIdx);

    string fileChunk;
    for (int j = 0; j < sortedTodoList.size(); j++)
    {
        string fileContent = getFileContent(files[sortedTodoList[j].fileIdx]);
        fileChunk = fileChunk + fileContent;
    }

    //write file chunk to file
    ofstream myfile;
    string fileName = "codeChunks/code" + to_string(distIdx);
    myfile.open(fileName);
    myfile << fileChunk;
    myfile.close();
}

void writeSortedCodeToFile(const char *outFile, string code)
{
    ofstream myfile;
    myfile.open(outFile);
    myfile << code;
    myfile.close();
}

void writeCombinedFile(int dataFileCount, const char *outFile)
{
    ofstream combined_file;
    combined_file.open(outFile);
    for (int k = 0; k < dataFileCount; k++)
    {
        string currFile = "codeChunks/code" + to_string(k);
        ifstream file(currFile);
        combined_file << file.rdbuf();
        file.close();
    }
    combined_file.close();
}

int main(int argc, const char *argv[])
{
    const char *processCount = argv[1];
    const char *dataRootPath = argv[2];
    const char *outFile = argv[3];

    //init server
    Server server;
    server.childrenCount = atoi(processCount);

    //get file names
    vector<string> fileName = getFiles(dataRootPath);

    // cout << fileName.size() << " files found" << endl;
    // for (int k = 0; k < fileName.size(); k++){
    //     cout << "\t" << fileName[k] << endl;
    //     // cout << "\t" << getOrderIdx(fileName[k]) << endl;
    // }
    //assign ranges to the distributor processes
    server.distributors = assignDistributers(fileName.size(), server.childrenCount);

    //distributors job
    //write process todolist to file
    for (int k = 0; k < server.distributors.size(); k++)
    {
        int p = fork();

        //write processe/file idx pairs to file
        //write process todolist to file
        if (p == 0)
        {
            //args dist
            distributeFiles(server.distributors.size(), server.distributors[k], fileName);
            exit(0);
        }
        else
        {
            wait(NULL);
        }
    }

    //data processing loop
    //the function should return the reorganized string
    // then the server concatanates them together
    for (int i = 0; i < server.distributors.size(); i++)
    {
        int p = fork();

        if (p == 0)
        {
            processData(server.distributors.size(), server.distributors[i].id, fileName);
            exit(0);
        }
        else
        {
            wait(NULL);
        }
    }
    //read from created files and write them
    writeCombinedFile(server.distributors.size(), outFile);

    return 0;
}
