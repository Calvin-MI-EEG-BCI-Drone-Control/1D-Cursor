#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include "../iWorxDAQ_64/iwxDAQ.h"
#include <sqlite3.h>
#include <windows.h>

using namespace std;

// g++ DatasetCollector.cpp -o DatasetCollector -I../iWorxDAQ_64 -L../iWorxDAQ_64 -liwxDAQ -I$env:VCPKG_ROOT/installed/x64-windows/include -L$env:VCPKG_ROOT/installed/x64-windows/lib -lsqlite3

/** USAGE
 * ./DatasetCollector <database>
 */

class Trial {
public:
	char* name;
	unsigned trial;
	float time;

	// constructors
	Trial() : name(""), trial(0), time(0) {};
	Trial(char* name, unsigned trial, float time)
		: name(name), trial(trial), time(time) {};
};

/**
 * A callback for whenever an SQL query (sqlite3_exec()) returns values
 * @param argc: the number of values returned
 * @param argv: an array of the values returned
 * @param azColName: the name of the column each value in argv was returned from
 */
static int SQLcallback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0; i<argc; i++){
    //   printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    return 0;
}

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
 * @param { char* } trial: the name of the trial being recorded
 * @param { int } trialNum: the trial number for this trial
 * @param { float } time: duration of this trial (in seconds)
 *
 */
int runTrial(int num_channels_recorded, float speed, char* trial, float duration) {
	// VARIABLES AND CONSTANTS //
	// constants
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
	// There is a delay between when the (iWorx) device is started and when meaningful data is recorded.
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
			fprintf(stderr, "\nERROR: Invalid number of samples per channel (trial %s)\n", trial);
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
	// printf("\nTrial %s: %d samples aquired per channel (total of %d) over %f seconds\n", trial, num_samples_per_ch * RECORD_ITERATIONS, num_samples_per_ch * num_channels_recorded * RECORD_ITERATIONS, time);

	return 0;
}

int startRecording(char* LOG_FILE, char* CONFIG_FILE) {
	// SETUP RECORDING

	//displayInterface();

	startHardware(LOG_FILE);

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
	// TODO: listen for trial events infinitely
	// for (int i = 0; i < trialArrayLength; ++i) {
		// iRet = runTrial(num_channels_recorded, speed, trialArray[i].name, trialArray[i].time);
	// }
	// TEMP TESTING: record a single trial for 10 seconds
	iRet = runTrial(num_channels_recorded, speed, "testTrial", 10);


	// Stop Acquisition
	StopAcq();
	printf("\nAquisition Stopped");

	// Close the iWorx Device
	CloseIworxDevice();
	return 0;
}

int _tmain(int argc, char **argv)
{
	// constants for file names
	char* LOG_FILE = "iworx.log";
	char* CONFIG_FILE = "../iWorxSettings/IX-EEG-Impedance-Check.iwxset";

	// set up SQLite database
	sqlite3 *db;
	char *ErrMsg = 0;
	if( sqlite3_open(argv[1], &db) ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
    }
	
	// table to contain all values from the impedence check settings (IX-EEG-Impedance-Check.iwxset)
	/* NOTE: 
		The iworx documentation does not describe which values received from ReadDataFromDevice() 
		correspond to each lead on the EEG cap. The column labels in the database may not be accurate.
		They are educated guesses based on the information shown in LabScribe.

		The data from ReadDataFromDevice() has a shape of (19,). 18 electrodes, one ground.
		TODO: Verify this with the settings groups/files https://iworx.com/docs/labscribe/create-your-own-settings-file-and-settings-group/?v=0b3b97fa6688
	
		The last values (class and time):
		class: the class this datapoint belongs to
		time: the wall clock time when this data was recorded (when ReadDataFromDevice() was called) 
	*/
	char *impedenceTable = "";
	if (CONFIG_FILE == "../iWorxSettings/IX-EEG-Impedance-Check.iwxset") {
		impedenceTable = 
		"CREATE TABLE IF NOT EXISTS ImpMotorImagery ("
		"id INTEGER PRIMARY KEY, "
		"FP1 REAL, FP2 REAL, F7 REAL, F3 REAL, Fz REAL, F4 REAL, F8 REAL, "
		"T3 REAL, C3 REAL, Cz REAL, C4 REAL, T4 REAL, T5 REAL, "
		"P3 REAL, Pz REAL, P4 REAL, T6 REAL, O1 REAL, O2 REAL"
		"Ground REAL, "
		"class TEXT, time TEXT)";
	}
	// create a table if needed
	sqlite3_exec(db, impedenceTable, SQLcallback, 0, &ErrMsg);

	startRecording(LOG_FILE, CONFIG_FILE);
	
	sqlite3_close(db);

	return 0;
}