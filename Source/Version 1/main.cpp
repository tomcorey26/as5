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
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <string>

using namespace std;

struct distributor
{
    int startIdx;
    int endIdx;
    int fileCount;
    std::vector<string> files;
};

struct Server
{
    int childrenCount;
    std::vector<distributor> distributors;
};

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
            fileName.push_back(string(entry->d_name));
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
        int remainder = fileCount % distCount;
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
        printf("distIdx: %d , start: %d, end: %d \n", i, start, end);
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

    server.distributors = assignDistributers(fileName.size(), server.childrenCount);
    cout << fileName.size() << " files found" << endl;
    // for (int k = 0; k < fileName.size(); k++)
    //     cout << "\t" << fileName[k] << endl;

    return 0;
}
