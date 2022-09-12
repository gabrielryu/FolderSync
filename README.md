# FolderSync
The simple one-way folder synchronization application, written in c++
-------------------------------------------------------------------------------------
- Synchronize the source folder and the replica folder periodically
- Can set the replica folder path, Synchronization interval time and the log file path using the CLI
- Check the file reliability using MD5 library (http://www.bzflag.org)
- Tested working on OSX

Build
-------------------------------------------------------------------------------------
- Make
```sh
$ make main
```

Usage
-------------------------------------------------------------------------------------
```sh
main [ Replica Folder Path | Time Interval(sec) | Log File Path]

example: 
$ ./main bar 5 log_sample.txt
```
If you don't add parameters, it runs with default values. Default : Original (foo), Replica (bar), Interval (10 sec), Log file (log.txt)