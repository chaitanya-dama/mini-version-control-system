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
#include <openssl/sha.h>
// using namespace std;

namespace add
{
    //tracked_current, tracked_history
    using namespace std;
    void copydirectory(string path1, string path2);
    void copyfile(string path1, string path2);

    namespace fs = std::filesystem;
    string vcspath = "./.vcs"; // add .vcs
    

    /* Read the file in 512KB chunks and find the SHA-1 value for the entire file ;
       Now store : fileLoc + "$" + fullFileSHA1 + "\n"   in   ./.vcs/tracked_current.txt file
       Seems like tracked_current.txt file in .vcs directory is simulating the staging area
    */
    void calculateFileSHA(string fileLoc)
    {
        unsigned char completeFileSHA[SHA_DIGEST_LENGTH];
        FILE *inFile = fopen(fileLoc.c_str(), "rb");
        SHA_CTX shaContext;
        int bytes;
        // 524288
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
        

        string appendData = fileLoc + "$" + fullFileSHA1 + "\n";
        string cwd = fs::current_path();
        string version;
        ofstream vfile;
        vfile.open("./.vcs/tracked_current.txt", std::ios_base::app); // append instead of overwrite
        vfile << appendData;
        vfile.close();
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
    
    
    /* given a directory path, copy all the files and directories in it to the staging area (./.vcs/versionNum/cwdpath) */
    void copy_version(string path)
    {
        string cwd = fs::current_path();
        cwd += "/"+path;
        string version;
        // string ver=to_string(version);
        // string command1="mkdir  ./.vcs/"+ver;
        // system(command1.c_str());
        // system("touch ./.vcs/version.info");
        ifstream vfile("./.vcs/version.info");
        vfile >> version;
        vfile.close();
        DIR *folder = opendir(cwd.c_str());
        if (folder == NULL)
            return;
        // reading files in a directory
        struct dirent *file = readdir(folder);
        while (file != NULL)
        {
            if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0 || strcmp(file->d_name, ".vcs") == 0 || strcmp(file->d_name, ".git") == 0 || strcmp(file->d_name, ".vscode") == 0 || strcmp(file->d_name, "main.cpp") == 0
            || strcmp(file->d_name, "add.h") == 0 || strcmp(file->d_name, "rollback.h") == 0 || strcmp(file->d_name, "commit.h") == 0 || strcmp(file->d_name, "diff.h") == 0 || strcmp(file->d_name, "status.h") == 0 || strcmp(file->d_name, "vcs") == 0){
                
                // cout <<file->d_name << " ignore" << endl;
            }
            else
            {

                string filepath = cwd + "/" + file->d_name;
                string new_path = "./.vcs/" + version + "/"+path+"/" + string(file->d_name);
                if (file->d_type == DT_DIR)
                {
                    // cout << "Directory " << filepath << endl;

                    // cout<<"directory_detected";
                    // create_dir_structure(new_path,"0");
                    copydirectory(filepath, new_path);
                    // cout << "outof dir\n";
                }

                else
                {
                    // cout << "file " << filepath << endl;
                    // create_dir_structure(new_path,"0");
                    copyfile(filepath, path+"/" + string(file->d_name));
                    // cout << "outof file\n";
                }
            }
            file = readdir(folder);
        }
        closedir(folder);
    }
    
    
    /* when given a path to directory, create a similar structure in staging area also so that we can later copy them directly */
    void create_dir_structure(string check_path, string version)
    {
        string cwd = fs::current_path();
        // fs::vcspath p=vcspath;
        // cout<<p.root_path()<<endl;
        // cout<<fs::absolute(p)<<endl;
        char resolved[2000];
        realpath(check_path.c_str(), resolved);
        // cout<<
        string abs_path = string(resolved);
        // cout << "Absolute Path: " << abs_path << endl;
        vector<string> directories_in_path;
        string temp;
        for (int i = cwd.size() + 1; i < (int)abs_path.size(); i++)
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
        string temp_path = vcspath + "/" + version;
        for (auto i : directories_in_path)
        {
            // cout << "Here " << i << endl;
            // if (count<2)
            //     {
            //         count++;
            //         temp_path=temp_path+"/"+i;
            //         continue;
            //     }
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
    
    
    /* copy file from cwd to staging area(./.vcs/version_no/filePath) */
    void copyfile(string path1, string filePath)
    {
        string version, path2 = filePath;
        // cout << "copyfile src " << path1 << endl;
        // cout << "copyfile dest " << path2 << endl;
        ifstream vfile("./.vcs/version.info");
        vfile >> version;
        vfile.close();

        if(filePath.substr(0, 7) != "./.vcs/"){
            // cout << "not found\n";
            path2 = "./.vcs/" + version + "/"+filePath;
        }

        string cwd = fs::current_path();
        // cout << "copyfile dest " << path2 << endl;
        calculateFileSHA(path1);
        char buffer[1000];
        memset(&buffer[0], 0, sizeof(buffer));
        int filecheck1, filecheck2;
        struct stat buff;
        stat(path1.c_str(), &buff);

        // opening the first file
        filecheck1 = open(path1.c_str(), O_RDONLY);

        if (filecheck1 == -1)
        {
            printf("Error opening first_file\n");
            close(filecheck1);
            return;
        }

        filecheck2 = open(path2.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (filecheck2 == -1)
        {
            create_dir_structure(path1, version);
            filecheck2 = open(path2.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            // cout << "Succces---" << endl;
        }
        int a = 0;
        while ((a = read(filecheck1, buffer, 1000)))
        {
            if(a != -1) {
                write(filecheck2, buffer, a);
            }
            else break;
        }
        chmod(path2.c_str(), buff.st_mode);

        // printf("Successful copy\n");

        close(filecheck1);
        close(filecheck2);
    }
    
    
    /* copy file from cwd to staging area(./.vcs/version_no/directoryPath) */
    void copydirectory(string path1, string path2)
    {
        string cwd = fs::current_path();
        struct stat mbuff;
        stat(path1.c_str(), &mbuff);
        // opening the folder
        DIR *folder = opendir(path1.c_str());
        // creating a directory with the same name at specified vcspath

        if (!(fs::exists(path2.c_str())))
        // if (createdir(path2) == -1)
        {
            string version;
            ifstream vfile("./.vcs/version.info");
            vfile >> version;
            vfile.close();
            create_dir_structure(path1, version);
            // cout << "no dir";
            // return;
        }

        if (folder == NULL){
            cout << "returning null";
            return;
        }
        // reading files in a directory
        struct dirent *file;
        file = readdir(folder);

        while (file != NULL)
        {

            // if(strcmp(file->d_name,name.c_str())==0){

            // 	cout<<name<<file->d_name<<endl;
            // 	return true;
            // 	}

            if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0){
                // cout << ". or .. detected" << endl;
            }
            else
            {

                // string filepath= path1 +"/"+file->d_name;
                if (file->d_type == DT_DIR)
                {
                    // cout << "Directory " << path1 + "/" + file->d_name << endl;
                    pair<string, string> PAIR;
                    PAIR.first = path1 + "/" + file->d_name;
                    PAIR.second = path2 + "/" + file->d_name;
                    struct stat buff;
                    stat(path1.c_str(), &buff);
                    copydirectory(PAIR.first, PAIR.second);
                }
                else
                {
                    pair<string, string> PAIR;
                    PAIR.first = path1 + "/" + file->d_name;
                    PAIR.second = path2 + "/" + file->d_name;
                    struct stat buff;
                    stat(path1.c_str(), &buff);
                    copyfile(PAIR.first, PAIR.second);
                }
            }
            file = readdir(folder);

            chmod(path2.c_str(), mbuff.st_mode);
        }

        closedir(folder);
    }

    
    /* add file/directory to staging area(./.vcs/versionNum/cwdfilePath) */
    void addFile(string addPath, string version)
    {
        string cwd = fs::current_path();
        string filepath = cwd + "/" + string(fs::path(addPath.c_str()));
        string new_path = "./.vcs/" + version + "/" + string(fs::path(addPath.c_str()));
        if (fs::is_directory(addPath.c_str()))          //if it is a directory
        {
            // cout << "Directory" << addPath << endl;
            create_dir_structure(addPath, version);
            copydirectory(addPath, new_path);
        }

        else
        {
            create_dir_structure(addPath, version);         //if it is a file
            copyfile(filepath, new_path);
        }
    }

    
    /* given a path to file/directory ; add it to staging area */
    void addComplete(string path)
    {
        string cwd = fs::current_path();
        string filepath1 = cwd + "/" + path;
        // cout << endl << path << endl;
        if(fs::is_directory(filepath1.c_str())) {       
            copy_version(path);       //if directory => do rec. traversal of that dir and copy all it's contents to staging area 
        } 
        else {                                          //if file => copy the file to staging area
            copyfile(filepath1,path);
        }

    }


}


/*
    Summary of implementation of add command : 
        1. Given the req file/directory path, we will add the file to staging area ( ./.vcs/version_num/filepath)
        2. Different logic for file and directory but at the end, we need to add them to staging area
        3. we are copying the entire file and not compressing it nor storing only the differences in current 
        and prev version (not so optimal)
*/
