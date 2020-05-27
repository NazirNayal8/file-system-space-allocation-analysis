#include "file_data_structures.h"
#include <sstream>
#include <fstream>
#include <chrono>
#include <ctime>

#define TimePoint chrono::_V2::system_clock::time_point
#define TimeNow chrono::system_clock::now
#define duration chrono::duration

struct Call {

  Call() {}
};

struct CreateCall : Call {
  int bytes;
  CreateCall(int _bytes): bytes(_bytes) {}
};

struct AccessCall : Call {
  int fileID;
  int offset;

  AccessCall(int _fileID, int _offset): fileID(_fileID), offset(_offset) {}
};

struct ExtendCall : Call {
  int fileID;
  int extension_amount;

  ExtendCall(int _fileID, int amount): fileID(_fileID), extension_amount(amount) {}
};

struct ShrinkCall : Call {
  int fileID;
  int shrink_amount;

  ShrinkCall(int _fileID, int amount): fileID(_fileID), shrink_amount(amount) {}
};

struct Results {

  int create_rejects;
  int extend_rejects;
  double run_time;

  Results(): create_rejects(0), extend_rejects(0), run_time(0.0) {}
  Results(int cr, int er, int rt): create_rejects(cr), extend_rejects(er), run_time(rt) {}
};

string InputFiles[] = {
  "io/input_8_600_5_5_0",
  "io/input_1024_200_5_9_9",
  "io/input_1024_200_9_0_0",
  "io/input_1024_200_9_0_9",
  "io/input_2048_600_5_5_0",
};


int BlockSizes[] = {8, 1024, 1024, 1024, 2048};

int ID = 1;

int GetID() {
  return ID++;
}

double GetDuration(TimePoint L, TimePoint R) {
  
  duration<double> D = R - L;
  
  return D.count();
}

int ToInt(string s) {

  stringstream str(s);

  int x = 0;
  str >> x;

  return x;
}

vector<string> Split(string str, char del) {

  vector<string> Res;

  int n = str.length(), i = 0;

  string s = "";

  while (i < n) {

    if (str[i] == del) {
      Res.push_back(s);
      s = "";
    } else {
      s += str[i];
    }
    i++;
  }

  if (s.length() > 0) Res.push_back(s);

  return Res;
}

Results RunExperiment(Allocation& A, string file_name, int block_size) {

  ifstream inFile;

  inFile.open(file_name);

  if(!inFile) {
    cerr << "File Open Failed\n";
  }

  string line;

  Results Res;

  while(inFile >> line) {

    vector<string> Args = Split(line, ':');

    if (Args[0] == "c") {
        
      CreateCall call = CreateCall(ToInt(Args[1]));

      int ID = GetID();
      int status = A.CreateFile(ID, call.bytes);

      continue;
    }

    if (Args[0] == "a") {
      
      continue;
    }

    if (Args[0] == "e") {
      
      continue;
    }

    if (Args[0] == "sh") {
      
      continue;
    }

    cerr << "Invalid Line Input\n";
    assert(false);
  }

  return Res;
}




int main() {



}
