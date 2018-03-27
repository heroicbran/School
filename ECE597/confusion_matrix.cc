#include <iostream>
#include <fstream>       //istream for read (input from file)
#include <vector>
using namespace std;


vector< vector<int> > generate_cmatrix(vector < vector<int> > cmatrix)     //Be sure to pass files by reference
{
  ifstream in;
  string p; //predicted value
  string a; //actual value
  int i,j; //indexes for matrix manipulation
  in.open("training_data");
  getline(in,p);        //Skip the file comment (header)

  while(getline(in,p,';')) //Reading predicted value into variable
  {
    getline(in,a);   //Reading actual value into variable

    if (p == "SITTINGDOWN")
      i = 0;
    else
      i = 1;

    if (a == "SITTINGDOWN")
      j = 0;
    else
      j = 1;

    cmatrix[i][j]++;
  }

  return cmatrix;
}

void output_matrix()
{

  //OUTPUT VECTOR INFO;
}


int main()
{
  vector< vector<int> > cmatrix;
  //cmatrix[0].size() = 4;
  generate_cmatrix(cmatrix);

  return 0;
}
