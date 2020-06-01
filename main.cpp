#include "file_data_structures.h"
#include <sstream>
#include <fstream>
#include <chrono>
#include <ctime>

#define TimePoint chrono::_V2::system_clock::time_point
#define TimeNow chrono::system_clock::now
#define duration chrono::duration
#define INPUT_N 5
#define REP 5

/*
  These structs below are used to modularize the handling of calls
  when splitting and parsing the files
*/

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

/*
  This type is used to collect the experiment result, and it additionally provides
  some functions to facilitate applying arithmetic operations on the data, as well
  as printing
*/
struct Results {

  double create_rejects;
  double extend_rejects;
  double run_time = 0.0;
  double access_time = 0.0;
  double create_time = 0.0;
  double extend_time = 0.0;
  double shrink_time = 0.0;
  double access_failure = 0.0;

  Results(): create_rejects(0.0), extend_rejects(0.0) {}
  Results(int cr, int er, int rt): create_rejects(cr), extend_rejects(er), run_time(rt) {}

  Results operator+ (const Results& Res) const {

    Results R;
    R.create_rejects = create_rejects + Res.create_rejects;
    R.extend_rejects = extend_rejects + Res.extend_rejects;
    R.run_time = run_time + Res.run_time;
    R.access_time = access_time + Res.access_time;
    R.create_time = create_time + Res.create_time;
    R.extend_time = extend_time + Res.extend_time;
    R.shrink_time = shrink_time + Res.shrink_time;
    R.access_failure = access_failure + Res.access_failure;

    return R;
  }

  void Div(double num) {

    create_rejects /= num;
    extend_rejects /= num;
    run_time /= num;
    access_time /= num;
    create_time /= num;
    extend_time /= num;
    shrink_time /= num;
    access_failure /= num;
  }

  void Add(Results Res) {

    create_rejects += Res.create_rejects;
    extend_rejects += Res.extend_rejects;
    run_time += Res.run_time;
    access_time += Res.access_time;
    create_time += Res.create_time;
    extend_time += Res.extend_time;
    shrink_time += Res.shrink_time;
    access_failure += Res.access_failure;
  }

  void Print(string title) {

    cout << title << endl;

    cout << "Avg Creation Rejections: " << create_rejects << endl;
    cout << "Avg Extension Rejections: " << extend_rejects << endl;
    cout << "Avg Total Runtime of input: " << run_time << " (ms)" << endl;
    cout << "Avg Creation Time: " << create_time << " (ms)" << endl;
    cout << "Avg Access Time: " << access_time << " (ms)" << endl;
    cout << "Avg Extension Time: " << extend_time << " (ms)" << endl;
    cout << "Avg Shrink Time: " << shrink_time << " (ms)" << endl;
    cout << "Avg Access failure: " << access_failure << endl;
    puts("");
  }
};

/*
  storing the paths to the input files
*/
string InputFiles[] = {
  "io/input_8_600_5_5_0.txt",
  "io/input_1024_200_5_9_9.txt",
  "io/input_1024_200_9_0_0.txt",
  "io/input_1024_200_9_0_9.txt",
  "io/input_2048_600_5_5_0.txt",
};

/*
  storing block sizes for each file
*/
int BlockSizes[] = {8, 1024, 1024, 1024, 2048};

int ID = 1;

GeneralLogger Logger = GeneralLogger("Experiment");

/*
  This is used to generate IDs for files
*/
int GetID() {
  return ID++;
}

void ResetID() {
  ID = 1;
}

/*
  utitlity to compute time durations
*/
double GetDuration(TimePoint L, TimePoint R) {

  duration<double> D = R - L;

  return D.count();
}

/*
  converse a string to an integer, useful for parsing
*/
int ToInt(string s) {

  stringstream str(s);

  int x = 0;
  str >> x;

  return x;
}

/*
  Given a string and a delimiter, split the string
  according to the delimeter, and return the chunks
  as a vector of strings
*/
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

