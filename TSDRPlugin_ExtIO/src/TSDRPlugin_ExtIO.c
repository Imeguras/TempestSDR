/*
#-------------------------------------------------------------------------------
# Copyright (c) 2014 Martin Marinov.
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the GNU Public License v3.0
# which accompanies this distribution, and is available at
# http://www.gnu.org/licenses/gpl.html
# 
# Contributors:
#     Martin Marinov - initial API and implementation
#-------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  
#include <pthread.h>
#include <setjmp.h>


#include "TSDRCodes.h"
#include "TSDRPlugin.h"
#include "ExtIOPluginLoader.h"

//#include <windows.h>

#include "errors.h"

#define EXTIO_HWTYPE_16B	3
#define EXTIO_HWTYPE_24B	5
#define EXTIO_HWTYPE_32B	6
#define EXTIO_HWTYPE_FLOAT	7

extiosource_t * source = NULL;
int hwtype;
tsdrplugin_readasync_function tsdr_cb;
void * tsdr_ctx;

volatile int is_running = 0;
int samplespercallback = 0;
float * outbuf = NULL;
int outbuf_size = 0;
int max_att_id = -1;

uint32_t req_freq = 100000000;
uint32_t act_freq = -1;
float req_gain = 0.5;
float act_gain = -1;
int hwopen = 0;

//HANDLE guisyncevent;
jmp_buf exceptionenv;
int errid = 0;
// This is a hack for handling WindowsAPI exceptions, when an exception is caught, the PC is
// transported back to where the setjump originally was and the call that causes  the exception
// is not invoked again.
/*LONG WINAPI exceptionhandler(struct _EXCEPTION_POINTERS *ExceptionInfo) {
	UNREFERENCED_PARAMETER(ExceptionInfo);

	errid++;
	if (errid == 1) longjmp(exceptionenv, errid);

	return EXCEPTION_CONTINUE_EXECUTION;
}*/
//TODO check things
void safecloseHw(){
	
	source->CloseHW();
	hwopen = 0;
	
}
/*void safecloseHw() {
	PVOID h = AddVectoredExceptionHandler(0, exceptionhandler);

	errid = 0;
	int statjump = setjmp(exceptionenv);
	if (statjump)
		source->CloseHW();
	hwopen = 0;

	RemoveVectoredExceptionHandler(h);
}*/

void closeextio(void) {

	if (source == NULL) return;

	if (source->HideGUI != NULL) source->HideGUI();
	if (hwopen) safecloseHw();
	extio_close(source);
	free(source);
	source = NULL;
}

void tsdrplugin_getName(char * name) {
	strcpy(name, "TSDR ExtIO Plugin");
}

uint32_t tsdrplugin_setsamplerate(uint32_t rate) {
	if (source != NULL) return source->GetHWSR();
	return 0;
}

uint32_t tsdrplugin_getsamplerate() {
	if (source != NULL) return source->GetHWSR();
	return 0;
}

int tsdrplugin_setbasefreq(uint32_t freq) {
	req_freq = freq;

	RETURN_OK();
}

int tsdrplugin_stop(void) {
	is_running = 0;

	RETURN_OK();
}

int tsdrplugin_setgain(float gain) {
	req_gain = gain;

	RETURN_OK();
}

void callback(int cnt, int status, float IQoffs, void *IQdata) {
	if (!is_running) return;
	if (status < 0 || cnt < 0)
		return;

	int i;

	switch (hwtype) {
	case EXTIO_HWTYPE_FLOAT:
		memcpy(outbuf, IQdata, samplespercallback * sizeof(float));
		break;
	case EXTIO_HWTYPE_16B:
	{
							 int16_t * buf = (int16_t *)IQdata;
							 for (i = 0; i < samplespercallback; i++)
								 outbuf[i] = *(buf++) / 32767.0f;
	}
		break;
	case EXTIO_HWTYPE_24B:
	{
							 uint8_t * buf = (uint8_t *)IQdata;
							 for (i = 0; i < samplespercallback; i++) {
								 const uint8_t first = *(buf++);
								 const uint8_t second = *(buf++);
								 const uint8_t third = *(buf++);

								 outbuf[i] = ((int16_t)(first | (second << 8) | (third << 16))) / 8388607.0f;
							 }
	}
		break;
	case EXTIO_HWTYPE_32B:
	{
							 int32_t * buf = (int32_t *)IQdata;
							 for (i = 0; i < samplespercallback; i++)
								 outbuf[i] = *(buf++) / 2147483647.0f;
	}
		break;
	}

	tsdr_cb(outbuf, samplespercallback, tsdr_ctx, 0);
}

