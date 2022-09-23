#include <iostream>
#include <vector>
#include <unistd.h> //Option parsing
#include <filesystem> //File system traversal
#include <wait.h> //Waiting for child processes

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

    bool recursive = false; // true if "-R" is set
    bool caseInsensitive = false; // true if "-i" is set

    std::string searchLocation = ""; //The first string that is not an option is the search location
    std::vector<std::string> filesToSearch; //Every other string that is not an option after the first one is a filename to search

    // Loops arguments to parse them
    // Custom parsing implemented to enable user giving options at any position within the argument list
    // Loop starts at index 1 since the initial program call at index[0] (f.ex. "./myprog") should be ignored
    for(int i = 1; i < argc; i++) {
        std::string argument = args[i];

        //Valid options are "-i" and "-R"
        if(argument == "-i") {caseInsensitive = true; continue;}
        if(argument == "-R") {recursive = true; continue;}
        //If another option is passed, write error message and return EXIT_FAILURE
        if(argument.at(0) == '-') {std::cout << "Unknown option. Options are\n\"-R\" recursive search\n\"-i\" case insensitive search\n\n./myfind [-R] [-i] searchpath filename1 [filename2] ... [filenameN]"; return EXIT_FAILURE;}
        //If searchLocation wasn't set yet, current argument is first non-option, meaning it must be the searchLocation
        if(!searchLocation.length()) {searchLocation = argument; continue;}

        //Every non-option string after searchLocation is a filename to search, so we add it to the vector
        filesToSearch.push_back(argument);
    }

    // Loops all filenames to search
    for(auto const& fileNameToSearch: filesToSearch) {

        // Forks once for every file we are searching for
        auto pid = fork();

        std::string foundFilePath;
        switch(pid) {
            // Error, should not happen
            case -1:
                return EXIT_FAILURE;
            
            // Child process searches file
            case 0:
                // If file is found, foundFilePath is the path where that file exists, otherwise it is an empty string ""
                foundFilePath = searchFileSystem(searchLocation, fileNameToSearch, recursive, caseInsensitive);

                //Prints success message
                if(foundFilePath.length())
                    std::cout << getpid() << ": " << fileNameToSearch << ": " << foundFilePath << std::endl;
                else
                    std::cout << getpid() << ": " << fileNameToSearch << ": Not found" << std::endl;

                // Returns (terminates) child process
                return EXIT_SUCCESS;

            // Parent process does nothing, it simply continues the loop
            default:
                break;
        }

    }

    // Once all forks are done, the parent process waits for all its children to finish

    /// !!!!!!!!!!!!!!!!!!!!!!!!! BUGGED !!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Sometimes the parent process finishes before its children do, no clue why
    // Actually, I'm not even sure if that is what's happening
    // The terminal sometimes just enables user input again, and prints some child-process output slightly later
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    pid_t childpid;
    while((childpid = waitpid(-1, NULL, WNOHANG))) {
        if((childpid == -1) && (errno != EINTR))
            break;
    }

	return EXIT_SUCCESS;
}