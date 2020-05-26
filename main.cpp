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
  This struct type encapsulates a file system implemented with
  Contiguous Allocation strategy. Its attributes are:

  block_size:       the size of each block in the directory
  available_space:  stores the available space left in the directory it
                    makes it easier and faster for several operations
  Directory:        represents the blocks of the directory
  Table:            represents the Directory Table data structure
  Logger:           used to log issues to standard error
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

  /*
    Takes length of bytes and returns number of blocks required
    to store these bytes, which is taken by dividing length by
    block size and taking the ceil value
  */
  int ByteToBlock(int length) {

    return (length + block_size - 1) / block_size;
  }

  /*
    This function takes a file ID and a new index, and moves that file
    to start from the new index. If while moving the file, some part of
    the destination appears to be occupied, the operation fails.
  */
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

      // move block by block, each block is moved directly
      // from its old block to its new block

      Directory[new_index + i] = fileID;
      Directory[old_index + i] = EMPTY;
    }

    // update index in Directory Table
    int status = Table.UpdateIndex(fileID, new_index);

    if (status == FAIL) return status;

    return SUCCESS;
  }

  /*
    This function applies a compaction operation on the directory, this
    operation is defined as follows, for all files stored in the directory
    the files are shifted from high indexes to low indexes in a way such
    that all blocks from the beginning of the directory are covered, leaving
    all available empty space at the end of the directory, which makes enough
    space for new additions or extensions.

    The compaction is implemented to work in linear time in terms of directory
    space. Also, it does not require any additional memory, as shifts of blocks
    all are being done in place block by block.

    A start index is given to the function because in some cases we will need to
    apply compaction on some suffix of the directory, this is specifically in
    cases where we apply compaction for extension, as after first compaction we
    want to move the file we want to extend to this end, which leaves its space
    empty, so we might want to do compaction again that case.
  */
  int ApplyCompaction(int start_index) {

    // last stores the index at which we expect to do
    // our next insertion
    int last = start_index;

    for (int i = start_index ; i < MAX_BLOCKS ; ++i) {

      if (Directory[i] == EMPTY) continue;

      int ID = Directory[i];

      // use move function to move the directory
      int status = Move(Directory[i], last);

      if (status == FAIL) {
        return status;
      }

      // after moving a file, increase the last index
      // by the length of the file, because that will
      // be the place at which the next file should be
      // inserted
      last += Table.GetFile(ID).block_len;
    }

    return SUCCESS;
  }

  /*
    This function checks if it is possible to extend a file
    to a certain region starting at the given index and has
    length of the amount given
  */
  bool CanExtend(int index, int amount) {

    for (int i = index ; i < index + amount ; ++i) {

      if (Directory[i] != EMPTY) return false;
    }

    return true;
  }

  /*
    This function attempts to find a space for a file of a
    given block length, it returns the index of the first
    spot found that fits the required number of blocks
  */
  int FindAvailableSpace(int block_num) {

    for (int i = 0, j ; i < MAX_BLOCKS ; i = j) {

      j = i + 1;

      if (Directory[i] != EMPTY) continue;

      // if an empty spot is found, check if there are block_num
      // consecutive empty spots starting from that one
      while (j < MAX_BLOCKS && Directory[j] == EMPTY) {

        // if enough space is reached, quit the loop
        if (j - i == block_num) break;

        j++;
      }

      if (j - i == block_num) return i;
    }

    return FAIL;
  }

  /*
    This function places a file in the directory by filling
    Directory blocks with the fileID of the file being inserted
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

  /*
    This function does the opposite of Fill, it makes directory blocks
    occupied by some file empty. Spots being emptied are expected to
    be occupied
  */
  int Empty(int index, int length) {

    for (int i = index ; i < index + length ; ++i) {

      if (Directory[i] == EMPTY) {
        Logger.Log("Empty", "Attempting to empty an already Empty spot, this should not happen");
        return FAIL;
      }

      Directory[i] = EMPTY;
    }

    return SUCCESS;
  }

  /*
    This function creates a new file.
  */
  int CreateFile(int fileID, int file_length) {

    // if a file with such fileID exists, abort creation
    if (Table.FileExists(fileID)) {
      Logger.Log("create_file", "Cannot create a file that already exists");
      return FAIL;
    }

    int block_num = ByteToBlock(file_length);

    // if there is no enough space, reject creation
    if (block_num > available_space) {
      Logger.Log("CreateFile", "Creation Rejected due to insufficient space");
      return REJECT;
    }

    // find an available spot to insert the file
    int index = FindAvailableSpace(block_num), status;

    // if no enough space is found, apply compaction to obtain space
    // it is guaranteed to find enough space after compaction because
    // we check at the beginning of total available space is enough
    if (index == FAIL) {

      ApplyCompaction(DIRECTORY_START);

      // this represents the first empty index after compaction
      // it will for sure be the place at which we can insert the file
      index = MAX_BLOCKS - available_space;
    }

    // add file to Directory Table
    status = Table.AddFile(fileID, File(index, block_num, file_length));

    if (status == FAIL) {
      return FAIL;
    }

    // fill in the blocks in Directory
    status = Fill(fileID, index, block_num);

    if (status == FAIL) {
      return FAIL;
    }

    // update available space
    available_space -= block_num;

    return SUCCESS;
  }

  /*
    This function takes a file ID and a byte offset of that file, and
    it returns the index of the block at which given offset of the
    file is stored in the directory
  */
  int Access(int fileID, int byte_offset) {

    // if not such file exist, the operation fails
    if (!Table.FileExists(fileID)) {
      Logger.Log("Access", "Cannot access file that does not exist");
      return FAIL;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    // if byte offset exceeds the file size, abort access operation
    if (F.byte_len < byte_offset) {
      Logger.Log("Access", "Byte offset to be accessed exceeds actual file size");
      return FAIL;
    }

    // see at which block of the file this offset exists
    int block_offset = ByteToBlock(byte_offset);

    // starting from the file index, if we add block
    // offset we will need to subtract 1 as the first
    // index of the file already contains the first block
    return F.index + block_offset - 1;
  }

  /*
    This function takes a file and an extension amount, and it
    extends the desired file by the number of blocks equal to
    the given amount
  */
  int Extend(int fileID, int extension_amount) {

    // if such file does not exist, the operation fails
    if (!Table.FileExists(fileID)) {
      Logger.Log("Extend", "Cannot extend file that does not exist");
      return FAIL;
    }

    // if no enough space for extension, operation is rejected
    if (available_space < extension_amount) {
      Logger.Log("Extend", "Extension Rejected due to insufficient space");
      return REJECT;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) return FAIL;

    // if current space is enough to extend, extend it already
    // if not, then apply compaction and extend
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

      // currently I don't know why, but after compaction and extension,
      // I am applying compaction again, this does not seems to be necessary
      // but I shall deal with it after starting experimentation to see
      // its effects
      ApplyCompaction(old_index);
    }

    // update Directory table with new block and byte lengths
    Table.UpdateBlockLen(fileID, F.block_len + extension_amount);
    Table.UpdateByteLen(fileID, F.byte_len + block_size * extension_amount);

    // update available_space
    available_space -= extension_amount;

    return SUCCESS;
  }

  /*
    This function shrinks a file by a given amount. If shrink values
    makes the number of blocks zero, then this file is removed from
    the directory
  */
  int Shrink(int fileID, int shrink_amount) {

    // if such file does not exists, the operation fails
    if (!Table.FileExists(fileID)) {
      Logger.Log("Shrink", "Cannot shrink file that does not exist");
      return FAIL;
    }

    // shirnk amount cannot be zero
    if (shrink_amount == 0) {
      Logger.Log("Shrink", "Shrink amount cannot be equal to zero");
      return FAIL;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    // shrink amount cannot be greater than block length of file
    if (F.block_len < shrink_amount) {
      Logger.Log("Shrink", "Shrink aborted because shrink amount is greater than file size");
      return FAIL;
    }

    int blocks_left = F.block_len - shrink_amount;

    // release the blocks of the directory
    int status = Empty(F.index + blocks_left, shrink_amount);

    if (status == FAIL) {
      return status;
    }

    // if blocks left for the file are equal to zero
    // then remove this file from directory, otherwise,
    // update the lengths in Directory Table
    if (blocks_left == 0) {

      status = Table.RemoveFile(fileID);

      if (status == FAIL) {
        return FAIL;
      }

    } else {

      Table.UpdateBlockLen(fileID, blocks_left);
      Table.UpdateByteLen(fileID, F.byte_len - block_size * shrink_amount);
    }

    // update available space
    available_space += shrink_amount;

    return SUCCESS;
  }

  /*
    Prints a slice of the Directory, used for debugging
  */
  void PrintSlice(int l, int r) {

    for (int i = l ; i < r ; ++i) {
      cout << Directory[i] << endl;
    }
  }

  /*
    Prints the Directory table contents, used for debugging
  */
  void ExploreTable() {

    for (auto el : Table.Table) {
      File F = el.second;
      cout << el.first << " : " << F.index << " " << F.block_len << " " << F.byte_len << endl;
    }
  }

};

