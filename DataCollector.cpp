#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "iWorxDAQ_64/iwxDAQ.h"
#include <MQTTClient.h>
#include <windows.h>

// g++ DataCollector.cpp -o DataCollector -I./iWorxDAQ_64  -L./iWorxDAQ_64 -liwxDAQ -I"$env:VCPKG_ROOT/installed/x64-windows/include" -L"$env:VCPKG_ROOT/installed/x64-windows/lib" -lpaho-mqtt3cs

// MQTT Constants
#define ADDRESS     "ssl://3d0ef2c001394874af7fdfa932b5e994.s1.eu.hivemq.cloud:8883"
#define CLIENTID    "EEG_Publisher"
#define SIZE_TOPIC	"EEG/size"
#define DATA_TOPIC	"EEG/data"
#define QOS         1
#define TIMEOUT     10000L

class Trial {
public:
	char* task;
	unsigned trial;
	float time;

	// constructors
	Trial() : task(""), trial(0), time(0) {};
	Trial(char* task, unsigned trial, float time)
		: task(task), trial(trial), time(time) {};
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

int startMQTT(MQTTClient* client) {
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    // Initialize the MQTT client
    if ((rc = MQTTClient_create(client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS) {
        // ensure that you include the dll for paho-mqtt3cs for ssl and pahomqttc for tcp. If using the wrong one, it will fail to create the client and return error -14 (or similar error)
        printf("Failed to create client, return code %d\n", rc);
        return -1;
    };
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = "C_Pub";
    conn_opts.password = "C_Pub";

    // Will fail to connect and return error code -6 if these ssl options are not set.
    MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
    ssl_opts.enableServerCertAuth = 0;
    // declare values for ssl options, here we use only the ones necessary for TLS, but you can optionally define a lot more
    // look here for an example: https://github.com/eclipse/paho.mqtt.c/blob/master/src/samples/paho_c_sub.c
    ssl_opts.verify = 1;
    ssl_opts.CApath = NULL;
    ssl_opts.keyStore = NULL;
    ssl_opts.trustStore = NULL;
    ssl_opts.privateKey = NULL;
    ssl_opts.privateKeyPassword = NULL;
    ssl_opts.enabledCipherSuites = NULL;

    // use TLS for a secure connection, "ssl_opts" includes TLS
    conn_opts.ssl = &ssl_opts;

    // Connect to the broker
    if ((rc = MQTTClient_connect(*client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        MQTTClient_destroy(client);
        // exit(EXIT_FAILURE);
        return -1;
    }
	return rc;
}

int endMQTT(MQTTClient* client) {
	// Disconnect from the broker
    MQTTClient_disconnect(*client, 10000);
    MQTTClient_destroy(client);
	return 0;
}

int publishData(MQTTClient client, MQTTClient_message pubmsg, float* sample, int sample_size) {
	int rc = MQTTCLIENT_SUCCESS;
	MQTTClient_deliveryToken token;
	pubmsg.payload = (void*)sample;
	pubmsg.payloadlen = sizeof(float) * sample_size;
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	
	rc = MQTTClient_publishMessage(client, DATA_TOPIC, &pubmsg, &token);
	if (rc != MQTTCLIENT_SUCCESS) {
		printf("Failed to publish message");
		printf("\nrc: %d", rc);
		MQTTClient_destroy(&client);
		exit(-1);
	}
	
	// printf("Publishing message: "); // debugging
	// for (int i = 0; i < 19; ++i) {
	// 	printf("%f, ", sample[i]);
	// }
	// printf("\n");

	rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
	if (rc != MQTTCLIENT_SUCCESS) {
		printf("Failed to complete message");
		MQTTClient_destroy(&client);
		exit(-1);
	}
	// printf("Message with delivery token %d delivered\n", token); // debugging
	return rc;
}

int publishSize(MQTTClient client, int size) {
	int rc = MQTTCLIENT_SUCCESS;
	MQTTClient_message sizemsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;
	sizemsg.payload = &size;
	sizemsg.payloadlen = sizeof(size);
	sizemsg.qos = QOS;
	sizemsg.retained = 0;
        
	rc = MQTTClient_publishMessage(client, SIZE_TOPIC, &sizemsg, &token);
	if (rc != MQTTCLIENT_SUCCESS) {
		printf("Failed to publish message");
		printf("\nrc: %d", rc);
		MQTTClient_destroy(&client);
		exit(-1);
	}
	// printf("Publishing message: %d\n", size); // debugging

	rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
	if (rc != MQTTCLIENT_SUCCESS) {
		printf("Failed to complete message");
		MQTTClient_destroy(&client);
		exit(-1);
	}
	printf("Message with delivery token %d delivered\n", token); // debugging
	return rc;
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
int runTrial(MQTTClient client, MQTTClient_message pubmsg, int num_channels_recorded, float speed, 
	bool SAVE_TO_FILE, char* task, unsigned trialNum, float duration, FILE* fout) {
	// PUBLISH NUMBER OF CHANNELS
	publishSize(client, num_channels_recorded);
	// RECORD DATA
	// Constants
	const int DATA_SIZE = 2000;
	const int RECORD_ITERATIONS = (duration * 1000) / speed; // number of seconds (in milliseconds) / sampling rate (in milliseconds)
	// variables for ReadDataFromDevice
	int num_samples_per_ch = 0;
	long trig_index = -1;
	char trig_string[256];
	float data[DATA_SIZE];
	// used to time when data is read from the device.
	time_t record_time; 
	int read_num = 0;
	// make column labels for output file
	if (SAVE_TO_FILE)
	{
		for (unsigned i = 0; i < num_channels_recorded; ++i) {
		fprintf(fout, "Channel %u,", i);
		}
		fprintf(fout, "Condition,Trial #,Read_Num,Timestamp");
		fprintf(fout, "\n");
	}
	

	// Read Data and save it to file
	for (int i = 0; i < RECORD_ITERATIONS; ++i) {
		Sleep(speed); 		// NOTE: no samples are recorded for the first iteration (iter 0) unless there is a Sleep(). The status of ReadDataFromDevice returns -3.
		int iRet = ReadDataFromDevice(&num_samples_per_ch, &trig_index, trig_string, 256, data, DATA_SIZE); // Note: num_samples_per_ch is not consistent. I've seen from 9 to 400
		record_time = time(NULL);
		read_num++;
		if (num_samples_per_ch * num_channels_recorded > DATA_SIZE) printf("\nWARNING: amount of data recorded by ReadDataFromDevice() exceeds size of \"data\" buffer\n");
		// catch errors
		if (num_samples_per_ch < 0) {
			fprintf(stderr, "\nERROR: Invalid number of samples per channel (trial %u)\n", trialNum);
			return 1;
		}

		// save data to publish array
		float sample_array[num_channels_recorded];
		// for each sample within the recently read data
		for (int j = 0; j < num_samples_per_ch; ++j) {
			// for each channel within the sample
			for (int k = 0; k < num_channels_recorded; ++k) {
				unsigned index = j * num_channels_recorded + k;
				if (index < DATA_SIZE) {
					sample_array[k] = data[index]; // prepare data for MQTT stream
					if (SAVE_TO_FILE) fprintf(fout, "%f,", data[index]); // write to file
				}
			}
			// MQTT
			publishData(client, pubmsg, sample_array, num_channels_recorded); 
			// Write to file
			if (SAVE_TO_FILE) 
			{
				// metadata
				fprintf(fout, "%s,%u,%d,%s", task, trialNum, read_num, asctime(gmtime(&record_time)));
				fflush(fout);
			}
		}
		Sleep(speed); // was 100 by default, now set to sampling speed
	}

	// summarize results
	// printf("\nTrial %u of %s: %d samples aquired per channel (total of %d) over %f seconds\n", trialNum, task, num_samples_per_ch * RECORD_ITERATIONS, num_samples_per_ch * num_channels_recorded * RECORD_ITERATIONS, time);

	return 0;
}

int runDemo(char* LOG_FILE, char* OUTPUT_FILE, char* FILE_MODE, char* CONFIG_FILE, bool SAVE_TO_FILE, Trial* trialArray, unsigned trialArrayLength) {
	//displayInterface();

	startHardware(LOG_FILE);

	// SETUP DEMO
	FILE* fout;
	if (SAVE_TO_FILE) 
	{
		fout = _tfopen(OUTPUT_FILE, FILE_MODE);
		if (fout == NULL) {
			perror("\nERROR: unable to open file");
			return 1;
		}
	}
	
	// Setup client and connect to MQTT broker
	MQTTClient client;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	startMQTT(&client);

	int iRet = LoadConfiguration(CONFIG_FILE); // Load a settings file that has been created with LabScribe
	if (iRet != 0) {
		perror("\nERROR: Failure to load configuration");
		CloseIworxDevice();
		endMQTT(&client);
		return 1;
	}

	//Get current sampling speed and num of channels
	int num_channels_recorded;
	float speed;
	int sampInfo = GetCurrentSamplingInfo(&speed, &num_channels_recorded);

	// Start Acquisition
	iRet = StartAcq(speed * num_channels_recorded);
	if (iRet != 0) {
		return 1;
	}

	// TRIALS
	for (int i = 0; i < trialArrayLength; ++i) {
		iRet = runTrial(client, pubmsg, num_channels_recorded, speed, SAVE_TO_FILE, trialArray[i].task, trialArray[i].trial, trialArray[i].time, fout);
	}


	// Stop Acquisition
	StopAcq();
	printf("\nAquisition Stopped");
	// Close the file
	// fclose(fout);

	// Close the iWorx Device
	CloseIworxDevice();
	// disconnect from broker and destroy client
	endMQTT(&client);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// constants for file names
	char* LOG_FILE = "iworx.log";
	char* OUTPUT_FILE = "output_file.csv";
	char* FILE_MODE = "w";
	char* CONFIG_FILE = "iWorxSettings/IX-EEG-Impedance-Check.iwxset";
	bool SAVE_TO_FILE = true;

	// Array of trials
	const unsigned TRIAL_ARRAY_LENGTH = 1;
	Trial trials[TRIAL_ARRAY_LENGTH];
	// trial/condition name, trial number, recording length (time) 
	trials[0] = Trial("demo", 0, 10);


	runDemo(LOG_FILE, OUTPUT_FILE, FILE_MODE, CONFIG_FILE, SAVE_TO_FILE, trials, TRIAL_ARRAY_LENGTH);
	
	return 0;
}
