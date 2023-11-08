#include <iostream>
#include <dirent.h>
#include <filesystem>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <string>
#include <bits/stdc++.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utility>
// using namespace std;

namespace rollback
{

    using namespace std;
    namespace fs = std::filesystem;

    string vcspath = "./.vcs";
    string version_path = vcspath;


    /*  */
    bool check_if_dir_struct_exists(string check_path, string version_path)
    {

        string cwd = fs::current_path();
        // cout<<"Check "<<check_path<<endl;
        // char resolved[3000];
        // realpath(check_path.c_str(), resolved);
        string abs_path = check_path;
        // cout << "Absolute Path: " << abs_path << endl;
        vector<string> directories_in_path;
        string temp;
        // version_path=version_path.substr(0,version_path.size()-1);

        if (!fs::exists(check_path))
        {
            return false;
        }

        for (int i = 0; i < (int)abs_path.size(); i++)
        {
            if (abs_path[i] != '/')
            {
                temp += abs_path[i];
            }
            else
            {
                directories_in_path.push_back(temp);
                temp = "";
            }
        }
        directories_in_path.push_back(temp);
        string temp_path = version_path;
        string rm_path = ".";
        // cout << "Version path : " << version_path << endl;
        for (auto i : directories_in_path)
        {
            if (i == ".")
                continue;

            temp_path = temp_path + "/" + i;
            rm_path = rm_path + "/" + i;

            if (fs::exists(temp_path))
            {
                // cout << "This exist in vcs : " << temp_path << endl;
                // continue;
            }
            else
            {
                string remove_command = "rm -r " + rm_path;
                system(remove_command.c_str());
                // cout << remove_command << endl;
                // break;
                return true;
            }
        }
        // cout << "End of removee ir" << endl;
        return false;
    }
    
    
    /* create a directory with rwx permissions to ugo at the specified path */
    int createdir(string path1)
    {
        int check;
        check = mkdir(path1.c_str(), 0777);
        if (check == 0)
            return 1;
        else
            return -1;
    }


    /* when given a path to directory, create a similar structure in cwd as in the given path */
    void create_dir_structure(string check_path)
    {
        string cwd = fs::current_path();
        // cout << check_path << endl;
        // char resolved[3000];
        // realpath(check_path.c_str(), resolved);
        string abs_path = check_path;
        // cout << "Absolute Path: " << abs_path << endl;
        vector<string> directories_in_path;
        string temp;
        for (int i = 0; i < (int)abs_path.size(); i++)
        {
            if (abs_path[i] != '/')
            {
                temp += abs_path[i];
            }
            else
            {
                directories_in_path.push_back(temp);
                temp = "";
            }
        }
        string temp_path = cwd;
        // cout << cwd << endl;
        for (auto i : directories_in_path)
        {
            // cout << "Here " << i << endl;
            temp_path = temp_path + "/" + i;
            if (fs::exists(temp_path))
            {
                continue;
            }
            else
            {
                string create_command = "mkdir " + temp_path;
                system(create_command.c_str());
            }
        }
    }


    /* Get all files recursively in the path/dirName directory and store it in the given vector */
    void getFileRecursive(vector<string> &st, string path, string dirName, int mode)
    {

        DIR *dir;
        struct dirent *sd;
        // cout << "Inside dir: ";
        string fullPath = path + dirName;
        // cout << fullPath << endl;
        // string version_path = vcspath + "/" + fullPath;

        // cout << fullPath << endl;
        dir = opendir(fullPath.c_str());
        if (fs::is_empty(fullPath) && mode)
        {
            if (check_if_dir_struct_exists(fullPath, version_path))
            {
                // cout << fullPath << endl;
                // cout << "True" << endl;
                return;
            }
        }
        // sd = readdir(dir);
        while ((sd = readdir(dir)) != NULL)
        {
            string currFile = sd->d_name;
            if (currFile == ".." || currFile == "." || currFile == ".vcs" || currFile == "add.h" || currFile == "commit.h" || currFile == "status.h" || currFile == "diff.h" || currFile == "vcs" || currFile == ".git" || currFile == ".vscode" || currFile == "main.cpp" || currFile == "rollback.h")
                continue;
            else
            {
                struct stat sfile;
                currFile = path + dirName + "/" + sd->d_name;
                stat(currFile.c_str(), &sfile);
                // cout << currFile << endl;

                if (fs::is_directory(currFile.c_str()))
                {
                    // cout << "recur " << dirName + "/" + sd->d_name << endl;
                    getFileRecursive(st, path, dirName + "/" + sd->d_name, mode);
                }
                else
                    st.push_back(dirName + "/" + sd->d_name);
            }
        }
        // cout << "going out\n";
    }