/*
  This struct type represents a file that is used in linked allocation.
  Its attributes:

  state: can be either empty or full
  next:   stores the ID of the next 
  ID:     stored the ID of the linked file
*/
struct LinkedFile {

  int state;
  int next;
  int ID;

  LinkedFile(): ID(-1), next(END_OF_FILE), state(EMPTY) {}
  LinkedFile(int _ID): ID(_ID), next(END_OF_FILE), state(EMPTY) {}

  /*
    This function changes the state of the File block to occupied
  */
  void Fill(int _ID) {
    state = _ID;
    ID = _ID;
  } 

  /*
    This function changes the state of the File block to empty
  */
  void Empty() {
    state = EMPTY;
    ID = NULL_ID;
    next = END_OF_FILE;
  }

  /*
    This function is used to update the next pointer 
  */
  void UpdateNext(int new_next) {
    next = new_next;
  }

  /*
    This function is used to update the state to a new given one.
  */
  void Updatestate(int new_state) {
    state = new_state;
  }

};

/*
  This struct type encpsulates a file system implemented using
  Linked Allocation. Its Attributes:

  block_size:       the size of the block
  available_space:  stores the number of blocks remaning empty
  Table:            stores Directory Table
  Directory:        stores list of Directory where each block is 
                    a LinkedFile instance
*/
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

  /*
    Takes length of bytes and returns number of blocks required
    to store these bytes, which is taken by dividing length by
    block size and taking the ceil value
  */
  int ByteToBlock(int length) {

    return (length + block_size - 1) / block_size;
  }

  /*
    This function attempts to find a space for a file of a
    given block length, it returns a list of indexes which
    are available to store the file
  */
  vector<int> FindAvailableSpace(int block_num) {

    vector<int> Res;

    for (int i = 0 ; i < MAX_BLOCKS ; ++i) {
      
      if (Directory[i].state == EMPTY) {
        Res.push_back(i);
      }

      if (Res.size() == block_num) return Res;
    }

    return Res;
  }

  /*
    This function creates a new file.
  */
  int CreateFile(int fileID, int file_length) {

    // if such file exists, the operation fails
    if (Table.FileExists(fileID)) {
      Logger.Log("create_file", "Cannot create a file that already exists");
      return FAIL;
    }

    int block_num = ByteToBlock(file_length);

    // if no available space, the operation is rejected
    if (block_num > available_space) {
      Logger.Log("CreateFile", "Creation Rejected due to insufficient space");
      return REJECT;
    }

    vector<int> space = FindAvailableSpace(block_num);

    // ensure that we got same number of spaces as required
    if (space.size() != block_num) {
      Logger.Log("CreateFile", "Number of Slots found does not match requested number.");
      return FAIL;
    }

    // iterate over spaces and fill them
    for (int i = 0 ; i < block_num ; ++i) {

      int x = space[i];

      // ensure that selected spaces are empty
      if (Directory[x].state != EMPTY) {
        Logger.Log("CreateFile", "Expected an empty space in directory, but it is not empty");
        return FAIL;
      }

      // fill block slot in directory
      Directory[x].Fill(fileID);

      // if not last block, update its next
      if (i + 1 < block_num) {

        int next = space[i + 1];
        Directory[x].UpdateNext(next);
      }
    }

    // add file to Directory Table
    int status = Table.AddFile(fileID, File(space[0], block_num, file_length));

    if (status == FAIL) {
      return status;
    }

    // update available space
    available_space -= block_num;

    return SUCCESS;
  }

  /*
    This function takes a file ID and a byte offset of that file, and
    it returns the index of the block at which given offset of the
    file is stored in the directory
  */
  int Access(int fileID, int byte_offset) {

    // if such file does not exist, operation fails
    if (!Table.FileExists(fileID)) {
      Logger.Log("Access", "Cannot access file that does not exist");
      return FAIL;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    // if byte offset is larger the file byte length, operation fails
    if (F.byte_len < byte_offset) {
      Logger.Log("Access", "Byte offset to be accessed exceeds actual file size");
      return FAIL;
    }

    int index = Table.GetFile(fileID).index;

    // keep moving from the start until we reach the block
    // which contains the required byte offset
    while (block_size < byte_offset) {
      byte_offset -= block_size;
      index = Directory[index].next;
    }

    return index;
  }

  /*
    This function takes a file and an extension amount, and it
    extends the desired file by the number of blocks equal to
    the given amount
  */
  int Extend(int fileID, int extension_amount) {

    // if such file does not exist, operation fails
    if (!Table.FileExists(fileID)) {
      Logger.Log("Extend", "Cannot extend file that does not exist");
      return FAIL;
    }

    // if no available space of extension, reject
    if (available_space < extension_amount) {
      Logger.Log("Extend", "Extension Rejected due to insufficient space");
      return REJECT;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    // find available indexes
    vector<int> space = FindAvailableSpace(extension_amount);

    // ensure that space slots found match required
    if (space.size() != extension_amount) {
      Logger.Log("Extend", "Number of Slots found does not match requested number.");
      return FAIL;
    }

    int index = F.index;

    // go to the last block in order to update its next
    while (Directory[index].next != END_OF_FILE) {
      index = Directory[index].next;
    }

    // set the next of the last block to its new next which
    // came to existence after extension
    Directory[index].next = space[0];

    for (int i = 0 ; i < space.size() ; ++i) {

      int x = space[i];

      // ensure chosen slot is empty
      if (Directory[x].state != EMPTY) {
        Logger.Log("CreateFile", "Expected an empty space in directory, but it is not empty");
        return FAIL;
      }

      Directory[x].Fill(fileID);

      // if not last block set its next pointer
      if (i + 1 < space.size()) {
        int next = space[i + 1];
        Directory[x].UpdateNext(next);
      }
    }

    // update lengths in Directory table
    Table.UpdateBlockLen(fileID, F.block_len + extension_amount);
    Table.UpdateByteLen(fileID, F.byte_len + block_size * extension_amount);

    // update available space
    available_space -= extension_amount;

    return SUCCESS;
  }

  /*
    This function shrinks a file by a given amount. If shrink values
    makes the number of blocks zero, then this file is removed from
    the directory
  */
  int Shrink(int fileID, int shrink_amount) {

    // if such file does not exist, operation fails
    if (!Table.FileExists(fileID)) {
      Logger.Log("Shrink", "Cannot shrink file that does not exist");
      return FAIL;
    }

    File F = Table.GetFile(fileID);

    if (F == NullFile) {
      return FAIL;
    }

    // shrink amoutn cannot exceed current length
    if (F.block_len < shrink_amount) {
      Logger.Log("Shrink", "Shrink aborted because shrink amount is greater than file size");
      return FAIL;
    }

    int blocks_left = F.block_len - shrink_amount;

    int index = F.index;

    // if blocks left is equal to zero, then file is removed from
    // directory
    if (blocks_left == 0) {

      int status = Table.RemoveFile(fileID);

      if (status == FAIL) return status;

    } else {

      // update lengths in Directory table
      Table.UpdateBlockLen(fileID, F.block_len - shrink_amount);
      Table.UpdateByteLen(fileID, F.byte_len - block_size * shrink_amount);

      // find the index of the last block that remains after shrinking
      for (int i = 0 ; i < blocks_left - 1 ; ++i) {
        index = Directory[index].next;
      }

      // set the next pointer of the last remaining block to end of file
      int next = Directory[index].next;
      Directory[index].UpdateNext(END_OF_FILE);
      index = next;
    }

    // remove all blocks that have been released after shrinking
    while (index != END_OF_FILE) {
      int next = Directory[index].next;
      Directory[index].Empty();
      index = next;
    }

    // update available space
    available_space += shrink_amount;

    return SUCCESS;
  }

  /*
    used for debugging
  */
  void PrintSlice(int l, int r) {

    for (int i = l ; i < r ; ++i) {
      cout << Directory[i].ID << endl;
    }
  }

  /*
    used for debugging
  */
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