/*
  This function takes a reference to an allocation method
  and a file name, and it runs the experiment on the given
  input file. It collects all metrics and returns those as
  a Results instance
*/
Results RunExperiment(Allocation& A, string file_name) {

  ResetID();


  ifstream inFile;

  inFile.open(file_name);

  if (!inFile) {
    cerr << "File Open Failed\n";
  }

  // counting occurence of calls to get average
  int create_count = 0;
  int access_count = 0;
  int extend_count = 0;
  int shrink_count = 0;

  string line;

  Results Res;

  TimePoint l_total = TimeNow();

  while (inFile >> line) {

    // parse line
    vector<string> Args = Split(line, ':');

    // creation call case

    if (Args[0] == "c") {

      CreateCall call = CreateCall(ToInt(Args[1]));

      int ID = GetID();

      TimePoint l_time = TimeNow();

      int status = A.CreateFile(ID, call.bytes);

      TimePoint r_time = TimeNow();

      if (status == REJECT) Res.create_rejects++;

      if (status == FAIL) {
        Logger.LogIssue("CreateCall", "Creation Failed: " + Args[1]);
      }

      Res.create_time += GetDuration(l_time, r_time);

      create_count++;

      continue;
    }

    // access call case

    if (Args[0] == "a") {

      vector<string> Args = Split(line, ':');

      AccessCall call = AccessCall(ToInt(Args[1]) + 1, ToInt(Args[2]));

      TimePoint l_time = TimeNow();

      int index = A.Access(call.fileID, call.offset);

      TimePoint r_time = TimeNow();

      if (index == FAIL) {
        Logger.LogInfo("AccessCall", "Access Failed: " + Args[1] + " " + Args[2]);
        Res.access_failure++;
      }

      Res.access_time += GetDuration(l_time, r_time);
      access_count++;

      continue;
    }

    // extension case

    if (Args[0] == "e") {

      vector<string> Args = Split(line, ':');

      ExtendCall call = ExtendCall(ToInt(Args[1]) + 1, ToInt(Args[2]));

      TimePoint l_time = TimeNow();

      int status = A.Extend(call.fileID, call.extension_amount);

      TimePoint r_time = TimeNow();

      if (status == REJECT) {
        Res.extend_rejects++;
      }

      if (status == FAIL) {
        Logger.LogIssue("Extend", "Extension Failed: " + Args[1] + " " + Args[2]);
      }

      Res.extend_time += GetDuration(l_time, r_time);

      extend_count++;

      continue;
    }

    // shrink case

    if (Args[0] == "sh") {

      vector<string> Args = Split(line, ':');

      ShrinkCall call = ShrinkCall(ToInt(Args[1]) + 1, ToInt(Args[2]));

      TimePoint l_time = TimeNow();

      int status = A.Shrink(call.fileID, call.shrink_amount);

      TimePoint r_time = TimeNow();

      if (status == FAIL) {
        Logger.LogIssue("Shrink", "Shrink failed: " + Args[1] + " " + Args[2]);
      }

      Res.shrink_time += GetDuration(l_time, r_time);

      shrink_count++;

      continue;
    }

    cerr << "Invalid Line Input\n";
    assert(false);
  }

  TimePoint r_total = TimeNow();

  // divide by occurence of each call to get average

  if (create_count != 0) Res.create_time /= create_count;
  if (access_count != 0) Res.access_time /= access_count;
  if (extend_count != 0) Res.extend_time /= extend_count;
  if (shrink_count != 0) Res.shrink_time /= shrink_count;

  Res.run_time = GetDuration(l_total, r_total);

  return Res;
}

void Log(string s) {

  cout << s << endl;
}


int main() {

  puts("It Has Begun");

  // this will be used to store outputs of all files

  Results ContigRes[INPUT_N];
  Results LinkedRes[INPUT_N];

  // run contiguous allocation 

  for (int i = 0 ; i < INPUT_N ; ++i) {

    string file_path = InputFiles[i];
    int block_size = BlockSizes[i];

    for (int j = 0 ; j < REP ; ++j) {

      Log("Contig: File " + to_string(i) + " Attempt " + to_string(j));

      ContiguousAllocation CA(block_size);

      Results Res = RunExperiment(CA, file_path);

      ContigRes[i].Add(Res);
    }

    ContigRes[i].Div(REP);
  }

  // run linked allocation

  for (int i = 0 ; i < INPUT_N ; ++i) {

    string file_path = InputFiles[i];
    int block_size = BlockSizes[i];

    for (int j = 0 ; j < REP ; ++j) {

      Log("Linked: File " + to_string(i) + " Attempt " + to_string(j));

      LinkedAllocation LA(block_size);

      Results Res = RunExperiment(LA, file_path);

      LinkedRes[i].Add(Res);
    }

    LinkedRes[i].Div(REP);
  }

  // print results

  for (int i = 0 ; i < INPUT_N ; ++i) {

    ContigRes[i].Print("Contiguous Results for file " + to_string(i));

    LinkedRes[i].Print("Linked Results for file " + to_string(i));
  }

}
