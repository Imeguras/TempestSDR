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
#include "ExtIOPluginLoader.h"
#include "TSDRCodes.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
// A platform independent dynamic library loader

/*void *extio_getfunction(extiosource_t * plugin, char *functname)
{
    return (int (*)(int)) dlsym(plugin->handle, functname);
	//return (void*)GetProcAddress(plugin->fd,functname);
}*/

void extio_close(extiosource_t * plugin)
{
	/*if (plugin->fd == NULL) return;
    FreeLibrary(plugin->fd);*/
	if(plugin->handle == NULL) return;
	//TODO check if sucessfull etc..
	dlclose(plugin->handle);
}

int extio_load(extiosource_t * plugin, const char *dlname)
{
	//this is basically the fd 
	plugin->handle=dlopen(dlname,RTLD_NOW|RTLD_GLOBAL); 
    if (plugin->handle == NULL){
		fprintf(stderr, "%s\n", dlerror());
		return TSDR_INCOMPATIBLE_PLUGIN;
	}
	//TODO CHECK IF EVERYTHING IS PRESENT, and if mandatory stuff is present
	//mandatory
	plugin->InitHW= (bool (*)(char *, char *, int *))dlsym(plugin->handle, "InitHW"); 
	plugin->OpenHW= (bool (*)(void))dlsym(plugin->handle, "OpenHW"); 
	plugin->CloseHW= (void (*) (void)) dlsym(plugin->handle, "CloseHW");
	plugin->StartHW= (int (*) (long))dlsym(plugin->handle, "StartHW");
	plugin->StopHW= (void (*) (void)) dlsym(plugin->handle, "StopHW");
	plugin->SetCallback= (void (*) (pfnExtIOCallback))dlsym(plugin->handle, "SetCallback");
	plugin->SetHWLO= (int (*) (long))dlsym(plugin->handle, "SetHWLO");
	plugin->GetStatus= (int (*) (void))dlsym(plugin->handle, "GetStatus");
	// mandatory functions that rtlsdr expects
	plugin->GetHWSR= (long (*) (void))dlsym(plugin->handle, "GetHWSR");
	// completely optional functions
	plugin->SetAttenuator= (int (*) (int))dlsym(plugin->handle, "SetAttenuator");
	plugin->GetAttenuators=(int(*) (int, float *))dlsym(plugin->handle, "GetAttenuators");
	plugin->ShowGUI= (int (*) (void))dlsym(plugin->handle, "ShowGUI");
	plugin->HideGUI= (int (*) (void))dlsym(plugin->handle, "HideGUI");
	// mandatory
	/*
	if ((plugin->InitHW = (bool(__stdcall *) (char *, char *, int *)) extio_getfunction(plugin, "InitHW")) == 0) return TSDR_ERR_PLUGIN;
	if ((plugin->OpenHW = (bool(__stdcall *) (void)) extio_getfunction(plugin, "OpenHW")) == 0) return TSDR_ERR_PLUGIN;
	if ((plugin->CloseHW = (void(__stdcall *) (void)) extio_getfunction(plugin, "CloseHW")) == 0) return TSDR_ERR_PLUGIN;
	if ((plugin->StartHW = (int(__stdcall *) (long)) extio_getfunction(plugin, "StartHW")) == 0) return TSDR_ERR_PLUGIN;
	if ((plugin->StopHW = (void(__stdcall *) (void)) extio_getfunction(plugin, "StopHW")) == 0) return TSDR_ERR_PLUGIN;
	if ((plugin->SetCallback = (void(__stdcall *) (pfnExtIOCallback)) extio_getfunction(plugin, "SetCallback")) == 0) return TSDR_ERR_PLUGIN;
	if ((plugin->SetHWLO = (int(__stdcall *) (long)) extio_getfunction(plugin, "SetHWLO")) == 0) return TSDR_ERR_PLUGIN;
	if ((plugin->GetStatus = (int(__stdcall *) (void)) extio_getfunction(plugin, "GetStatus")) == 0) return TSDR_ERR_PLUGIN;

    // mandatory functions that rtlsdr expects
	if ((plugin->GetHWSR = (long(__stdcall *) (void)) extio_getfunction(plugin, "GetHWSR")) == 0) return TSDR_ERR_PLUGIN;

    // completely optional functions
	if ((plugin->SetAttenuator = (int(__stdcall *) (int)) extio_getfunction(plugin, "SetAttenuator")) == 0) plugin->SetAttenuator = NULL;
	if ((plugin->GetAttenuators = (int(__stdcall *) (int, float *)) extio_getfunction(plugin, "GetAttenuators")) == 0) plugin->GetAttenuators = NULL;
	if ((plugin->ShowGUI = (int(__stdcall *) (void)) extio_getfunction(plugin, "ShowGUI")) == 0) plugin->ShowGUI = NULL;
	if ((plugin->HideGUI = (int(__stdcall *) (void)) extio_getfunction(plugin, "HideGUI")) == 0) plugin->HideGUI = NULL;
	*/
    return TSDR_OK;
}
