
// C headers
#include <stdlib.h>

// C++ headers
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
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
using std::min;
using std::numeric_limits;

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
// Extra block size
static unsigned                   extra_size;
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
  out << "dims=";
  while (input.good()) {
    double dim;
    input >> dim;
    if (dim == 0) break;
    out << dim << " ";
    dims.push_back(dim);
  }
  out << endl;
  input.close();
}

// Sends block to process above.
static void send_block (unsigned block) {
  CommObjectList  data(&zero);
  unsigned        width = block_size + (block >= p-id-1)*extra_size,
                  height = block*block_size+width;
  data.setSize(width*height);
  out << "Sending " << width*height << endl;
  for (unsigned i = 0; i < height; ++i)
    for (unsigned j = 0; j < width; ++j)
      set_data(data[i*width+j], costs[id*block_size+i][(id+block)*block_size+j]);
  comm->send(id-1, data, 0);
}

// Receives block from process below
static void receive_block (unsigned block) {
  CommObjectList  data(&zero);
  unsigned        width =  block_size + (block >= p-id-1)*extra_size,
                  height = (block-1)*block_size+width;
  int             actual_source = -1;
  int num = comm->receive(id+1, data, 0, &actual_source);
  out << "Received " << num << " (expected " << width*height << ")" << endl;
  for (unsigned i = 0; i < height; ++i)
    for (unsigned j = 0; j < width; ++j)
      costs[(id+1)*block_size+i][(id+block)*block_size+j] =
        get_data(data[i*width+j]);
}

// Computes multiplication costs in block
static void compute_block (unsigned block) {
  unsigned start_i  = id*block_size,
           start_j  = start_i + block*block_size,
           end_i    = start_i + block_size + (id == p-1)*extra_size,
           end_j    = start_j + block_size + (block >= p-id-1)*extra_size;
  out << "Computing block [" << start_i << ".." << end_i-1 << ","
                             << start_j << ".." << end_j-1 << "]" << endl;
  for (unsigned i = end_i-1; i+1 >= start_i+1; --i) {
    for (unsigned j = start_j+(i-start_i+1)*!block; j < end_j; ++j) {
      costs[i][j] = numeric_limits<unsigned>::max();
      for (unsigned k = i; k < j; ++k) {
        //out << "step=" << k << endl;
        //out << "c[i][k]=" << costs[i][k] << endl;
        //out << "c[k+1][j]=" << costs[k+1][j] << endl;
        //out << "aux=" << (costs[i][k] + costs[k+1][j] + dims[i]*dims[k+1]*dims[j+1]) << endl;
        costs[i][j] = min(
          costs[i][k] + costs[k+1][j] + dims[i]*dims[k+1]*dims[j+1],
          costs[i][j]
        );
      }
      out << "costs[" << i << "," << j << "]=";
      out << costs[i][j] << endl;
    }
  }
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

  // Output file per process
  stringstream outputname;
  outputname << "out-" << id;
  out.open(outputname.str().c_str(), ios_base::out);

  // Setup cleanup callback
  atexit(cleanup);

  // Output
  out << "===================" << endl;
  out << "p=" << p << endl;
  out << "id=" << id << endl;

  // Arg check
  if (argc != 2) {
    cout << "[Proccess " << p << "] Aborting due to invalid arguments." << endl;
    return EXIT_FAILURE;
  }

  // Arg handle
  load_input(argv[1]);

  // More startup values
  n           = dims.size()-1;
  block_size  = n/p;
  extra_size  = n%p;

  // Output
  out << "n=" << n << endl;
  out << "blocksize=" << block_size << endl;
  out << "extrasize=" << extra_size << endl;

  // Build cost matrix
  for (size_t i = 0; i < n; ++i)
    costs.push_back(vector<unsigned>(n, 0));

  comm->synchronize();

  // Run parallel algorithm
  for (int i = 0; i < p-id; ++i) {
    compute_block(i);
    if (id != Comm::PROC_ZERO)
      send_block(i);
    if (i < p-id-1)
      receive_block(i+1);
  }

  // Output answer
  if (id == Comm::PROC_ZERO)
    out << costs.front().back() << endl;

  // Bye bye
  return EXIT_SUCCESS;

}

