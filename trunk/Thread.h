// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		15aug04		initial version
		01		18oct04		ck: remove set debug name, add debug name to create

		wrap Win32 thread object

*/

#ifndef WTHREAD_INCLUDED
#define WTHREAD_INCLUDED

#ifndef WHANDLE_DEFINED
#define WHANDLE_DEFINED
typedef void *WHANDLE;
#endif

class WThread : public WObject {
public:
	// Define the thread function prototype.
	typedef	unsigned (__stdcall *THRFUNCP)(void *);
//
// Constructs a thread.
//
	WThread();
//
// Destroys the thread.
//
	~WThread();
//
// Creates the thread.
//
// Returns: True if successful.
//
	bool	Create(
		void *Security,				// Security descriptor for new thread.
		unsigned	StackSize,		// Stack size for new thread or 0.
		THRFUNCP	StartAddress,	// Start address of new thread.
		void		*ArgList,		// Argument list to be passed to new thread or NULL.
		unsigned	InitFlag,		// Initial state of new thread.
		unsigned	*ThreadID,		// Receives the thread identifier.
		LPCSTR	DebugName = NULL	// Optional pointer to the debug name.
	);
//
// Closes the thread handle.
//
	void	Close();
//
// Retrieves the thread's handle.
//
// Returns: The handle.
//
	operator WHANDLE() const;

protected:
	WHANDLE	m_hThread;		// The thread handle.
};

inline WThread::operator WHANDLE() const
{
	return(m_hThread);
}

#endif
