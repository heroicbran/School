#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
int main()
{

  //Show
  DIR *testdir = opendir("./");
  dirent *testd = readdir(testdir);
  cout << testd->d_name << endl;
  testd = readdir(testdir);
  cout << testd->d_name << endl;

  if (testd = readdir(testdir))
    cout << testd->d_name << endl;
  return 0;
}
