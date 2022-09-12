/**
 * @file        main.cpp
 * @version     1.0
 * @brief       FolderSync main application source
 * @date        Sep 12, 2022
 * @author      Youngrak Ryu (gabrielryu@me.com)
 */
#include "main.h"

//#include <iostream>
//#include <unistd.h>
#include <cstdio>

using namespace std;
namespace fs = filesystem;

const string snapshot_path = "snapshot.dat";
const string origin_path = "foo";
string replica_path = "bar";
string logPath = "log.txt";
int interval = 10;

inline string getCurrentDateTime(string s)
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    if (s == "now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if (s == "date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return string(buf);
};
inline void Logger(string logMsg, const char *func)
{
    string now = getCurrentDateTime("now");
    ofstream ofs(logPath.c_str(), std::ios_base::out | std::ios_base::app);
    ofs << now << '\t' << '[' << func << ':' << __LINE__ << ']' << logMsg << '\n';
    cout << now << '\t' << '[' << func << ':' << __LINE__ << ']' << logMsg << '\n';
    ofs.close();
}

void Usage(const char *pname)
{
    cout << "Useage: " << pname << " [Replica Folder]"
         << " [Time Interval(sec)]" << " [log file]" << "\n";
    cout << "ex) " << pname << "bar 5 log_sample.txt\n";
    cout << "If you don't add parameters, it runs with default values.\n";
    cout << "Default : Original (foo), Replica (bar), Interval (10 sec), Log file (log.txt)\n";
    cout << "\n\n\n";
}

int main(int argc, char *argv[])
{
    cout << "****************************************\n";
    cout << "*      Two Folder Sync Application     *\n";
    cout << "****************************************\n\n\n";
    if (argc < 4)
        Usage(argv[0]);
    else if (argc > 4)
    {
        Usage(argv[0]);
        return 0;
    }
    else
    {
        replica_path = argv[1];
        interval = atoi(argv[2]);
        logPath = argv[3];
    }
    
    
    
    if (init())
    {
        Logger("INFO: Completed to update the snapshot and replica directory", __func__);
    }
    else
    {
        Logger("ERROR: Failed to initialize", __func__);
        exit(1);
    }

    sync();
    return 0;
}

bool init()
{
    ofstream out(snapshot_path, ios::trunc);
    try
    {
        if (!out.is_open())
            throw "ERROR: No such file or directory";
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        Logger(e.what(), __func__);
        return false;
    }
    read_files(out);

    if (!file_copy())
    {
        Logger("ERROR: Failed to copy files", __func__);
        return false;
    }
    return true;
}

bool file_copy()
{
    auto from = fs::path{fs::current_path() / origin_path};
    auto to = fs::path{fs::current_path() / replica_path};
    try
    {
        fs::copy(from, to, fs::copy_options::recursive | fs::copy_options::update_existing);
        Logger("Completed to copy files", __func__);
    }
    catch (const fs::filesystem_error &err)
    {
        Logger(err.what(), __func__);
        Logger(err.path1().c_str(), __func__);
        Logger(err.path2().c_str(), __func__);
        Logger(err.code().message().c_str(), __func__);
        return false;
    }
    return true;
}

void remove_file(string path)
{
    error_code err;
    const fs::path p(fs::current_path() / path);
    try
    {
        fs::remove_all(p);
        Logger("INFO: Removed the file : " + (string)p.filename(), __func__);
    }
    catch (const fs::filesystem_error &err)
    {
        Logger(err.what(), __func__);
        Logger(err.path1().c_str(), __func__);
        Logger(err.path2().c_str(), __func__);
        Logger(err.code().message().c_str(), __func__);
        exit(1);
    }
}

void read_files(ofstream &out)
{
    fs::recursive_directory_iterator itr(fs::current_path() / origin_path);
    while (itr != fs::end(itr))
    {
        const fs::directory_entry &entry = *itr;
        if (entry.path().filename() == ".DS_Store")
        {
            itr++;
            continue;
        }
        if (!entry.path().empty())
        {
            out << entry.path().c_str() << "\n";
            out << md5chk(entry.path().c_str()) << "\n";
        }

        itr++;
    }
    out.close();
}
void read_files(ifstream &in)
{
    bool changed;
    string snap_fpath, snap_md5, current_md5;

    in.seekg(0, in.end);
    int f_length = in.tellg();
    in.seekg(0, in.beg);
    fs::recursive_directory_iterator itr(fs::current_path() / origin_path);
    while (itr != fs::end(itr))
    {
        const fs::directory_entry &entry = *itr;
        if (entry.path().filename() == ".DS_Store")
        {
            itr++;
            continue;
        }

        getline(in, snap_fpath);
        getline(in, snap_md5);
        current_md5 = md5chk(entry.path().c_str());
        // When the file and the md5 value are not updated
        if (snap_fpath == entry.path() && snap_md5 == current_md5)
        {
            itr++;
            continue;
        }
        // file is updated
        if (snap_fpath == entry.path() && snap_md5 != current_md5)
        {
            file_copy();
            Logger("INFO: Update the file", __func__);
            changed = true;
        }
        // The file is added or removed
        if (snap_fpath != entry.path())
        {
            string new_snap_fpath;
            in.seekg(0, ios::beg);
            getline(in, new_snap_fpath);
            getline(in, snap_md5);

            while (in)
            {
                if (snap_fpath == new_snap_fpath)
                {
                    // The legacy file is removed
                    string curr_path = fs::current_path().c_str();
                    string to = snap_fpath.substr(curr_path.length() + origin_path.length() + 2, snap_fpath.length());
                    to.insert(0, "/");
                    to.insert(0, replica_path);
                    remove_file(to);
                    in.close();
                    return;
                }
            }
            in.close();
            // Add a new file
            if (init())
                Logger("INFO: Add a new file", __func__);
            else
            {
                Logger("ERROR: Failed to synchronize", __func__);
                exit(1);
            }
            return;
        }
    }
    if (in.tellg() < f_length) // When the last file of the snapshot list is removed, it check one more and remove.
    {
        getline(in, snap_fpath);
        getline(in, snap_md5);
        string curr_path = fs::current_path().c_str();
        string to = snap_fpath.substr(curr_path.length() + origin_path.length() + 2, snap_fpath.length());
        to.insert(0, "/");
        to.insert(0, replica_path);
        remove_file(to);
        in.close();
        return;
    }

    if (!changed)
        Logger("INFO: No updates", __func__);
    in.close();
}

void sync()
{
    while (true)
    {
        sleep(interval);
        ifstream in(snapshot_path);
        try
        {
            if (!in.is_open())
                throw "ERROR: No such file or directory";
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            Logger(e.what(), __func__);
            return exit(1);
        }

        read_files(in);

        ofstream out(snapshot_path, ios::trunc);
        try
        {
            if (!out.is_open())
                throw "ERROR: No such file or directory";
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            Logger(e.what(), __func__);
            exit(1);
        }
        read_files(out);
        Logger("INFO: Synchronized origin and replica folders", __func__);
    }
}

string md5chk(const char *filename)
{
    FILE *inFile;
    int bytes;
    inFile = fopen(filename, "rb");

    MD5 md5sum = MD5();
    unsigned char buff[1024];

    try
    {
        if (inFile == NULL)
            throw "ERROR: No such file or directory";
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        Logger(e.what(), __func__);
        return 0;
    }

    while ((bytes = fread(buff, 1, 1024, inFile)) != 0)
        md5sum.update(buff, bytes);

    fclose(inFile);
    md5sum.finalize();
    return md5sum.hexdigest();
}