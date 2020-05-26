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
#include <unordered_map>

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

#define END_OF_FILE -1
#define DIRECTORY_START 0
#define NULL_ID -1

/*
  This struct type is used to log any issues or unexpected behaviour. Its
  constructor reieves the name of the class the struct is being used in.
*/
struct IssueLogger {

  string class_name;

  IssueLogger() {}
  IssueLogger(string _class_name): class_name(_class_name) {}

  /*
    This function recieves the name of the function that an issue has appeared
    in, and a string describing the error. Then it concatenates all necessary
    information and prints it to standard error.
  */
  void Log(string func_name, string error) {

    string Res = "Issue: " + class_name + ": " + func_name + ": " + error + "\n";

    cerr << Res ;
  }
};

/*
  This struct type encapsulates file metadata.
  index:      index at which the first block of the file is stored in Directory
  block_len:  number of blocks occupied by the file
  byte_len:   number of bytes occupied by the file
*/
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

/*
  NullFile represents an empty undefined file. It used as a return value
  for cases where a function issues an error and the return type is a file.
*/
File NullFile = File(-1, -1, -1);

/*
  This function represents the data structure that holds info about files
  in the directory. It consists of a mapping whose key is the file ID, and
  whose value the file metadata represented by File struct defined above.
*/
struct DirectoryTable {

  // this map has file ID as key and file metadata as value
  unordered_map<int, File> Table;
  IssueLogger Logger;

  DirectoryTable(): Logger(IssueLogger("DirectoryTable")) {}

  /*
    This function checkes if a file exists in the directory given
    its ID.
  */
  bool FileExists(int fileID) {

    return Table.count(fileID);
  }

  /*
    This function takes a file ID and returns the metadata of that file
  */
  File GetFile(int fileID) {

    // if a file does not exist, raise an issue
    if (!Table.count(fileID)) {
      Logger.Log("GetFile", "Requesting an entry that does not exist");
      return NullFile;
    }

    return Table[fileID];
  } 

  /*
    This function adds a new file to the Directory table
  */
  int AddFile(int fileID, File entry) {

    // If a file of the given fileID already exists, raise an issue.
    if (Table.count(fileID)) {
      Logger.Log("AddFile", "Cannot add Entry that already exists");
      return FAIL;
    }

    Table[fileID] = entry;

    return SUCCESS;
  }

  /*
    This function removes a file from the Directory Table.
  */
  int RemoveFile(int fileID) {

    // if file does not exist, raise an issue, because we expect
    // a file we remove to exist
    if (!Table.count(fileID)) {
      Logger.Log("RemoveFile", "Cannot Remove Entry that does not exist");
      return FAIL;
    }

    Table.erase(fileID);

    return SUCCESS;
  }

  /*
    This function updates the index of a file stored in the directory.
    Remember: the index is the index of the block at which this file
    starts in the Directory.
  */
  int UpdateIndex(int fileID, int new_index) {

    // if file does not exist, raise an issue
    if (!Table.count(fileID)) {
      Logger.Log("UpdateIndex", "Given fileID does not exist");
      return FAIL;
    }

    Table[fileID].index = new_index;

    return SUCCESS;
  }

  /*
    This function updates the length of bytes of a file in the directory
  */
  int UpdateByteLen(int fileID, int new_len) {

    // if file does not exist, raise an issue
    if (!Table.count(fileID)) {
      Logger.Log("UpdateByteLength", "Given fileID does not exist");
      return FAIL;
    }

    Table[fileID].byte_len = new_len;

    return SUCCESS;
  }

  /*
    This function updates the length of blocks of a file in the directory
  */
  int UpdateBlockLen(int fileID, int new_len) {

    // if file does not exist, raise an issue
    if (!Table.count(fileID)) {
      Logger.Log("UpdateBlockLen", "Given fileID does not exist");
      return FAIL;
    }

    Table[fileID].block_len = new_len;

    return SUCCESS;
  }

};