    /* go to the previous commit version */
    void go_to_previous(string version_path, string curr_path)
    {
        DIR *dir1, *dir2, *dir;
        struct dirent *sd;

        // cout << "Version Path : " << version_path << endl;
        // cout << "Current path : " << curr_path << endl;
        dir1 = opendir(version_path.c_str());
        dir2 = opendir(curr_path.c_str());

        if (dir1 == NULL || dir2 == NULL)
        {
            // cout << "Here" << endl;
            cout << "'.vcs' folder corrupted.\n";
            return;
        }

        vector<string> old, latest; // stores path of all the files / directories
        dir = dir2;
        while ((sd = readdir(dir)) != NULL)
        {
            string currFile = sd->d_name;
            // string fileDetails = getFileDetails(dir_to_search + '/'+sd->d_name);
            if (currFile == ".." || currFile == "." || currFile == ".vcs" || currFile == "add.h" || currFile == "commit.h" || currFile == "status.h" || currFile == "diff.h" || currFile == "vcs" || currFile == ".git" || currFile == ".vscode" || currFile == "main.cpp" || currFile == "rollback.h")
                continue;
            else
            {
                struct stat sfile;
                currFile = curr_path + sd->d_name;
                stat(currFile.c_str(), &sfile);

                if (fs::is_directory(currFile.c_str()))
                    getFileRecursive(latest, curr_path, sd->d_name, 1);
                else
                    latest.push_back(sd->d_name);
            }
            // cout << "latest " << sd->d_name << endl;
        }

        dir = dir1;
        while ((sd = readdir(dir)) != NULL)
        {
            string currFile = sd->d_name;
            // string fileDetails = getFileDetails(dir_to_search + '/'+sd->d_name);
            if (currFile == ".." || currFile == "." || currFile == ".vcs" || currFile == "add.h" || currFile == "commit.h" || currFile == "status.h" || currFile == "diff.h" || currFile == "vcs" || currFile == ".git" || currFile == ".vscode" || currFile == "main.cpp")
                continue;
            else
            {
                struct stat sfile;
                currFile = version_path + currFile;
                stat(currFile.c_str(), &sfile);

                if (fs::is_directory(currFile.c_str()))
                {
                    // cout << "Before The Old Dirctory Name is : "<< sd->d_name << endl;
                    getFileRecursive(old, version_path, sd->d_name, 0);
                    // cout << "After The Old Dirctory Name is : "<<sd->d_name << endl;
                }

                else
                    old.push_back(sd->d_name);
            }
            // cout << "old " << sd->d_name << endl;
        }

        // for (auto it : old)
        // {
        //     cout << "old : " << it << endl;
        // }

        // for (auto it : latest)
        // {
        //     cout << "current : " << it << endl;
        // }

        for (auto it : latest)
        {
            string add_patch = it + ".patch";

            auto prev_itr = find(old.begin(), old.end(), add_patch);
            if (prev_itr != old.end())
            {
                string command = "patch " + it + " " + version_path + add_patch;
                system(command.c_str());
                old.erase(prev_itr);
            }

            else
            {
                string command = "rm " + it;

                // cout<<"Here Above"<<endl;
                if (check_if_dir_struct_exists(it, version_path))
                {
                    // cout << "HEre : " << command << endl;
                    // system(command.c_str());
                }

                // cout<<"Here Above"<<endl;
                // ifstream ud(it, ios::ate | ios::binary);

                // cout<<"Here"<<endl;
                // if (ud.good())
                // {
                // cout << command << endl;

                // }
                // else
                // {
                //     cout << "Ignoring the file presented in deleted folder " << endl;
                // }
            }
        }

        for (auto it : old)
        {
            create_dir_structure(it);
            string command = "cp " + version_path + it + " " + "./" + it;
            system(command.c_str());
        }
    }


    /* go back to the specified version number */
    void rollback(string version_no, string curr_path, int total_commits)
    {
        // string version_path = "./.vcs/" + version_no + "/";
        // version_path = vcspath + "/" + version_no + "/";

        int count = total_commits - stoi(version_no) - 1; // Number of times to rollback
        int i = 1;
        while (count > 0)
        {

            cout << "Rolling Backing to : " << total_commits - 1 - i << endl;
            // cout<<"----------------------------------------------------------------------------------"<<endl;
            version_path = vcspath + "/" + to_string(total_commits - 1 - i) + "/";
            go_to_previous(version_path, curr_path);
            count--;
            i++;
            // int vno = stoi(version_no);
            // vno--;
            // version_no = to_string(vno);
        }

        return;
    }

}