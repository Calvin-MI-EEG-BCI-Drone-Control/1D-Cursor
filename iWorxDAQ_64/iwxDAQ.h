// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IWXDAQ_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IWXDAQ_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
/*******************************************************************************
********************************************************************************

   COPYRIGHT © 2014 iWorx Systems Inc

   Contains confidential and proprietary information which may not be copied,
   disclosed or used by others except as expressly authorized in writing
   by iWorx Systems Inc

   PM3DDI.h   External Interface Definitions for the PM3 Device Discovery & 
				  Interface DLL.

   About:     This module contains the DLL external interface definitions
				  that allow PC-based applications to communicate with 
				  iWorx Data recorders. 
				  

   Target:        Win32 Compatible

   Written for:   iWorx Systems Inc
				  62 Littleworth Road
				  Dover, NH 03820

   Contacts:      Ashish More
				  603-742-2492 x15
				  info@iworx.com

   Written by:    Ashish More

   $Revision: 1 $

--------------------------------------------------------------------------------

****************************************************************************/
#ifndef _IWXDAQ_H_
#define _IWXDAQ_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifdef IWXDAQ_EXPORTS
#define IWXDAQ_API __declspec(dllexport)
#else
#define IWXDAQ_API __declspec(dllimport)
#endif



const int STRING_BUFFER_SIZE = 256;
/****************************************************************************
   OpenIworxDevice

   About:      Opens an Iworx Device

   Inputs:     char *log_file       log_file is used to keep a log.

   Outputs:    Nothing

   Returns:    true if iworx device created

****************************************************************************/
IWXDAQ_API bool OpenIworxDevice(char *log_file);

/****************************************************************************
   CloseIworxDevice

   About:      Closes an Iworx Device

   Inputs:     Nothing

   Outputs:    Nothing

   Returns:    true if iworx device closed

****************************************************************************/
IWXDAQ_API bool CloseIworxDevice();

