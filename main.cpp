#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string.h>
#include <string>
#include <vector>

using namespace std;  

/*
  This code snippet serves as a debugger it can take an arbitrary number of
   arguments in one call, and print to standard error in the format
   [name_of_variable] = [value_of_variable]
   which is quite useful and lightweight for debugging
*/
void __print(int x) {cerr << x;}
void __print(long x) {cerr << x;}
void __print(long long x) {cerr << x;}
void __print(unsigned x) {cerr << x;}
void __print(unsigned long x) {cerr << x;}
void __print(unsigned long long x) {cerr << x;}
void __print(float x) {cerr << x;}
void __print(double x) {cerr << x;}
void __print(long double x) {cerr << x;}
void __print(char x) {cerr << '\'' << x << '\'';}
void __print(const char *x) {cerr << '\"' << x << '\"';}
void __print(const string &x) {cerr << '\"' << x << '\"';}
void __print(bool x) {cerr << (x ? "true" : "false");}
template<typename T, typename V>
void __print(const pair<T, V> &x) {
  cerr << '{'; __print(x.first);
  cerr << ','; __print(x.second); cerr << '}';
}
template<typename T>
void __print(const T &x) {
  int f = 0; cerr << '{';
  for (auto &i : x)
    cerr << (f++ ? "," : ""), __print(i); cerr << "}";
}
void _print() {cerr << "]\n";}
template <typename T, typename... V>
void _print(T t, V... v) {
  __print(t);
  if (sizeof...(v))
    cerr << ", "; _print(v...);
}

//#define ONLINE_MODE
#ifndef ONLINE_MODE
#define debug(x...) cerr << "[" << #x << "] = ["; _print(x)
#else
#define debug(x...)
#endif



#define MAX_BLOCKS 32768
#define FAIL -1
#define EMPTY 0
#define FULL 1
#define SUCCESS 2
#define REJECT 3

#define DIRECTORY_START 0

struct IssueLogger {

  string class_name;

  IssueLogger() {}
  IssueLogger(string _class_name): class_name(_class_name) {}

  void Log(string func_name, string error) {

    string Res = "Issue: " + class_name + ": " + func_name + ": " + error + "\n";

    cerr << Res ;
  }
};

struct File {

  int index;
  int block_len;
  int byte_len;

  File() {}
  File(int index, int block_len, int byte_len): index(index), block_len(block_len), byte_len(byte_len) {}

  bool operator<(const File& F) const {
    return index < F.index;
  }

  bool operator==(const File& F) const {
    return index == F.index && byte_len == F.byte_len;
  }

};

File NullFile = File(-1, -1, -1);

struct DirectoryTable {

  map<int, File> Table;
  IssueLogger Logger;

  DirectoryTable(): Logger(IssueLogger("DirectoryTable")) {}

  bool FileExists(int fileID) {

    return Table.count(fileID);
  }

  File GetFile(int fileID) {

    if (!Table.count(fileID)) {
      Logger.Log("GetFile", "Requesting an entry that does not exist");
      return NullFile;
    }

    return Table[fileID];
  }

  int AddFile(int fileID, File entry) {

    if (Table.count(fileID)) {
      Logger.Log("AddFile", "Cannot add Entry that already exists");
      return FAIL;
    }

    Table[fileID] = entry;

    return SUCCESS;
  }

  int RemoveFile(int fileID) {

    if (!Table.count(fileID)) {
      Logger.Log("RemoveFile", "Cannot Remove Entry that does not exist");
      return FAIL;
    }

    Table.erase(fileID);

    return SUCCESS;
  }

  int UpdateIndex(int fileID, int new_index) {

    if (!Table.count(fileID)) {
      Logger.Log("UpdateIndex", "Given fileID does not exist");
      return FAIL;
    }

    Table[fileID].index = new_index;

    return SUCCESS;
  }

  int UpdateByteLength(int fileID, int new_len) {

    if (!Table.count(fileID)) {
      Logger.Log("UpdateByteLength", "Given fileID does not exist");
      return FAIL;
    }

    Table[fileID].byte_len = new_len;

    return SUCCESS;
  }

