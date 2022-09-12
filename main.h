/**
 * @file        main.h
 * @version     1.0
 * @brief       FolderSync main application header
 * @date        Sep 12, 2022
 * @author      Youngrak Ryu (gabrielryu@me.com)
 */

#ifndef _MAIN_H
#define _MAIN_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include "md5.h"

using namespace std;


string md5chk(const char *filename);
void read_files(std::ofstream &out);
void read_files(ifstream &in);
void sync();
bool init();
bool file_copy();
void remove_file(string path);


#endif /* _MAIN_H */
