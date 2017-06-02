#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Version.hpp>
#include <SoapySDR/Errors.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#define _USE_MATH_DEFINES
#include <cmath>

#include "OSFilter.hxx"

void doExperiment(const std::string & out_fname, 
		  SoapySDR::Device * dev, 
		  SoapySDR::Stream * rxstr, 
		  SoDa::OSFilter & rx_filter, 
		  std::complex<float> * vec, std::complex<float> * filt_vec, 
		  int buf_size) 
{
  std::cout << boost::format("Performing experiment %s\n") % out_fname; 
  std::complex<float> * bp[1]; 
  int flags = 0; 
  long long timeNS = -1; 
  int stat; 
  
  for(int i = 0; i < 50; i++) {
    bp[0] = vec;     
    int togo = buf_size; 
    while(togo > 0) {
      stat = dev->readStream(rxstr, (void**) bp, togo, flags, timeNS, 100000);
      if(stat < 0) {
	std::cerr << boost::format("RX Error on stream : [%s]\n") 
	  % SoapySDR::errToStr(stat);
	break; 
      }
      else {
	togo -= stat; 
	bp[0] += stat;
      }
    }
    // run each rx buffer through the filter...
    rx_filter.apply(vec, filt_vec); 
  }
  
  // now dump the last RX buffer.
  std::ofstream ofs(out_fname.c_str());
  for(int i = 0; i < buf_size; i++) {
    ofs << boost::format("%d %g %g\n") % i % filt_vec[i].real() % filt_vec[i].imag(); 
  }
  ofs.close();
}

		   
			      
int main()
{
  SoapySDR::Device * dev; 
  int stat; 

  // set the clock to allow a 625 k sample/sec sample rate, as SoDaRadio does. 
  double master_clock_rate = 40.0e6; 
  // set the sample rate
  double sample_rate = 625000.0; 

  SoapySDR::KwargsList kwl = SoapySDR::Device::enumerate(std::string("driver=lime"));
  if(kwl.size() == 0) {
    std::cerr << "No LimeSDR device was found.\n";
    exit(-1);
  }

  dev = SoapySDR::Device::make(kwl[0]);

  dev->setMasterClockRate(master_clock_rate); 
  std::cerr << "Setting RX frequency to 144.295MHz" << std::endl; 
  dev->setFrequency(SOAPY_SDR_RX, 0, 144.295e6);
  dev->setSampleRate(SOAPY_SDR_RX, 0, sample_rate);
  dev->setAntenna(SOAPY_SDR_RX, 0, "LNAL");

  // set the gains for all stages.  
  dev->setGain(SOAPY_SDR_RX, 0, "LNA", 30.0);
  dev->setGain(SOAPY_SDR_RX, 0, "PGA", 19.0);
  dev->setGain(SOAPY_SDR_RX, 0, "TIA", 12.0);

  // setup a few buffers
  const int buf_size = 30000; 
  std::complex<float> vec[buf_size];
  std::complex<float> filt_vec[buf_size];   

  // sig gen is at 144.310 so build a filter to fit.
  float tx_offset = 144.310e6 - 144.295e6; 
  SoDa::OSFilter rx_filter(tx_offset * 0.8, tx_offset * 0.9, // low skirt
			   tx_offset * 1.1, tx_offset * 1.2, // high skirt
			   256,  // number of effective taps
			   1.0, sample_rate, 
			   buf_size);
  
  // create a streamer.
  std::vector<size_t> chans; 
  SoapySDR::Stream * rxstr = dev->setupStream(SOAPY_SDR_RX, "CF32", chans); 

  // activate the streamer. 
  stat = dev->activateStream(rxstr, 0, 0, 0); 
  if(stat < 0) {
    std::cerr << boost::format("RX activateStream got bad return stat = %d [%s]\n") 
      % stat % SoapySDR::errToStr(stat);
  }

  doExperiment(std::string("RX_IQ_AutoSettings.dat"), 
	       dev, rxstr, 
	       rx_filter, 
	       vec, filt_vec, buf_size);
  
  dev->setIQBalance(SOAPY_SDR_RX, 0, std::complex<double>(1.0, 1.0e-6));

  doExperiment(std::string("RX_IQ_1r0_0.dat"), 
	       dev, rxstr, 
	       rx_filter, 
	       vec, filt_vec, buf_size);
  

  // shutdown the stream and exit. 
  stat = dev->deactivateStream(rxstr, 0, 0); 
  if(stat < 0) {
    std::cerr << boost::format("deactivateStream (rx) got bad return stat = %d [%s]\n") 
      % stat % SoapySDR::errToStr(stat);
  }
  else {
    std::cerr << "Deactivated rx streamer.\n";
  }
  dev->closeStream(rxstr); 
  
  SoapySDR::Device::unmake(dev); 

  exit(0); 
}

