#include "one.hpp"
Sq *sq = NULL;

#define SQ_API __declspec(dllexport)

EXTERN_C SQ_API void init(const char* dir, const char *log_filename, int inter, int proxy_port, int log_enable)
{
	sq = new Sq(dir, inter, proxy_port);
	if (log_enable) {
		sq->enable_log(log_filename);
	}
}

EXTERN_C SQ_API void start()
{
	sq->start();
}

EXTERN_C SQ_API void stop()
{
	sq->stop();
}

EXTERN_C SQ_API void add_rule(const char *path, const char *pre_rule, const char *post_rule)
{
	sq->proxy()->addPreRule(path, std::string(pre_rule));
	sq->proxy()->addPostRule(path, std::string(post_rule));
}

BOOL WINAPI DllMain(HINSTANCE module_handle, DWORD reason_for_call, LPVOID reserved)
{
	if (reason_for_call == DLL_PROCESS_ATTACH) // Self-explanatory
	{
		DisableThreadLibraryCalls(module_handle); // Disable DllMain calls for DLL_THREAD_*
		//if (reserved == NULL) // Dynamic load
		//{
		//	//foobar_meow(); // Initialize your stuff or whatever
		//	// Return FALSE if you don't want your module to be dynamically loaded
		//}
		//else // Static load
		//{
		//	// Return FALSE if you don't want your module to be statically loaded
		//	return FALSE;
		//}
	}

	if (reason_for_call == DLL_PROCESS_DETACH) // Self-explanatory
	{
		if (reserved == NULL) // Either loading the DLL has failed or FreeLibrary was called
		{
			// Cleanup
		}
		else // Process is terminating
		{
			// Cleanup
		}
	}
	return TRUE;
}
