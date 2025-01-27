#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include <time.h>
#include <algorithm>
#include <functional>

#include "../iWorxDAQ_64/iwxDAQ.h"
#include "GUIs/ExperimentGUI.h"
#include "GUIs/SubjectGUI.h"
#include <sqlite3.h>
#include <windows.h>

static int SQLcallback(void *NotUsed, int argc, char **argv, char **azColName);
static int handleSQLErrors(int returnCode, char *ErrorMessage);
extern int startHardware(char* logfile);
extern int collectData(sqlite3 *db, int num_channels_recorded, float speed);
extern int startRecording(sqlite3 *db);
extern void dataCollector(int agrc, char **argv);

#endif
