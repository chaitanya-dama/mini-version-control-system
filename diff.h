#include <iostream>
#include <dirent.h>
#include <filesystem>
#include <string>
#include <string.h>
// #include <bits/stdc++.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utility>
// using namespace std;

namespace diff
{
    using namespace std;
    namespace fs = std ::filesystem;
    string cwd = fs ::current_path();
    string vcspath = "./.vcs"; // add .vcs


/* Prints the given string in bright red to denote error (using ANSI escape sequences which is a character seq. to format
output in diff. colours and fonts */
    void error_print(string print)
    {
        cout << "\033[1;31m";
        cout << print << endl;
        cout << "\033[0m";
        return;
    }


/* Given a path, check if it corresponds to a directory and the 1st entry also must be a directory ; else return false */
    bool check_directory(string path1)
    {
        DIR *location = opendir(path1.c_str());

        if (location == NULL)
            return false;

        struct dirent *file = readdir(location);
        if (file->d_type == DT_DIR)
            return true;
        else
            return false;
    }


/* Look at the summary */
    void difference_between_two(string file)
    {
        int versionNo;
        string temp;
        ifstream vfile("./.vcs/version.info");              //contains the current version number
        vfile >> temp;
        vfile.close();
        versionNo = stoi(temp);                             //get the current version number

        //path1 => cwd file path ; path2 => staging area file path
        string path1 = "./" + file, path2 = "./.vcs/" + to_string(versionNo) + "/" + file;  

        char resolved[2000];
        realpath(path1.c_str(), resolved);
        path1 = string(resolved);       //get absolute path1

        realpath(path2.c_str(), resolved);
        path2 = string(resolved);       //get absolute path2

        // cout << path1 << " " << path2 << endl;
        if (fs::exists(path1) == false)             //check if file exists in the given cwd
        {
            // cout << "File not present\n";
            error_print("File not present");
            return;
        }
        else if (fs::exists(path2) == false)        //check if file exists in the staging area
        {
            path2 = "./.vcs/" + to_string(versionNo - 1) + "/" + file;      //check in previous version of staging area
            realpath(path2.c_str(), resolved);
            path2 = string(resolved);

            // cout << path2 << endl;
            if (fs::exists(path2) == false)
            {
                cout << "File added recently\n";
                return;
            }
        }

        if ((!check_directory(path1) && !check_directory(path2)) || (check_directory(path1) && check_directory(path2)))
        {                   //if they are only files(ignore directories) => use diff shell command to get the req. difference;
            string command = "diff ";
            command += path1 + " " + path2;
            system(command.c_str());
        }
    }


}

/*
    Summary of implementation of diff command : 
        1. Given the filename, get the prev version of the file from staging area or commited area ( ./.vcs/version_num/file)
        2. Check if the file exists in both the cwd and staging area or previous commit
        3. if yes => use diff shell command to print the req. differences 

*/
