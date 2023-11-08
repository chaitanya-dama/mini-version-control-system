### Description
Built a mini version control system. Every change to the code is recorded by version control software in a particular form of database. When a mistake is made, engineers can go back to an earlier version of the code to help correct it with the least amount of interruption to the entire team.

Implemented commands like 'init', 'add', 'commit', 'log', 'status, 'diff', 'rollback', 'retrieve -a' and 'retrieve SHA'.




## Supports Commands : 

### ./vcs init
```
./vcs init 
```

This command will create a .vcs repository and the required structure. This folder will contain all the version control information. 

### ./vcs status
```
./vcs status 
```
The following command will show the status of the current working tree. Initially all will be untracked. 

### ./vcs add [file name/s]
```
./vcs add .
```
This command will add the file/files in the current directory to the staging area. 

### ./vcs commit [message]
```
./vcs commit “first commit” 
```
The following command will commit all the files in the staging area. 

### ./vcs diff [file name]
```
./vcs diff temp.txt 
```
The following command shows the difference for temp.txt. As described above. 


### ./vcs log
```
./vcs log 
```
The following command will log all the previous commit data. 

### ./vcs rollback [commmitId]
```
./vcs rollback 76931fa 
```
This will first commit the current staging area, and then rollback to the version corresponding to the commit id given. 

### ./vcs retrieve -a [versionNo]
```
./vcs retrieve -a 0 
```
This command will return the “commit id” of the version no, if the version no is valid. 

### ./vcs retrieve SHA [versionNo]
```
./vcs retrieve SHA 0 
```
This command will return the “commit SHA” of the version no, if the version no is valid. 