/****************************************************************************
   FindHardware

   About:      Finds an connected Iworx Device

   Inputs:     Nothing

   Outputs:    int model : model of the device present
			   char * name_buffer : stores the Name of the device
				int name_buffer_size : size of the name_buffer array
				char *sn_buffer : stores the serial number of the device.
				int sn_buffer_size : size of the sn_buffer array
				// bufferSize  should be  atleast 16 bytes long

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int FindHardware(int* model, char *name_buffer, int name_buffer_size,  char *sn_buffer, int sn_buffer_size); // bufferSize  should be  atleast 16 bytes long

/****************************************************************************
   GetCurrentSamplingInfo

   About:      Gets the current sampling information

   Inputs:     Nothing

   Outputs:    float* speed, : current sampling speed
				int* num_channels_recorded : number of channels being recorded.

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int GetCurrentSamplingInfo(float* speed, int* num_channels_recorded);
/****************************************************************************
   GetSamplingSpeed

   About:      Gets the possible sampling speeds for the device

   Inputs:     Nothing

   Outputs:    float* max_speed, 
				float* min_speed, 
				bool* simultaneous_sampling : true if each channel can be set to the max sampling speed
												false if the channels share the sampling speed , 

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int GetSamplingSpeed(float* max_speed, float* min_speed, bool* simultaneous_sampling);

/****************************************************************************
   GetNumChannels

   About:      Gets the number of channels available on the device

   Inputs:     Nothing

   Outputs:    int& analog_input_channels, 
				int& analog_output_channels, 
				int& digital_input_bits, 
				int& digital_output_bits

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int GetNumChannels(int* analog_input_channels, int* analog_output_channels, int* digital_input_bits, int* digital_output_bits);

/****************************************************************************
   GetNumAnalogInputModes

   About:      Get number of input modes. Input modes are setup in LabScribe

   Inputs:     unsigned int ch_index: Channel index

   Outputs:    int * num_ch_modes

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int GetNumAnalogInputModes(unsigned int ch_index, int * num_ch_modes);

/****************************************************************************
   GetAnalogInputInfo

   About:      Gets information about the analog input channel settings

   Inputs:     unsigned int ch_index, : Channel index

   Outputs:    
			    int mode_index, : index of the input mode, input modes are defined in LabScribe
				float& max_range, : Maximum range 
				float& min_range, : minimum range
				float& highpassfilter, : Highpass filter set for the channel
				float& lowpassfilter, : Lowpass filter set for the channel
				char* unit,       : Units for the channel
				unsigned int unit_bufferSize   : size of units string
   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int GetAnalogInputInfo(unsigned int ch_index, int mode_index, float* range_max, float* range_min, float* highpassfilter, float* lowpassfilter, char* unit, unsigned int unit_bufferSize, char* mode_name, unsigned int mode_bufferSize );

/****************************************************************************
   GetAnalogOutputVoltageRange

   About:      Gets information about the analog output channel settings

   Inputs:     unsigned int ch_index, : Channel index

   Outputs:    
				float* max_range, : Maximum range 
				float* min_range, : minimum range
				char* unit,       : Units for the channel
				unsigned int unit_bufferSize   : size of units string

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int GetAnalogOutputVoltageRange(unsigned int ch_index, float* max_range, float* min_range, char* unit, unsigned int unit_bufferSize);


// Acquisition
/****************************************************************************
   StartAcq

   About:      Start acquisition

   Inputs:     unsigned int data_buffer_size : size of internal buffer to use. 

   Outputs:    Nothing

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int StartAcq(unsigned int data_buffer_size);

/****************************************************************************
   StopAcq

   About:      Stop Acquisition

   Inputs:     Nothing

   Outputs:    Nothing

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int StopAcq();

/****************************************************************************
   ReadChannelData

   About:      Reads data for a channel from the device, this can be used to read channel data if a Null array is passed to ReadDataFromDevice.  
				ReadDataFromDevice  has to be called before calling this function.

   Inputs:     int ch_index: Channel index
				float* data : array to return the channel data
				unsigned int num_samples_to_read : number of samples to read, this should match the num_samples_acquired_per_channel received from the ReadDataFromDevice  function. 

   Outputs:    Nothing

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int ReadChannelData(int ch_index, float* data, unsigned int* num_samples);

/****************************************************************************
   ReadDataFromDevice

   About:      Reads data from the device. 

   Inputs:     float* data : array to hold the data read from the device. If this is NULL, ReadChannelData can be called for each channel data
               int data_size : size of data array

   Outputs:    int num_samples_acquired_per_channel : Number of samples of data acquired per channel
				long trigger_index : if a trigger was received, the trigger index will be positive. 
				char* trigger_str, : the type and text of the trigger received
				int trig_str_size  : size of the trigger string.

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int ReadDataFromDevice(int* num_samples_acquired_per_channel, long* trigger_index, char* trigger_str, int trig_str_size, float* data, int data_size);

//Analog and Digital Outputs
/****************************************************************************
   SetAnalogOutputValue

   About:      Set the output value of the stimulator.

   Inputs:     unsigned int ch_index : stimulator index
			   float value  : value to set the stimulator to

   Outputs:    Nothing

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int SetAnalogOutputValue(unsigned int ch_index, float value);

/****************************************************************************
   SetAnalogOutputSequence

   About:      Load an analog stimulator protocol from file

   Inputs:     unsigned int ch_index : stimulator channel 
			   char *filename  : file with the stimulator protocol

   Outputs:    Nothing

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int SetAnalogOutputSequence(unsigned int ch_index, char *filename);

/****************************************************************************
   RefireAnalogOutput

   About:      Refire the stimulator with the existing stimulation protocol.

   Inputs:     unsigned int ch_index : channel index of the stimulator to fire.

   Outputs:    Nothing

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int RefireAnalogOutput(unsigned int ch_index);

/****************************************************************************
   SetDigitalOutput

   About:      Sets the digital output value

   Inputs:     unsigned int value : Value to set the digital outputs to

   Outputs:    Nothing

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int SetDigitalOutput(unsigned int value);

// configuration
/****************************************************************************
   LoadConfiguration

   About:      Loads a settings file created by LabScribe that has all the configuration information for the hardware

   Inputs:     char *filename  : filename of the settings file to load

   Outputs:    Nothing

   Returns:    0 if Success
			   Error code if there is an error;

****************************************************************************/
IWXDAQ_API int LoadConfiguration(  char *filename);
//IWXDAQ_API int SaveConfiguration( char *filename, unsigned int bufferSize);

// Error Handling
//IWXDAQ_API const char*  GetError(int error_code );

#ifdef __cplusplus
}
#endif

#endif