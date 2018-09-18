#ifndef RINGBUFFERLOGGER_H_
#define RINGBUFFERLOGGER_H_

/* ********************************************************************** */
/*  includes                                                              */
/* ********************************************************************** */
#include <iostream>

/* ********************************************************************** */
/*  defines/globals                                                       */
/* ********************************************************************** */
#define MAX_NUM_OF_LOGFILES 4

/* ************************************************************************* * /
/* Type Definitions                                                          */
/* ************************************************************************* */
typedef enum LOG_FACILITY : unsigned short
{
	WEBSERVER_AUTHENTICATION = 0x00000001,
	WEBSERVER_SERVER_APPLICATIONS = 0x00000002,
	WEBSERVER_HTTP_REQUESTS = 0x00000003,
	TEST = 0x00000004,
	WEBSERVER_ALL = 0x0000000F
} LOG_FACILITIES;

typedef enum LOG_LEVEL : unsigned short
{
	LOG_LEVEL_DEBUG = 4,
	LOG_LEVEL_INFO = 3,
	LOG_LEVEL_WARNING = 2,
	LOG_LEVEL_ALWAYS = 1
} LOG_LEVEL;

typedef enum FILE_OPEN_ERROR_CODES : unsigned short
{
	FO_OK = 0,
	FO_ERROR = 1
} FILE_OPEN_ERROR_CODES;

typedef enum LOG_FORMAT: unsigned short
{
	LOG_FMT_TXT = 1,
	LOG_FMT_XML = 2
} LOG_FORMAT;

/* ********************************************************************** */
/* classes                                                                */
/* ********************************************************************** */
class RingBufferLogger
{

private:
	void prepFileForWriting();
	void writeEntry(char* buffer);
	/** Disable copy constructor. */
	explicit RingBufferLogger(const RingBufferLogger&);
	/** Disable assignment operator. */
	RingBufferLogger& operator=(const RingBufferLogger&);
public:
	RingBufferLogger(const std::string path, std::string fileName, unsigned int maxStorageSpace, unsigned int maxSizeofEntry, unsigned int numOfLogFiles, LOG_FORMAT format);
	~RingBufferLogger();
	void setConfiguration(LOG_LEVEL level, LOG_FACILITY facility, unsigned short mask);
	void LOG(LOG_LEVEL level, LOG_FACILITY facility, unsigned short mask, const char *format, ...);
	void insertXmlNodeInString(char* str, unsigned short len, char* nodeName, char* val);

private:
	std::string m_path;
	std::string m_fileName;
	char* m_realPath;	
	char* m_tmpEntry;
	unsigned int  m_maxStorageSpace;
	unsigned int  m_maxSizeofEntry;
	unsigned int  m_numOfLogFiles;
	unsigned int  m_writtenBytes;
	unsigned short m_index;
	unsigned short  m_actualError;
	FILE*     fp[MAX_NUM_OF_LOGFILES];

	//Internal configuration - string sizes
	unsigned short m_REALPATH_LENGTH;	//calculated as: pathname + filename + format(4 bytes) + iteration(1 byte)
	const static unsigned short m_TIMESTAMP_LENGTH;	//const static
	const static unsigned short m_FORMAT_LENGTH;	//const static
	const static char* m_XMLSTARTTAG;	//const static

	//External configuration
	LOG_LEVEL m_logLevel;
	LOG_FACILITIES m_logFacilities;
	LOG_FORMAT m_format;
};


#endif /* RINGBUFFERLOGGER_H_ */