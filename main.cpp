#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <filesystem>
#include <string>

// Takes a path and a fileName, and returns the path if that path points to the given file
std::string compareLocationToFileName (std::string fileName, std::string filePath, bool caseInsensitive) {

    // Extracts the filename from the filePath
    auto lastIndexOfSlash = filePath.find_last_of("/");
    auto pathFileName = filePath.substr(lastIndexOfSlash + 1);

    // If search is case insensitive, converts every character in both strings to lowercase
    if(caseInsensitive) {
        for(auto& c : fileName) { c = tolower(c); }
        for(auto& c : pathFileName) { c = tolower(c); }
    }

    // If names match, return the path
    if(fileName == pathFileName)
        return filePath;

    // Otherwise return an empty string
    return "";
}

// Searches the given path for a given file
std::string searchFileSystem(std::string locationPath, std::string fileName, bool recursive, bool caseInsensitive) {

    // Performs a recursive search if -R option is set, does a non-recursive one if it is not
    if(recursive) {
        //Loops each path in each directory recursively
        for (const auto & entry : std::filesystem::recursive_directory_iterator(locationPath)) {

            // Checks if path points to file
            auto foundFilePath = compareLocationToFileName(fileName, entry.path().string(), caseInsensitive);

            // If path points to file, foundFilePath is that path => length is bigger than 0, otherwise foundFilePath is an empty string => length is 0
            if(foundFilePath.length())
                return foundFilePath;
        }
    }
    else {
        // Exactly the same logic as above, only non-recursively
        for (const auto & entry : std::filesystem::directory_iterator(locationPath)) {

            auto foundFilePath = compareLocationToFileName(fileName, entry.path().string(), caseInsensitive);

            if(foundFilePath.length())
                return foundFilePath;
        }
    }

    return "";
}

int main(int argc, char* args[]) {
    int opt; // used to parse options
    bool recursive = false; // true if option is set
    bool caseInsensitive = false; // true if option is set

    // used to determine at which index the searchLocation path begins
    // with no options set, index = 1, otherwise it gets increased with each set option
    int searchLocationIndex = 1; 
    
    // Parses options (option begins with "-", then a letter, allowed are "-R" and "-i")
    while((opt = getopt(argc, args, "Ri")) != EOF) 
    { 
        searchLocationIndex++;

        switch(opt) 
        { 
            case 'i': 
                caseInsensitive = true;
                break;
            case 'R': 
                recursive = true;
                break; 
        } 
    } 

    // After options are parsed, searchLocation is the next argument after that, meaning searchLocationIndex now points to the correct index
    auto searchLocation = args[searchLocationIndex];

    // Loops all arguments after searchLocation (which should all be fileNames)
    for(int fileNameIndex = searchLocationIndex + 1; fileNameIndex < argc; fileNameIndex++) {

        // If file is found, foundFilePath is the path where that file exists, otherwise it is an empty string ""
        auto foundFilePath = searchFileSystem(searchLocation, args[fileNameIndex], recursive, caseInsensitive);

        if(foundFilePath.length())
            std::cout << args[fileNameIndex] << ": " << foundFilePath << std::endl;
        else
            std::cout << args[fileNameIndex] << ": Not found" << std::endl;

    }

	return 0;
}
