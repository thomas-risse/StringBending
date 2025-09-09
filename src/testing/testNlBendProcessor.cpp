#include <chrono>
#include <iostream>
#include <fstream>

#include "NlBendProcessor.h"

using namespace std::chrono;

int main(int argc, char const *argv[])
{
  // Samplerate
  float sr = 44100;
  float simDuration = 10;
  // Parameters
  int Nmodes  = 2;

  NlBendProcessor<double> proc(sr, Nmodes);

  Eigen::VectorXd Amps, Omegas, Decays;
  Amps = Eigen::VectorXd::Ones(Nmodes);
  Omegas = Eigen::VectorXd::Ones(Nmodes) * 2 * M_PI * 10;
  Decays = Eigen::VectorXd::Ones(Nmodes) * 10;

  proc.setLinearParameters(Amps, Omegas, Decays);

  Eigen::VectorXd out0 = Eigen::VectorXd::Zero(static_cast<int>(sr * simDuration));

  auto start = high_resolution_clock::now();

  Eigen::VectorXd in = Eigen::VectorXd::Ones(1);
  Eigen::VectorXd out = Eigen::VectorXd::Ones(1);
  for (int i = 0; i < sr*simDuration; i++) {
    if (i==0){
      proc.process(in, out);
    } else {
      in(0) = 0;
      proc.process(in, out);
    }
    out0(i) = out(0);
  }
  auto stop = high_resolution_clock::now();
  float rtRatio = (duration_cast<microseconds>(stop - start)).count() * 1e-6/ simDuration;
    
  std::cout << "Time Processor: " << rtRatio * 100 << "%" << std::endl;

  // Write output to file
  std::ofstream outfile("output.txt");
  if (outfile.is_open()) {
    for (int i = 0; i < out.size(); ++i) {
      outfile << out(i) << "\n";
    }
    outfile.close();
    std::cout << "Output written to output.txt" << std::endl;
  } else {
    std::cerr << "Failed to open output.txt for writing." << std::endl;
  }
  return 0;
}
