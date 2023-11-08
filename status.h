#include <iostream>
#include <dirent.h>
#include <filesystem>
#include <string>
#include <string.h>
#include <bits/stdc++.h>
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
#include <openssl/sha.h>

namespace status
{
    using namespace std;
    namespace fs = std::filesystem;
    string vcspath1 = "./.vcs/tracked_current.txt";
    string vcspath2 = "./.vcs/tracked_history.txt";
    map<string, string> previousSHA;            // <filename, sha1_value> of files in staging area
    map<string, string> currSHA;                // <filename, sha1_value> of files in cwd area
    map<string, string> currTrackedSHA;


    /* Given the file name path, just get the file name */
    string getFileName(string fileName)
    {
        string cwd = fs::current_path();
        // cout << cwd;
        return fileName.substr(cwd.size() + 1, fileName.size());
    }


    /* Read the file in 512KB chunks and find the SHA-1 value for the entire file ;
       Now store : fileLoc + "$" + fullFileSHA1 + "\n"   in   ./.vcs/tracked_current.txt file
       Seems like tracked_current.txt file in .vcs directory is simulating the staging area
    */
    void getCurrentFileSHA(string fileLoc)
    {

        unsigned char completeFileSHA[SHA_DIGEST_LENGTH];
        FILE *inFile = fopen(fileLoc.c_str(), "rb");
        SHA_CTX shaContext;
        int bytes;
        unsigned char data[524288];

        if (inFile == NULL)
        {
            cout << "Cannot open " << fileLoc << endl;
            return;
        }

        // vector<string> chunkSHA1;
        SHA1_Init(&shaContext);
        while ((bytes = fread(data, 1, 524288, inFile)) != 0)
        {
            SHA1_Update(&shaContext, data, bytes);
        }
        SHA1_Final(completeFileSHA, &shaContext);
        fclose(inFile);
        stringstream ss;
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        {
            ss << hex << setw(2) << setfill('0') << (int)completeFileSHA[i];
        }
        string fullFileSHA1 = ss.str();
        // cout << fileLoc << " sha is " << fullFileSHA1 << endl;
        currSHA[fileLoc] = fullFileSHA1;
    }


    // get previous files SHA from version.info
    void getPrevFilesSHA()
    {

        string line;

        ifstream myfile;
        myfile.open(vcspath2);

        if (!myfile.is_open())
        {

            perror("Error open");
            return;
        }

        // get files SHA from tracked_history
        while (getline(myfile, line))
        {
            string fileData = line;
            int pos = fileData.find("$");

            string fileLoc = fileData.substr(0, pos);
            string fileSHA = fileData.substr(pos + 1);
            // cout << fileLoc << " " << fileSHA << endl;
            char resolved[2000];
            realpath(fileLoc.c_str(), resolved);
            string abs_path = string(resolved);
            previousSHA[abs_path] = fileSHA;
        }
        myfile.close();

        ifstream myfile1;
        // cout << "going for path1 ?" << endl;
        myfile1.open(vcspath1);
        if (!myfile1.is_open())
        {
            perror("Error open");
            return;
        }
        // get files SHA from tracked_current
        while (getline(myfile1, line))
        {
            // cout << "COMING INSIDE";

            string fileData = line;
            int pos = fileData.find("$");
            string fileLoc = fileData.substr(0, pos);
            string fileSHA = fileData.substr(pos + 1);
            char resolved[2000];
            realpath(fileLoc.c_str(), resolved);
            string abs_path = string(resolved);
            // cout << fileLoc << " " << fileSHA << endl;
            previousSHA[abs_path] = fileSHA;
            // cout << "Adding SHA to " << abs_path << endl;
            currTrackedSHA[abs_path] = fileSHA;
        }

        myfile1.close();
    }


