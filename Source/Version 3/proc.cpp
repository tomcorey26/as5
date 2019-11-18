/** \file proc.cpp
 * Version 3
 * process files 
 */

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

using namespace std;

struct fileData
{
    int fileIdx; //!< Actual index of the file pertaining to the vector it is held in
    int orderIdx; //!< The order the text of the file has to be in

    bool operator()(fileData i, fileData j) { return (i.orderIdx < j.orderIdx); }
} fileD;

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

/**
 * Gets all the indexes that it has been assigned for processing 
 * 
 * @param distCount number of distributors
 * @param files vector of file paths to data files
 * @param distIdx index of parent distributor
 */ 
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

/**
 * Writes code chunks to file
 * 
 * @param distCount number of distributors
 * @param files vector of file paths to data files
 * @param distIdx index of parent distributor
 */ 
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

int main(int argc, const char *argv[])
{
    int distCount = atoi(argv[1]);
    int distId = atoi(argv[2]);
    const char *dataRootPath = argv[3];

    vector<string> fileName = getFiles(dataRootPath);

    processData(distCount, distId, fileName);

    return 0;
}