/*
  
*/
struct ContiguousAllocation {

  int block_size;
  int available_space;
  int Directory[MAX_BLOCKS];
  DirectoryTable Table;
  IssueLogger Logger;

  ContiguousAllocation(int _block_size) {

    block_size = _block_size;
    available_space = MAX_BLOCKS;
    Table = DirectoryTable();
    Logger = IssueLogger("ContiguousAllocation");
    memset(Directory, EMPTY, sizeof(Directory));
  }

  int ByteToBlock(int length) {

    return (length + block_size - 1) / block_size;
  }

  int Move(int fileID, int new_index) {

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    int old_index = F.index;

    for (int i = 0 ; i < F.block_len ; ++i) {

      if (Directory[new_index + i] != EMPTY) {
        Logger.Log("Move", "Failed to Move because destination is occupied at");
        return FAIL;
      }

      Directory[new_index + i] = fileID;
      Directory[old_index + i] = EMPTY;
    }

    int status = Table.UpdateIndex(fileID, new_index);

    if (status == FAIL) return status;

    return SUCCESS;
  }

  int ApplyCompaction(int start_index) {

    int last = start_index;

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

  bool CanExtend(int index, int amount) {

    for (int i = index ; i < index + amount ; ++i) {

      if (Directory[i] != EMPTY) return false;
    }

    return true;
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

        string t = "(" + to_string(fileID) + "," + to_string(index) + to_string(length) + ")";
        Logger.Log("Fill", "Cannot fill in already full slot:" + t);
        return FAIL;
      }

      Directory[i] = fileID;
    }