    /* Traverse all files in cwd recursively and store < curr_file, it's_sha1_value >in the corresponding map */
    void traverseFiles(string cwd)
    {
        // cout << "+++++++++++++++++++++++++++++++++++++++" << endl;
        // get current files SHA
        DIR *folder = opendir(cwd.c_str());
        if (folder == NULL)
        {
            perror("Error open");
            return;
        }
        // reading files in a directory
        struct dirent *file = readdir(folder);
        while (file != NULL)
        {
            if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0 || strcmp(file->d_name, ".vcs") == 0 || strcmp(file->d_name, ".git") == 0 || strcmp(file->d_name, ".vscode") == 0 || strcmp(file->d_name, "main.cpp") == 0 || strcmp(file->d_name, "add.h") == 0 || strcmp(file->d_name, "commit.h") == 0 || strcmp(file->d_name, "diff.h") == 0 || strcmp(file->d_name, "status.h") == 0 || strcmp(file->d_name, "rollback.h") == 0 || strcmp(file->d_name, "vcs") == 0){

            }
            else{
                string filepath = cwd + "/" + file->d_name;
                string new_path = "./.vcs/0/" + string(file->d_name);
                if (file->d_type == DT_DIR)
                {
                    // cout << "Directory" << filepath << endl;
                    traverseFiles(filepath);
                }
                else
                {
                    getCurrentFileSHA(filepath);
                }
            }
            file = readdir(folder);
        }
        closedir(folder);
    }


    /* Compare prev sha map and curr sha map and think of all cases and figure out untracked, modofied, deleted, staged files */
    void git_status(map<string, string> &previous_sha, map<string, string> &current_sha)
    {
        string un_tracked = "Untracked Files:\n";
        string deleted_file = "Deleted Files:\n";
        string modified_file = "Modified Files:\n";
        string tracked_file = "Staged Files:\n";
        string name_of_un = "";
        string name_of_del = "";
        string name_of_mod = "";
        string name_of_track = "";

        if (previous_sha.size() != 0)
        {
            // cout << "Inside" << endl;
            for (auto i : previous_sha)
            {
                // cout << "file path is:--> " << i.first << endl;
                if (current_sha.count(i.first) > 0)
                {
                    if (current_sha[i.first] == previous_sha[i.first])
                    {
                        // cout << "Here--->>" << i.first << endl;
                        //  name_of_track += i.first + "\n";
                    }
                    else                                            //this file is different from prev version => modified
                    {
                        name_of_mod += "\t modified:   " + (getFileName(i.first));
                        name_of_mod += "\n";
                        // cout << "Modified file-->>" << name_of_mod;
                    }
                }   
                else                                                //this file is not in cwd => it is deleted
                {
                    name_of_del += "\t deleted:   " + (getFileName(i.first));
                    name_of_del += "\n";
                }
            }
        }

        for (auto i : current_sha)
        {
            if (previous_sha.count(i.first) == 0)                  //this file was not tracked previously
            {
                name_of_un += "\t untracked:   " + (getFileName(i.first));
                name_of_un += "\n";
            }
        }

        for (auto i : currTrackedSHA)
        {

            name_of_track += "\t staged:   " + (getFileName(i.first));
            name_of_track += "\n";
        }

        string output = "";
        if (name_of_del.size() == 0 && name_of_un.size() == 0 && name_of_track.size() == 0 && name_of_mod.size() == 0)
        {
            cout << "\033[0;36m";
            cout << "Working directory clean, nothing to commit\n";
            cout << "\033[0m";
        }
        else
        {

            cout << "\033[1;32m";
            cout << "\033[4;32m";
            cout << tracked_file<< endl;
            cout << "\033[0m";

            cout << "\033[0;32m";
            cout << name_of_track<< endl;
            cout << "\033[0m";

            cout << "\033[1;36m";
            cout << "\033[4;36m";
            cout << un_tracked<< endl;
            cout << "\033[0m";

            cout << "\033[0;36m";
            cout << name_of_un<< endl;
            cout << "\033[0m";

            cout << "\033[1;31m";
            cout << "\033[4;31m";
            cout << deleted_file<< endl;
            cout << "\033[0m";

            cout << "\033[0;31m";
            cout << name_of_del<< endl;
            cout << "\033[0m";

            cout << "\033[1;33m";
            cout << "\033[4;33m";
            cout << modified_file<< endl;
            cout << "\033[0m";

            cout << "\033[0;33m";
            cout << name_of_mod<< endl;
            cout << "\033[0m";


            // output += tracked_file;
            // output += name_of_track;
            // output += "\n";
            // output += un_tracked;
            // output += name_of_un;
            // output += "\n";
            // output += deleted_file;
            // output += name_of_del;
            // output += "\n";
            // output += modified_file;
            // output += name_of_mod;
        }

        // cout << output;
    }

    /* just like git status, it's implementation is the above function */
    void vcsCmndStatus()
    {
        
        getPrevFilesSHA();                          //compute prev version files SHA values and fill in the corresponding map
        string cwd = fs::current_path();
        traverseFiles(cwd);
        // cout << "Current SHA vals" << endl;
        // for(auto itr = currSHA.begin();itr != currSHA.end();itr++)
        //     cout << itr->first << " " << itr->second << endl;

        // cout << "Previous SHA vals" << endl;
        // for(auto itr = previousSHA.begin();itr != previousSHA.end();itr++)
        //     cout << itr->first << " " << itr->second << endl;

        git_status(previousSHA, currSHA);
    }

}

/*
    Summary of implementation of status command : 
    In the curr directory, when we do git status : 
        1. Figure out all the files and directories recursively in the cwd, Find their current sha-1 values
        2. Go to the staging area and figure out it's contents and their sha-1 values
        3. Now compare both the prevSHA map and currSHA map and find out which files are modified, deleted, untracked, staged
        4. You need to check different cases to do that.

*/