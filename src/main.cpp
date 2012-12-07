
// C headers
#include <stdlib.h>

// C++ headers
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

// CGM headers
#include "Comm.h"
#include "GeneralUtilities.h"

// Local headers
#include "utils.h"
#include "PairCommObject.h"

using std::cout;
using std::endl;
using std::ios_base;
using std::ofstream;

// Number of processes
static int p;
// ID of this process
static int id;

// Dimensions of the matrix chain
static vector<unsigned> dims;

// Zero
static BasicCommObject<double>  zero(0.0);

// Load matrix dimensions from input file
static void load_input (const string& intutfile);

static double get_data (CommObject* obj) {
  return dynamic_cast<BasicCommObject<double>*>(obj)->getData();
}

static void set_data (CommObject* obj, double value) {
  dynamic_cast<BasicCommObject<double>*>(obj)->setData(value);
}

int main (int argc, char **argv) {
  Comm *comm = Comm::getComm (&argc, &argv, NULL);
  p = comm->getNumberOfProcessors ();
  id = comm->getMyId ();

  if (argc != 2) {
    cout << "[Proccess " << p << "] Aborting due to invalid arguments." << endl;
    return EXIT_FAILURE;
  }

  load_input(argv[1]);

  CommObjectList  data(&zero);
  CommObjectList  response(&zero);
  int             actual_source = -1;

  data.setSize(1);
  set_data(data[0], 1.0);

  for (int i = 0; i < p-id; ++i) {
    //comm->synchronize();
    if (id != Comm::PROC_ZERO)
      comm->send(id-1, data, 0);
    if (i+1 < p-id) {
      comm->receive(id+1, response, 0, &actual_source);
      double value = get_data(data[0]),
             inc = get_data(response[0]);
      set_data(data[0], value+inc);
    }
  }

  if (id == Comm::PROC_ZERO) {
    ofstream out("out", ios_base::out);
    out << "HUZZAH: "
        << get_data(data[0])
        << endl;
    out.close();
  }

  comm->dispose ();

}

void load_input (const string& inputfile) {
  
}

