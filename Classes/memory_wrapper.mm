/*
 Copyright (C) 2009 by Kyle Poole <kyle.poole@gmail.com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
 */


#include <UIKit/UIKit.h>
#include <TargetConditionals.h>	// defines TARGET_IPHONE_SIMULATOR
#import <mach/mach.h>
#import <mach/mach_host.h>
#include <pthread.h>
pthread_t main_thread;


size_t iPhoneMemory(void);


// KP: using a project setting define, eg DISABLE_POOL_ALLOC, causes it to recompile the whole project, which takes very long...
// this way, we can just set the #if statement below to 1 or 0 to enable/disable the allocators

#if !(TARGET_IPHONE_SIMULATOR)


// Use Google's tcmalloc - fast and minimum overhead, and also useful for memory profiling

// When linked in statically, the base tcmalloc has a lot of problems fighting with the system allocator. Some memory
// created by tcmalloc is freed by the system allocator, or vice versa, causing errors.
// So we provide the source code for the normal system malloc, so if tcmalloc does not recognize the
// pointer being deleted, it can pass it off to the system free()

#include "memory_wrapper.h"

#include "google/tcmalloc.h"
#include "google/heap-profiler.h"

#include <exception> // for std::bad_alloc
#include <new>
#include <cstdlib> // for malloc() and free()

#include <stdio.h>
#include <errno.h>




// uncomment to profile memory usage
//#define PROFILE_HEAPS

// uncomment to use tcmalloc globally in all threads
#define OVERRIDE_ALL



extern "C" {
	void* dlmalloc(size_t size);
	void* dlcalloc(size_t count, size_t size);
	void* dlvalloc(size_t size);
	void* dlmemalign(size_t alignment, size_t size);
	void* dlrealloc(void* ptr, size_t size);
	void dlfree(void* ptr);
};

pthread_mutex_t dlmalloc_mutex = PTHREAD_MUTEX_INITIALIZER;

void memory_profiler_start(const char *path)
{
#if defined(TARGET_IPHONE_SIMULATOR) && defined(PROFILE_HEAPS)
	HeapProfilerStart(path);
#endif
}

void memory_stats(const char *reason)
{
	printf("\n\n%s\n", reason);
	tc_malloc_stats();
#if defined(TARGET_IPHONE_SIMULATOR) && defined(PROFILE_HEAPS)
	HeapProfilerDump(reason);
#endif
	iPhoneMemory();
}



extern "C" {
	
void* malloc(size_t size) __THROW                 
{
#ifdef OVERRIDE_ALL
	return tc_malloc(size);
#else
	if(pthread_equal(pthread_self(), main_thread))
	   return tc_malloc(size);
	   
   return dlmalloc(size);
#endif
}

void* calloc(size_t count, size_t size) __THROW       
{
#ifdef OVERRIDE_ALL
	return tc_calloc(count, size);
#else
	if(pthread_equal(pthread_self(), main_thread))
	   return tc_calloc(count, size);
	return dlcalloc(count, size);
#endif
}
	
#ifdef OVERRIDE_ALL
void* valloc(size_t size) __THROW                 
{ 
	return tc_valloc(size);
}	
 
void* pvalloc(size_t s) __THROW                
{ 
	return tc_pvalloc(s);      
}

void* memalign(size_t alignment, size_t size) __THROW     
{
	return tc_memalign(alignment, size);
}
	
int posix_memalign(void** memptr, size_t alignment, size_t size) __THROW 
{
	return tc_posix_memalign(memptr, alignment, size);
}
#endif

void* realloc(void* p, size_t s) __THROW       
{ 
	return tc_realloc(p, s);   
}

void free(void* p) __THROW
{
#ifdef OVERRIDE_ALL
	tc_free(p);                
#else
	if(pthread_equal(pthread_self(), main_thread))
		tc_free(p);
	else
		dlfree(p);
#endif
	
}

void  cfree(void* p) __THROW                   
{ 
	tc_cfree(p);               
}


#ifdef OVERRIDE_ALL
void* __libc_malloc(size_t size)               { return malloc(size);       }
void  __libc_free(void* ptr)                   { free(ptr);                 }
void* __libc_realloc(void* ptr, size_t size)   { return realloc(ptr, size); }
void* __libc_calloc(size_t n, size_t size)     { return calloc(n, size);    }
void  __libc_cfree(void* ptr)                  { cfree(ptr);                }
void* __libc_memalign(size_t align, size_t s)  { return memalign(align, s); }
void* __libc_valloc(size_t size)               { return valloc(size);       }
void* __libc_pvalloc(size_t size)              { return pvalloc(size);      }
#endif
} // extern "C"
	

// also override c++ global operators, to work in STL as well
void* operator new (size_t size)
{
	return malloc(size); 
}

void* operator new[](size_t size)
{
	return malloc(size);
}

void* operator new (size_t size, const std::nothrow_t& t)
{
	return malloc(size);
}

void* operator new[] (size_t size, const std::nothrow_t& t)
{
	return malloc(size);
}


void operator delete (void *p)
{
	free(p);
}

void operator delete (void *p, const std::nothrow_t&)
{
	free(p);
}


void operator delete[] (void *p)
{
	free(p);
}	

void operator delete[] (void *p, const std::nothrow_t&)
{
	free(p);
}	



#else



void memory_profiler_start(const char *path)
{

}

#include <stdio.h>
void memory_stats(const char *reason)
{
	// get general overall iPhone memory stats
	printf("%s\n", reason);

	iPhoneMemory();
}

#endif

void init_custom_malloc()
{
	main_thread = pthread_self();
	
	
	size_t freeMem = iPhoneMemory();
	
	// display memory info in a popup dialog
	float freeMemF = (float)freeMem / (1024*1024);

	NSString *str;
	char cStr[256];
	if (freeMemF < 24.0f)
	{
		sprintf(cStr, "Low memory detected, performance problems or crashes may occur. Please reset your device to free up memory!", freeMemF);
		str = [NSString stringWithUTF8String: cStr];
		
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Memory Warning!" message:str delegate:nil cancelButtonTitle:@"Ok" otherButtonTitles:nil];
		[alert show];
		[alert release];		
	}
/*	else
	{
		sprintf(cStr, "Current free memory is %4.2f mb", freeMemF);
		str = [NSString stringWithUTF8String: cStr];
	
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Beta debug info" message:str delegate:nil cancelButtonTitle:@"Ok" otherButtonTitles:nil];
		[alert show];
		[alert release];
	}
*/	
}

size_t iPhoneMemory(void)
{
	mach_port_t host_port;
	mach_msg_type_number_t host_size;
	vm_size_t pagesize;

	host_port = mach_host_self();
	host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
	host_page_size(host_port, &pagesize);        

	vm_statistics_data_t vm_stat;

	if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS)
		printf("Failed to fetch vm statistics\n");

	/* Stats in bytes */ 
	natural_t mem_used = (vm_stat.active_count +
						  vm_stat.inactive_count +
						  vm_stat.wire_count) * pagesize;
	natural_t mem_free = vm_stat.free_count * pagesize;
	natural_t mem_total = mem_used + mem_free;
	printf("iPhone system memory used: %u free: %u total: %u\n", mem_used, mem_free, mem_total);
	return mem_free;
}