//DWORD WINAPI doGuiStuff(LPVOID arg) {
uint32_t doGuiStuff(void* arg) {
	char * params = (char *)arg;

	if (outbuf == NULL) {
		outbuf = (float *)malloc(sizeof(float));
		outbuf_size = 1;
	}

	// if an extio was already initialized before, now change
	if (source != NULL)
		closeextio();

	// inititalize source
	source = (extiosource_t *)malloc(sizeof(extiosource_t));

	int status;
	if ((status = extio_load(source, params)) == TSDR_OK) {
		char name[200];
		char model[200];

		if (source->InitHW(name, model, &hwtype)) {
			source->SetCallback(&callback);

			if (hwopen) {
				hwopen = 0;
				safecloseHw(); 
			}

			if (hwtype != EXTIO_HWTYPE_16B && hwtype != EXTIO_HWTYPE_24B && hwtype != EXTIO_HWTYPE_32B && hwtype != EXTIO_HWTYPE_FLOAT) {
				closeextio();
				announceexception("The sample format of the ExtIO plugin is not supported.", TSDR_CANNOT_OPEN_DEVICE);
			} else if (source->OpenHW()) {
				hwopen = 1;

				if (source->ShowGUI != NULL) source->ShowGUI();

				// list attenuators
				if (source->GetAttenuators != NULL) {
					max_att_id = 0;
					float att;
					while (source->GetAttenuators(max_att_id++, &att) == 0) {};
				}

				errormsg_code = TSDR_OK;
			} else {
				closeextio();
				announceexception("The ExtIO driver failed to open a device. Make sure your device is plugged in and its drivers are installed correctly.", TSDR_CANNOT_OPEN_DEVICE);
			}
		}
		else {
			closeextio();
			announceexception("The ExtIO driver failed to initialize a device. Make sure your device is plugged in and its drivers are installed correctly.", TSDR_CANNOT_OPEN_DEVICE);
		}
	}
	else {

		if (status != TSDR_INCOMPATIBLE_PLUGIN)
			extio_close(source);

		free(source);
		source = NULL;

		if (status == TSDR_INCOMPATIBLE_PLUGIN)
			announceexception("The ExtIO dinamic library is not compatible with the current machine or does not exist. Please check the filename is correct and the file is a valid ExtIO dinamic library and try again.", TSDR_PLUGIN_PARAMETERS_WRONG);
		else
			announceexception("The provided library is not a valid/compatible ExtIO dinamic library. Please check the filename is correct and the file is a valid ExtIO dinamic library and try again.", TSDR_PLUGIN_PARAMETERS_WRONG);
	}

	// notify we have finished loading
	//SetEvent(guisyncevent);

	// do GUI handling
	/*Get the message?*/
	/*
	MSG msg;
	BOOL bRet;

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// error!
			return -1;
		}
		else
		{
			if (source == NULL) return 0;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	*/
	return 0;
}

//int    tsdrplugin_init(const char * params) {
int tsdrplugin_init(const char * params) {
	// create synchronization event
	//guisyncevent = CreateEvent(0, FALSE, FALSE, 0);
	pthread_t thread_id;
    
	// do the initialization in a GUI friendly thread
	pthread_create(&thread_id, NULL, doGuiStuff, params);
	//CreateThread(NULL, 0, doGuiStuff, (LPVOID)params, 0, NULL);
	pthread_join(thread_id, NULL);
	// wait for initialization to finish
	//WaitForSingleObject(guisyncevent, INFINITE);
	
	// close synchronozation event
	//CloseHandle(guisyncevent);

	// return whatever error code was generated during thread execution
	return errormsg_code;
}

void attenuate(float gain) {
	if (max_att_id > 0 && source->SetAttenuator != NULL) {
		int att_id = (int)(gain * max_att_id);
		if (att_id >= max_att_id) att_id = max_att_id - 1;
		if (att_id < 0) att_id = 0;
		source->SetAttenuator(att_id);
	}
}

int tsdrplugin_readasync(tsdrplugin_readasync_function cb, void *ctx) {
	if (source == NULL)
		RETURN_EXCEPTION("Please provide a full path to a valid ExtIO dinamic library.", TSDR_PLUGIN_PARAMETERS_WRONG);

	// start callbacks
	tsdr_cb = cb;
	tsdr_ctx = ctx;
	is_running = 1;

	act_freq = req_freq;

	int bufsize;
	if ((bufsize = source->StartHW(act_freq)) < 0) {
		is_running = 0;
		RETURN_EXCEPTION("The device has stopped responding.", TSDR_CANNOT_OPEN_DEVICE);
	}

	samplespercallback = bufsize * 2;
	if (samplespercallback > outbuf_size) {
		outbuf_size = samplespercallback;
		outbuf = (float *)realloc((void *)outbuf, sizeof(float)* outbuf_size);
	}

	act_gain = req_gain;
	attenuate(act_gain);
	//TODO yep this is dumb
	while (is_running) {
		Sleep(50);

		if (req_freq != act_freq) {
			act_freq = req_freq;
			source->SetHWLO(act_freq);
		}

		if (req_gain != act_gain) {
			act_gain = req_gain;
			attenuate(act_gain);
		}
	}

	source->StopHW();

	RETURN_OK();
}

void tsdrplugin_cleanup(void) {
	if (outbuf != NULL) {
		free(outbuf);
		outbuf = NULL;
	}

	if (source == NULL) return;

	closeextio();
}
