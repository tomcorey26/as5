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
    int fileIdx;
    int orderIdx;

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

int main(int argc, const char *argv[])
{
    int distCount = atoi(argv[1]);
    int distId = atoi(argv[2]);
    const char *dataRootPath = argv[3];

    vector<string> fileName = getFiles(dataRootPath);

    processData(distCount, distId, fileName);

    return 0;
}