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

/** \file main.cpp
 * Version 1 
 * Distributors and processors are functions
 */

#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <algorithm>

using namespace std;
void writeSortedCodeToFile(const char *outFile, string code);

struct distributor
{
    int id; //!< index of distributor
    int startIdx; //!< start idx of assigned files
    int endIdx; //!< end idx of assigned files
    int fileCount; //!< num of file assigned to dist
    std::vector<int> todoList; //!< indexes of files to data process
};

struct Server
{
    int childrenCount; //!< num of distributors
    std::vector<distributor> distributors; //!< vector that holds all dists
};

/**
 * Opens directory with all textfiles and returns a vector with all the filenames
 *
 * @param dataRootPath path to folder that holds all the text files
 * @return the vector of filenames
 */
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

/**
 * Assigns distributors with file ranges that they will handle for
 * distribution
 *
 * @param fileCount Total files in data set directory 
 * @param distCount Amount of distributors
 * @return Distributor struct with ranges assigned 
 */
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

/**
 *  Gets the index of process that is assigned to the given file
 *
 * @param filePath Path to file in data directory 
 * @return Index of process that is assigned to the file 
 */
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

/**
 *  Gets the order the text in the file should be in 
 *
 * @param filePath Path to file in data directory 
 * @return Index of the order the text file should be in 
 */
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

/**
 *  Gets content of file without numbers in the beginning
 *
 * @param filePath Path to file in data directory 
 * @return String of the file content 
 */
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

vector< pair<int, int> > distributeFiles(distributor &dist, vector<string> &files)
{
    // init map that holds distributor/file idx pairs to return
    vector< pair<int, int> > processFilePairs;

    //loop through files in distributor range
    for (int i = dist.startIdx; i <= dist.endIdx; i++)
    {
        //read from file and get the num of process it should be assigned to
        int processNum = getProcessNum(files[i]);
        int fileIdx = i;

        //add processes to current distributors todo list
        // if their assigned process matches current distributor
        if (processNum == dist.id)
        {
            dist.todoList.push_back(fileIdx);
        }
        // if not add to map the distributor id/ fileidx pair
        else
        {
            processFilePairs.push_back(make_pair(processNum, fileIdx));
        }
    }
    return processFilePairs;
}

struct fileData
{
    int fileIdx;
    int orderIdx;

    bool operator()(fileData i, fileData j) { return (i.orderIdx < j.orderIdx); }
} fileD;

string processData(vector<int> &todoList, vector<string> &files)
{

    //loop through todo list
    vector<fileData> sortedTodoList;
    for (int i = 0; i < todoList.size(); i++)
    {
        fileData currfile;
        currfile.fileIdx = todoList[i];
        currfile.orderIdx = getOrderIdx(files[todoList[i]]);
        sortedTodoList.push_back(currfile);
    }

    //sort lists
    sort(sortedTodoList.begin(), sortedTodoList.end(), fileD);

    string fileChunk;
    for (int j = 0; j < sortedTodoList.size(); j++)
    {
        string fileContent = getFileContent(files[sortedTodoList[j].fileIdx]);
        fileChunk = fileChunk + fileContent;
    }

    return fileChunk;
}

void writeSortedCodeToFile(const char *outFile, string code)
{
    ofstream myfile;
    myfile.open(outFile);
    myfile << code;
    myfile.close();
}

int main(int argc, const char *argv[])
{
    const char *processCount = argv[1];
    const char *dataRootPath = argv[2];
    const char *outFile = argv[3];

    // printf("Count: %s, directory: %s,Outfile: %s", processCount, dataRootPath, outFile);

    //init server
    Server server;
    server.childrenCount = atoi(processCount);

    //get file names
    vector<string> fileName = getFiles(dataRootPath);

    //assign ranges to the distributor processes
    server.distributors = assignDistributers(fileName.size(), server.childrenCount);

    // cout << fileName.size() << " files found" << endl;
    // for (int k = 0; k < fileName.size(); k++){
    //     cout << "\t" << fileName[k] << endl;
    //     // cout << "\t" << getOrderIdx(fileName[k]) << endl;
    // }
    //distributors job
    for (int k = 0; k < server.distributors.size(); k++)
    {
        //init map
        vector< pair<int, int> > processFilePairs;
        //distributor function here
        processFilePairs = distributeFiles(server.distributors[k], fileName);
        for (int j = 0; j < processFilePairs.size(); j++)
        {
            //add file pair idex to correspongding distritubtor
            server.distributors[processFilePairs[j].first].todoList.push_back(processFilePairs[j].second);
        }
    }

    //data processing loop
    //the function should return the reorganized string
    // then the server concatanates them together
    string wholeFile;
    for (int i = 0; i < server.distributors.size(); i++)
    {
        string fileChunk = processData(server.distributors[i].todoList, fileName);
        wholeFile = wholeFile + fileChunk;
    }

    //write the completed file to the output file
    //provided as argument
    writeSortedCodeToFile(outFile, wholeFile);

    return 0;
}