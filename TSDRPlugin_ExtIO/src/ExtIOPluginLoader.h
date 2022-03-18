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
#ifndef _TSDRPluginLoader
#define _TSDRPluginLoader

#include <stdint.h>
#include <stdbool.h>
#include <dlfcn.h>
//#include <windows.h>

typedef void(*pfnExtIOCallback) (int cnt, int status, float IQoffs, void *IQdata);

	struct extiosource {
		//HINSTANCE fd;
		void* handle;
		// mandatory functions that the dll must implement
		bool( * InitHW) (char *name, char *model, int *hwtype); // for hwtype see enum extHWtypeT
		bool( * OpenHW) (void);
		void( * CloseHW) (void);
		int( * StartHW) (long extLOfreq);
		void( * StopHW) (void);
		void( * SetCallback) (pfnExtIOCallback funcptr);
		int( * SetHWLO) (long extLOfreq);   // see also SetHWLO64
		int( * GetStatus) (void);

		// mandatory functions that tsdrrequires
		long( * GetHWSR) (void);

		// completely optional functions
		int( * SetAttenuator) (int atten_idx);
		int( * GetAttenuators) (int atten_idx, float * attenuation);
		int( * ShowGUI) (void);
		int( * HideGUI) (void);

	} typedef extiosource_t;

	int extio_load(extiosource_t * plugin, const char *dlname);
	void extio_close(extiosource_t * plugin);

#endif
