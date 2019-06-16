
#include <stdio.h>

extern "C"
{
	/// this function is the entry point into the plugin
	void ar_plugin_init()
	{
		/// The standard Hello World plugin.
		printf("Hello World! plugin.\n");
	}
}
