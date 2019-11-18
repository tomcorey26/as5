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
 * Version 3
 *  Proccesses and exec calls 
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
#include <sys/wait.h>

using namespace std;
void writeDistributorOutputToFile(vector<vector<int>> assignedIndexes, int idx);

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
 * Combines all code chunks created by data proccessing into one file
 *
 * @param dataFileCount total code chunk files 
 * @param distCount path to outfile 
 */
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
            // args size, start idx, end idx, data root path
            string distCount = to_string(server.distributors.size());
            string startIdx = to_string(server.distributors[k].startIdx);
            string endIdx = to_string(server.distributors[k].endIdx);
            string distId = to_string(server.distributors[k].id);
            const char *args[] = {(char *)"./dist", (char *)distCount.c_str(), (char *)startIdx.c_str(), (char *)endIdx.c_str(), (char *)dataRootPath, (char *)distId.c_str(), (char *)NULL};
            execvp(args[0], (char *const *)args);
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
            //args dist count , dist id , outpath
            string distCount = to_string(server.distributors.size());
            string distId = to_string(server.distributors[i].id);
            const char *args[] = {(char *)"./proc", (char *)distCount.c_str(), (char *)distId.c_str(), (char *)dataRootPath, (char *)NULL};
            execvp(args[0], (char *const *)args);
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