  int UpdateBlockLen(int fileID, int new_len) {

    if (!Table.count(fileID)) {
      Logger.Log("UpdateBlockLen", "Given fileID does not exist");
      return FAIL;
    }

    Table[fileID].block_len = new_len;
  }

};


struct ContiguousFileAllocation {

  int block_size;
  int available_space;
  int Directory[MAX_BLOCKS];
  DirectoryTable Table;
  IssueLogger Logger;

  ContiguousFileAllocation(int _block_size) {

    block_size = _block_size;
    available_space = MAX_BLOCKS;
    Table = DirectoryTable();
    Logger = IssueLogger("ContiguousFileAllocation");
    memset(Directory, EMPTY, sizeof(Directory));
  }

  int ByteToBlock(int length) {

    return (length + block_size - 1) / block_size;
  }

  int Move(int fileID, int new_index) {

    File F = Table.GetFile(fileID);

    int old_index = F.index;

    for (int i = 0 ; i < F.block_len ; ++i) {

      if (Directory[new_index + i] != EMPTY) {
        Logger.Log("Move", "Failed to Move because destination is occupied at");
        return FAIL;
      }

      Directory[new_index + i] = fileID;
      Directory[old_index + i] = EMPTY;
    }

    int status = Table.RemoveFile(fileID);

    if (status == FAIL) {
      return status;
    }

    F.index = new_index;

    Table.AddFile(fileID, F);

    return SUCCESS;
  }

  int ApplyCompaction(int start_index) {

    int last = 0;

    for (int i = start_index ; i < MAX_BLOCKS ; ++i) {

      if (Directory[i] == EMPTY) continue;

      int ID = Directory[i];

      int status = Move(Directory[i], last);

      if (status == FAIL) {
        return status;
      }

      last += Table.GetFile(ID).block_len;
    }

    return SUCCESS;
  }

  int FindAvailableSpace(int block_num) {

    for (int i = 0, j ; i < MAX_BLOCKS ; i = j) {

      j = i + 1;

      if (Directory[i] != EMPTY) continue;

      while (j < MAX_BLOCKS && Directory[j] == EMPTY) {

        if (j - i == block_num) break;

        j++;
      }

      if (j - i == block_num) return i;
    }

    return FAIL;
  }

  /*
    Blocks will be filled with the FileID of the file being stored
  */
  int Fill(int fileID, int index, int length) {

    for (int i = index ; i < index + length ; ++i) {

      if (Directory[i] != EMPTY) {
        Logger.Log("Fill", "Cannot fill in already full slot");
        return FAIL;
      }

      Directory[i] = fileID;
    }

    return SUCCESS;
  }

  int create_file(int fileID, int file_length) {

    if (Table.FileExists(fileID)) {
      Logger.Log("create_file", "Cannot create a file that already exists");
      return FAIL;
    }

    int block_num = ByteToBlock(file_length);

    if (block_num > available_space) return REJECT;

    int index = FindAvailableSpace(block_num), status;

    if (index == FAIL) {

      ApplyCompaction(DIRECTORY_START);

      index = MAX_BLOCKS - available_space;
    }

    status = Table.AddFile(fileID, File(index, block_num, file_length));

    if (status == FAIL) {
      return FAIL;
    }

    status = Fill(fileID, index, block_num);

    if (status == FAIL) {
      return FAIL;
    }

    return SUCCESS;
  }

  void access(int fileID, int byte_offset) {

  }

  void extend(int fileID, int extension_amount) {

  }

  void shrink(int fileID, int shirinking_amount) {

  }

};



int main() {

  ContiguousFileAllocation CFA(1024);

  CFA.create_file(7,20000);
  CFA.create_file(8,2049);

  int status  = CFA.Move(7, 50);

  cout << "Status: " << status << endl;

  CFA.ApplyCompaction(0);

  for(int i = 0 ; i < 100 ; ++i) {
    cout << CFA.Directory[i] << endl;
  }

}