    return SUCCESS;
  }

  int Empty(int index, int length) {

    for (int i = index ; i < index + length ; ++i) {

      if (Directory[i] == EMPTY) {
        Logger.Log("Emty", "Attempting to Empty an already Empty spot, this should not happen");
        return FAIL;
      }

      Directory[i] = EMPTY;
    }

    return SUCCESS;
  }

  int CreateFile(int fileID, int file_length) {

    if (Table.FileExists(fileID)) {
      Logger.Log("create_file", "Cannot create a file that already exists");
      return FAIL;
    }

    int block_num = ByteToBlock(file_length);

    if (block_num > available_space) {
      Logger.Log("CreateFile", "Creation Rejected due to insufficient space");
      return REJECT;
    }

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

    available_space -= block_num;

    return SUCCESS;
  }

  int Access(int fileID, int byte_offset) {

    if (!Table.FileExists(fileID)) {
      Logger.Log("Access", "Cannot access file that does not exist");
      return FAIL;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    if (F.byte_len < byte_offset) {
      Logger.Log("Access", "Byte offset to be accessed exceeds actual file size");
      return FAIL;
    }

    int block_offset = ByteToBlock(byte_offset);

    return F.index + block_offset - 1;
  }

  int Extend(int fileID, int extension_amount) {

    if (!Table.FileExists(fileID)) {
      Logger.Log("Extend", "Cannot extend file that does not exist");
      return FAIL;
    }

    if (available_space < extension_amount) {
      Logger.Log("Extend", "Extension Rejected due to insufficient space");
      return REJECT;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) return FAIL;

    if (CanExtend(F.index + F.block_len, extension_amount)) {

      int status = Fill(fileID, F.index + F.block_len, extension_amount);

      if (status == FAIL) {
        return status;
      }

    } else {

      ApplyCompaction(DIRECTORY_START);

      int old_index = Table.GetFile(fileID).index;
      int new_index = MAX_BLOCKS - available_space;

      Move(fileID, new_index);

      int status = Fill(fileID, new_index + F.block_len, extension_amount);

      if (status == FAIL) {
        return status;
      }

      ApplyCompaction(old_index);
    }

    Table.UpdateBlockLen(fileID, F.block_len + extension_amount);
    Table.UpdateByteLen(fileID, F.byte_len + block_size * extension_amount);

    available_space -= extension_amount;

    return SUCCESS;
  }

  int Shrink(int fileID, int shrink_amount) {

    if (!Table.FileExists(fileID)) {
      Logger.Log("Shrink", "Cannot shrink file that does not exist");
      return FAIL;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    if (F.block_len < shrink_amount) {
      Logger.Log("Shrink", "Shrink aborted because shrink amount is greater than file size");
      return FAIL;
    }

    int blocks_left = F.block_len - shrink_amount;

    int status = Empty(F.index + blocks_left, shrink_amount);

    if (status == FAIL) {
      return status;
    }

    if (blocks_left == 0) {

      status = Table.RemoveFile(fileID);

      if (status == FAIL) {
        return FAIL;
      }

    } else {

      Table.UpdateBlockLen(fileID, blocks_left);
      Table.UpdateByteLen(fileID, F.byte_len - block_size * shrink_amount);
    }

    available_space += shrink_amount;

    return SUCCESS;
  }

  void PrintSlice(int l, int r) {

    for (int i = l ; i < r ; ++i) {
      cout << Directory[i] << endl;
    }
  }

  void ExploreTable() {

    for (auto el : Table.Table) {
      File F = el.second;
      cout << el.first << " : " << F.index << " " << F.block_len << " " << F.byte_len << endl;
    }
  }

};

struct LinkedFile {

  int status;
  int next;
  int ID;

  LinkedFile(): ID(-1), next(END_OF_FILE), status(EMPTY) {}
  LinkedFile(int _ID): ID(_ID), next(END_OF_FILE), status(EMPTY) {}

  void Fill(int _ID) {
    status = _ID;
    ID = _ID;
  }

  void Empty() {
    status = EMPTY;
    ID = NULL_ID;
    next = END_OF_FILE;
  }

  void UpdateNext(int new_next) {
    next = new_next;
  }

  void UpdateStatus(int new_status) {
    status = new_status;
  }

};

struct LinkedAllocation {

  int block_size;
  int available_space;
  DirectoryTable Table;
  LinkedFile Directory[MAX_BLOCKS];
  IssueLogger Logger;

  LinkedAllocation(int _block_size) {
    Table = DirectoryTable();
    block_size = _block_size;
    Logger = IssueLogger("LinkedAllocation");
    available_space = MAX_BLOCKS;
  }

  int ByteToBlock(int length) {

    return (length + block_size - 1) / block_size;
  }

  vector<int> FindAvailableSpace(int block_num) {

    vector<int> Res;

    for (int i = 0 ; i < MAX_BLOCKS ; ++i) {
      if (Directory[i].status == EMPTY) {
        Res.push_back(i);
      }

      if (Res.size() == block_num) return Res;
    }

    return Res;
  }

  int CreateFile(int fileID, int file_length) {

    if (Table.FileExists(fileID)) {
      Logger.Log("create_file", "Cannot create a file that already exists");
      return FAIL;
    }

    int block_num = ByteToBlock(file_length);

    if (block_num > available_space) {
      Logger.Log("CreateFile", "Creation Rejected due to insufficient space");
      return REJECT;
    }

    vector<int> space = FindAvailableSpace(block_num);

    if (space.size() != block_num) {
      Logger.Log("CreateFile", "Number of Slots found does not match requested number.");
      return FAIL;
    }

    for (int i = 0 ; i < block_num ; ++i) {

      int x = space[i];

      if (Directory[x].status != EMPTY) {
        Logger.Log("CreateFile", "Expected an empty space in directory, but it is not empty");
        return FAIL;
      }

      Directory[x].Fill(fileID);

      if (i + 1 < block_num) {

        int next = space[i + 1];
        Directory[x].UpdateNext(next);
      }
    }

    int status = Table.AddFile(fileID, File(space[0], block_num, file_length));

    if (status == FAIL) {
      return status;
    }

    available_space -= block_num;

    return SUCCESS;
  }

  int Access(int fileID, int byte_offset) {

    if (!Table.FileExists(fileID)) {
      Logger.Log("Access", "Cannot access file that does not exist");
      return FAIL;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    if (F.byte_len < byte_offset) {
      Logger.Log("Access", "Byte offset to be accessed exceeds actual file size");
      return FAIL;
    }

    int index = Table.GetFile(fileID).index;

    while (block_size < byte_offset) {
      byte_offset -= block_size;
      index = Directory[index].next;
    }

    return index;
  }

  int Extend(int fileID, int extension_amount) {

    if (!Table.FileExists(fileID)) {
      Logger.Log("Extend", "Cannot extend file that does not exist");
      return FAIL;
    }

    if (available_space < extension_amount) {
      Logger.Log("Extend", "Extension Rejected due to insufficient space");
      return REJECT;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    vector<int> space = FindAvailableSpace(extension_amount);

    if (space.size() != extension_amount) {
      Logger.Log("Extend", "Number of Slots found does not match requested number.");
      return FAIL;
    }

    int index = F.index;

    while (Directory[index].next != END_OF_FILE) {
      index = Directory[index].next;
    }

    Directory[index].next = space[0];

    for (int i = 0 ; i < space.size() ; ++i) {

      int x = space[i];

      if (Directory[x].status != EMPTY) {
        Logger.Log("CreateFile", "Expected an empty space in directory, but it is not empty");
        return FAIL;
      }

      Directory[x].Fill(fileID);

      if (i + 1 < space.size()) {
        int next = space[i + 1];
        Directory[x].UpdateNext(next);
      }
    }

    Table.UpdateBlockLen(fileID, F.block_len + extension_amount);
    Table.UpdateByteLen(fileID, F.byte_len + block_size * extension_amount);

    available_space -= extension_amount;

    return SUCCESS;
  }

  int Shrink(int fileID, int shrink_amount) {

    if (!Table.FileExists(fileID)) {
      Logger.Log("Shrink", "Cannot shrink file that does not exist");
      return FAIL;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    if (F.block_len < shrink_amount) {
      Logger.Log("Shrink", "Shrink aborted because shrink amount is greater than file size");
      return FAIL;
    }

    int blocks_left = F.block_len - shrink_amount;

    int index = F.index;

    if (blocks_left == 0) {

      int status = Table.RemoveFile(fileID);

      if (status == FAIL) return status;

    } else {

      Table.UpdateBlockLen(fileID, F.block_len - shrink_amount);
      Table.UpdateByteLen(fileID, F.byte_len - block_size * shrink_amount);

      for (int i = 0 ; i < blocks_left - 1 ; ++i) {
        index = Directory[index].next;
      }

      int next = Directory[index].next;
      Directory[index].UpdateNext(END_OF_FILE);
      index = next;
    }

    while (index != END_OF_FILE) {
      int next = Directory[index].next;
      Directory[index].Empty();
      index = next;
    }

    available_space += shrink_amount;

    return SUCCESS;
  }

  void PrintSlice(int l, int r) {

    for (int i = l ; i < r ; ++i) {
      cout << Directory[i].ID << endl;
    }
  }

  void ExploreTable() {

    for (auto el : Table.Table) {
      File F = el.second;
      cout << el.first << " : " << F.index << " " << F.block_len << " " << F.byte_len << endl;
    }
  }

};



int main() {

  LinkedAllocation LA(1024);
  ContiguousAllocation CA(1024);

  LA.CreateFile(3, 10000);
  CA.CreateFile(3, 10000);
  
  LA.PrintSlice(0, 20);
  puts("");
  CA.PrintSlice(0, 20);
  puts("");

  LA.Shrink(3, 3);
  CA.Shrink(3, 3);

  LA.PrintSlice(0, 20);
  puts("");
  CA.PrintSlice(0, 20);
  puts("");

  LA.Extend(3, 1);
  CA.Extend(3, 1);

  LA.PrintSlice(0, 20);
  puts("");
  CA.PrintSlice(0, 20);
  puts("");

}
