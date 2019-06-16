#ifdef HAVE_ANTTWEAKBAR

#ifndef ANTTWEAKBAR_H_INC
#define ANTTWEAKBAR_H_INC

#include <AntTweakBar.h>
#include <ardev/ardev_types.h>
#include <ardev/ardev.h>

#include <list>

class TweakBar
{
	private:
		
		// toggling menus
		bool b_displayList;
		bool b_environments;
		bool b_data;

		// bars
		TwBar *mainbar;
		TwBar *displaybar;
		TwBar *envbar;
		TwBar *databar;
		TwBar *selectbar;
		
		// toggles the display list window
		static void TW_CALL toggleDisplayList(void* tweakBar);
		// toggles the environments window
		static void TW_CALL toggleEnvironments(void* tweakBar);
		// toggles the data window
		static void TW_CALL toggleData(void* tweakBar);
		// callback for next env button
		static void TW_CALL nextEnvironment(void*);
		// callback for prev env button
		static void TW_CALL previousEnvironment(void*);
		// callback for prev env button
		static void TW_CALL absoluteEnvironment(void* data);
		// callback for clear paths button
		static void TW_CALL clear(void* data);

		// callback for exit button
		static void TW_CALL Exit(void*);

		static void TW_CALL PlayerDisconnect(void*);
		static void TW_CALL SetAutoConnectTW(const void *value,void *clientData);
		static void TW_CALL GetAutoConnectTW(void *value,void *clientData);

		// callback for setting render enabled
		static void TW_CALL SetEnabledTW(const void *value,void *clientData);
		// callback for getting path history length
		static void TW_CALL GetHistoryTW(void *value,void *clientData);
		// callback for setting path history length
		static void TW_CALL SetHistoryTW(const void *value,void *clientData);
		// callback for getting render enabled settings
		static void TW_CALL GetEnabledTW(void *value,void *clientData);
		// callback for setting colour
		static void TW_CALL SetColourTW(const void *value,void *clientData);
		// callback for getting colour
		static void TW_CALL GetColourTW(void *value,void *clientData);
		
		// callback for setting a position origin x
		static void TW_CALL SetXTW(const void *value,void *clientData);
		// callback for getting a position origin x
		static void TW_CALL GetXTW(void *value,void *clientData);
		// callback for setting a position origin y
		static void TW_CALL SetYTW(const void *value,void *clientData);
		// callback for getting a position origin y
		static void TW_CALL GetYTW(void *value,void *clientData);
		// callback for setting a position origin z
		static void TW_CALL SetZTW(const void *value,void *clientData);
		// callback for getting a position origin z
		static void TW_CALL GetZTW(void *value,void *clientData);
		
	public:
		
		// sets up the intial tweak bar settings and default windows
		TweakBar();
		// removes all tweak bars and shuts down
		~TweakBar();
		// creates the display list window and controls
		void generateDisplayMenu(RenderList &List);

		// setup call to register environments in the gui
		void addEnvironment(int i, const char* envName=NULL);
		// removes the default environment, which is added as a place holder
		void removeDefaultEnv();

		/// Change select window to display currently selected
		void SelectWindow(RenderObject* selected);

		/// Get a bar from outside of this class
		TwBar* GetDisplayBar(){return displaybar;};
		TwBar* GetMainBar(){return mainbar;};
		TwBar* GetEnvironmentBar(){return envbar;};
		TwBar* GetDataBar(){return databar;};
		TwBar* GetSelectBar(){return selectbar;};
};

#endif //ANTTWEAKBAR_H_INC
#endif //HAVE_ANTTWEAKBAR
