#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include "../iWorxDAQ_64/iwxDAQ.h"
// #include <sqlite3.h>
#include <windows.h>

using namespace std;

// g++ DatasetCollector.cpp -o DataCollector -I../iWorxDAQ_64 -L../iWorxDAQ_64 -liwxDAQ -I$env:VCPKG_ROOT/installed/x64-windows/include -L$env:VCPKG_ROOT/installed/x64-windows/lib -lsqlite3

class Trial {
public:
	char* name;
	unsigned trial;
	float time;

	// constructors
	Trial() : name(""), trial(0), time(0) {};
	Trial(char* task, unsigned trial, float time)
		: name(name), trial(trial), time(time) {};
};

int displayInterface() {
	// opportunity to show information, ask for demographics info, etc.
	return 0;
}

int startHardware(char* logfile) {
	// Open the iWorx Device. If it fails, return an error.
	if (!OpenIworxDevice(logfile)) {
		perror("\nERROR: Unable to open iWorx device");
		return 1;
	}

	//Find Hardware
	int model;
	char name_buffer[1000], sn_buffer[1000];
	FindHardware(&model, name_buffer, 1000, sn_buffer, 1000); // bufferSize  should be  atleast 16 bytes long
	printf("Hardware found: %s\nSerial Number: %s\n", name_buffer, sn_buffer);
	if (model < 0) {
		perror("\nERROR: No Hardware Found");
		return 1;
	}
	return 0;
}

/* runTrial()
 * @param { FILE* } fout: the name of the (existing) output file
 * @param { int } num_channels_recorded: the number of channels to record from -- gotten from GetCurrentSamplingInfo()
 * @param { float } speed: the sampling speed -- gotten from GetCurrentSamplingInfo()
 * @param { char* } task: the name of the task being performed by the subject
 * @param { int } trialNum: the trial number for this task
 * @param { float } time: duration of this trial (in seconds)
 *
 */
int runTrial(int num_channels_recorded, float speed, bool SAVE_TO_FILE, char* task, unsigned trialNum, float duration) {
	// RECORD DATA
	// Constants
	const int DATA_SIZE = 2000; // maximum datapoints to collect per call to ReadDataFromDevice()
	const int RECORD_ITERATIONS = (duration * 1000) / speed; // number of seconds (in milliseconds) / sampling rate (in milliseconds)
	// variables for ReadDataFromDevice
	int num_samples_per_ch = 0;
	long trig_index = -1;
	char trig_string[256];
	float data[DATA_SIZE];
	// used to time when data is read from the device.
	time_t record_time; 
	int read_num = 0;

	/// READ DATA ///

	// Make sure we are not getting junk data. 
	// There is a delay between when the device is started and when meaningful data is recorded.
	int iRet = ReadDataFromDevice(&num_samples_per_ch, &trig_index, trig_string, 256, data, DATA_SIZE);
	unsigned total_datapoints = num_channels_recorded * num_samples_per_ch;
	// if the array is full of 0s, wait for data.
	while (std::all_of(data, data + total_datapoints, [](int x) { return x == 0; })) {
		// read more samples
		Sleep(speed);
		iRet = ReadDataFromDevice(&num_samples_per_ch, &trig_index, trig_string, 256, data, DATA_SIZE);
		total_datapoints = num_channels_recorded * num_samples_per_ch;
	}

	// main loop
	while (true) {
		// Sleep(speed); 		// NOTE: no samples are recorded for the first iteration (iter 0) unless there is a Sleep(). The status of ReadDataFromDevice returns -3.
		iRet = ReadDataFromDevice(&num_samples_per_ch, &trig_index, trig_string, 256, data, DATA_SIZE);
		record_time = time(NULL);
		read_num++;
		if (num_samples_per_ch * num_channels_recorded > DATA_SIZE) printf("\nWARNING: amount of data recorded by ReadDataFromDevice() exceeds size of \"data\" buffer\n");
		// catch errors
		if (num_samples_per_ch < 0) {
			fprintf(stderr, "\nERROR: Invalid number of samples per channel (trial %u)\n", trialNum);
			return 1;
		}

		// save data to publish array
		vector<float> sample_array(num_channels_recorded);
		// for each sample within the recently read data
		for (int j = 0; j < num_samples_per_ch; ++j) {
			// for each channel within the sample
			for (int k = 0; k < num_channels_recorded; ++k) {
				unsigned index = j * num_channels_recorded + k;
				if (index < DATA_SIZE) {
					data[index]; // the data that should be saved -- a sample
					// sample_array[k] = data[index]; // prepare data for MQTT stream
					// if (SAVE_TO_FILE) fprintf(fout, "%f,", data[index]); // write to file
				}
			}
			// MQTT
			// publishData(client, pubmsg, sample_array, num_channels_recorded); 
			// Write to file
			// if (SAVE_TO_FILE) 
			// {
			// 	// metadata
			// 	fprintf(fout, "%s,%u,%d,%s", task, trialNum, read_num, asctime(gmtime(&record_time)));
			// 	fflush(fout);
			// }
		}
		// Sleep(speed); // was 100 by default, now set to sampling speed
	}

	// summarize results
	// printf("\nTrial %u of %s: %d samples aquired per channel (total of %d) over %f seconds\n", trialNum, task, num_samples_per_ch * RECORD_ITERATIONS, num_samples_per_ch * num_channels_recorded * RECORD_ITERATIONS, time);

	return 0;
}

int runDemo(char* LOG_FILE, char* OUTPUT_FILE, char* FILE_MODE, char* CONFIG_FILE, bool SAVE_TO_FILE, Trial* trialArray, unsigned trialArrayLength) {
	//displayInterface();

	startHardware(LOG_FILE);

	// SETUP DEMO
	int iRet = LoadConfiguration(CONFIG_FILE); // Load a settings file that has been created with LabScribe
	if (iRet != 0) {
		perror("\nERROR: Failure to load configuration");
		CloseIworxDevice();
		return 1;
	}

	//Get current sampling speed and num of channels
	int num_channels_recorded;
	float speed;
	int sampInfo = GetCurrentSamplingInfo(&speed, &num_channels_recorded);

	// Start Acquisition
	iRet = StartAcq(speed * num_channels_recorded);
	if (iRet != 0) {
		perror("\nERROR: failed to start data acquisition");
		CloseIworxDevice();
		return 1;
	}

	// TRIALS
	for (int i = 0; i < trialArrayLength; ++i) {
		iRet = runTrial(num_channels_recorded, speed, SAVE_TO_FILE, trialArray[i].name, trialArray[i].trial, trialArray[i].time);
	}


	// Stop Acquisition
	StopAcq();
	printf("\nAquisition Stopped");
	// Close the file
	// fclose(fout);

	// Close the iWorx Device
	CloseIworxDevice();
	return 0;
}

int _tmain(int argc, char **argv)
{
	// sqlite3 *db;
	// constants for file names
	char* LOG_FILE = "iworx.log";
	char* OUTPUT_FILE = "output_file.csv";
	char* FILE_MODE = "w";
	char* CONFIG_FILE = "IX-EEG-Impedance-Check.iwxset";
	bool SAVE_TO_FILE = true;

	// Array of trials
	const unsigned TRIAL_ARRAY_LENGTH = 1;
	Trial trials[TRIAL_ARRAY_LENGTH];
	// trial/condition name, trial number, recording length (time) 
	trials[0] = Trial("demo", 0, 10);

	runDemo(LOG_FILE, OUTPUT_FILE, FILE_MODE, CONFIG_FILE, SAVE_TO_FILE, trials, TRIAL_ARRAY_LENGTH);
	
	return 0;
}