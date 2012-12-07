
// C headers
#include <stdlib.h>

// C++ headers
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>

// CGM headers
#include "Comm.h"
#include "BasicCommObject.h"

// Local headers
// TODO?

using std::cout;
using std::endl;
using std::ios_base;
using std::ifstream;
using std::ofstream;
using std::stringstream;

//==== Process management related ====//
// Number of processes
static int p;
// ID of this process
static int id;

//==== Communication related ====//
// Communication object
static Comm                     *comm = NULL;
// Zero
static BasicCommObject<double>  zero(0.0);

//==== Algorithm related ====//
// Cost matrix size
static unsigned                   n;
// Size of cost blocks
static unsigned                   block_size;
// Dimensions of the matrix chain
static vector<unsigned>           dims;
// Matrix of costs
static vector< vector<unsigned> > costs;

// Output
static ofstream out;

// Get data from CommObject
static double get_data (CommObject* obj) {
  return dynamic_cast<BasicCommObject<double>*>(obj)->getData();
}

// Set data to CommObject
static void set_data (CommObject* obj, double value) {
  dynamic_cast<BasicCommObject<double>*>(obj)->setData(value);
}

// Load matrix dimensions from input file
static void load_input (const string& inputfile) {
  ifstream input(inputfile.c_str());
  if (input.fail()) {
    cout << "[Proccess " << p << "] Aborting due to invalid input." << endl;
    exit(EXIT_FAILURE);
  }
  while (input.good()) {
    double dim;
    input >> dim;
    dims.push_back(dim);
  }
  input.close();
}

// Sends block to process above.
static void send_block (unsigned block) {
  CommObjectList  data(&zero);
  unsigned        width = (block < p-id-1 ? block_size : n % block_size),
                  height = block*block_size+width;
  data.setSize(width*height);
  for (unsigned i = 0; i < height; ++i)
    for (unsigned j = 0; j < width; ++j)
      set_data(data[i*width+j], costs[id*block_size+i][block*block_size+j]);
  comm->send(id-1, data, 0);
}

// Receives block from process below
static void receive_block (unsigned block) {
  CommObjectList  data(&zero);
  unsigned        width = (block < p-id-1 ? block_size : n % block_size),
                  height = (block-1)*block_size+width;
  int             actual_source = -1;
  int num = comm->receive(id+1, data, 0, &actual_source);
  for (unsigned i = 0; i < height; ++i)
    for (unsigned j = 0; j < width; ++j)
      costs[(id+1)*block_size+i][block*block_size+j] =
        get_data(data[i*width+j]);
}

// Computes multiplication costs in block
static void compute_block (unsigned block) {

}

static void cleanup () {
  comm->dispose ();
  out.close();
}

int main (int argc, char **argv) {
  // Startup values
  comm  = Comm::getComm (&argc, &argv, NULL);
  p     = comm->getNumberOfProcessors ();
  id    = comm->getMyId ();
  stringstream outputname;
  outputname << "out-" << id;
  out.open(outputname.str().c_str(), ios_base::out);
  atexit(cleanup);

  // Arg check
  if (argc != 2) {
    cout << "[Proccess " << p << "] Aborting due to invalid arguments." << endl;
    return EXIT_FAILURE;
  }

  // Arg handle
  load_input(argv[1]);

  // More startup values
  n           = dims.size()-2;
  block_size  = n/p;
  if (block_size*p < n) ++block_size;

  for (size_t i = 0; i < n; ++i)
    costs.push_back(vector<unsigned>(n, 0));

  //CommObjectList  data(&zero);
  //CommObjectList  response(&zero);
  //int             actual_source = -1;

  //data.setSize(1);
  //set_data(data[0], 1.0);

  for (int i = 0; i < p-id; ++i) {
    compute_block(i);
    if (id != Comm::PROC_ZERO)
      send_block(i);
      //comm->send(id-1, data, 0);
    if (i+1 < p-id)
      receive_block(i+1);
    //if (i+1 < p-id) {
    //  comm->receive(id+1, response, 0, &actual_source);
    //  double value = get_data(data[0]),
    //         inc = get_data(response[0]);
    //  set_data(data[0], value+inc);
    //}
  }

  // Output answer
  if (id == Comm::PROC_ZERO)
    out << costs.front().back() << endl;

  // Bye bye
  return EXIT_SUCCESS;

}

