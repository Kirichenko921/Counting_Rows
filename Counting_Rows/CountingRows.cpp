#include <iostream>
#include <fstream>
#include <filesystem>
#include "ThreadPool.h"
namespace fs = std::filesystem;

std::atomic <unsigned > allQuantity = 0; // the number of lines of all files in the directory
// Function that determines the number of lines in the file.
void CountLinesInFile(std::string filename)
{

    const char* outFilenamePtr = filename.c_str();
    std::ifstream currentFile(outFilenamePtr, std::ios::in);
    if (!currentFile) //if the file has not opened
    {
        return;
    }
    unsigned int count = 0;      // row counter
    char buffer[1000]; // buffer for saving a single line
    while (!currentFile.eof()) // we read the file until it ends
    {
        count++; //Increasing the row counter
        currentFile.getline(buffer, 1000); //Reading one line into the buffer
    }
    currentFile.close();
    allQuantity += count;
}

void fileSearch(std::string& pathDirectory) // Search and processing of files in the selected directory
{
    RequestHandler rh;
    for (const auto& entry : fs::directory_iterator(pathDirectory))
    {
        std::filesystem::path outFilename = entry.path();
        std::string outFilenameStr = outFilename.string();
            rh.pushRequest(CountLinesInFile, outFilenameStr);
    }
}


int main()
{
    std::cout << "Enter the path to the folder" << std::endl;
    std::string path;
    std::cin >> path;
    fileSearch(path);
    std::cout << "\tRow count < " << path << " >\n\n\t\t" << allQuantity << std::endl;


    return 0;
}