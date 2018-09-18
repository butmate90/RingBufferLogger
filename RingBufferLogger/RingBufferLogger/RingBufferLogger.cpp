/* ********************************************************************** */
/* Includes                                                               */
/* ********************************************************************** */
#include "RingBufferLogger.h"
#include <iostream>
#include <string>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <time.h>
/* ********************************************************************** */
/*  defines/globals                                                       */
/* ********************************************************************** */
const unsigned short RingBufferLogger::m_TIMESTAMP_LENGTH = 25;
const unsigned short RingBufferLogger::m_FORMAT_LENGTH = 4;
const char* RingBufferLogger::m_XMLSTARTTAG = "< ? xml version = \"1.0\" encoding = \"UTF-8\" ? >";
//*************************************************************************
/// \brief     Constructor
/// \param
///            path:            path of the log-files
///            fileName:        base name of the log-file
///            maxCfCardSpace:  maximum occupied space(bytes)
///            maxSizeofEntry:  maximum size of one log entry (bytes)
///            numOfLogFiles:   maximum number of log-files
///            format:          1 - TXT, 2 - XML
/// \return    -
//*************************************************************************
RingBufferLogger::RingBufferLogger(const std::string path, const std::string fileName, unsigned int maxStorageSpace, unsigned int maxSizeofEntry, unsigned int numOfLogFiles, LOG_FORMAT format) :
	m_maxStorageSpace(maxStorageSpace),
	m_maxSizeofEntry(maxSizeofEntry),
	m_numOfLogFiles(numOfLogFiles),
	m_format(format),
	m_writtenBytes(0),
	m_index(0),
	m_actualError(FO_OK),
	m_logLevel(LOG_LEVEL_DEBUG),
	m_logFacilities(WEBSERVER_ALL)
{
	m_path = path;
	m_fileName = fileName;
	
	m_tmpEntry = new char[m_TIMESTAMP_LENGTH + m_maxSizeofEntry];
	memset(m_tmpEntry, 0, m_TIMESTAMP_LENGTH + m_maxSizeofEntry);

	m_REALPATH_LENGTH = m_path.size() + m_fileName.size() + m_FORMAT_LENGTH + 2;	//pathname + filename + format(4 bytes) + iteration(1 byte) + '/0'(1byte)
	m_realPath = new char[m_REALPATH_LENGTH];
	memset(m_realPath, 0, m_REALPATH_LENGTH);

	//Setting up the first file for writing
	prepFileForWriting();
}


//*************************************************************************
/// \brief     Destructor
/// \param
///
/// \return    -
//*************************************************************************
RingBufferLogger::~RingBufferLogger()
{
	delete[] m_tmpEntry; 
	delete[] m_realPath;
}


//*************************************************************************
/// \brief     Creates the real path and opens the actually pointed
///            file for writing.
/// \param
///
/// \return    void
//*************************************************************************
void RingBufferLogger::prepFileForWriting()
{
	printf("\n\r Opening new Security Log-FILE! \n\r filename: %s%d", m_fileName.c_str(), m_index);
	snprintf(m_realPath, m_REALPATH_LENGTH, "%s%s%d", m_path.c_str(), m_fileName.c_str(), m_index);
	if (LOG_FMT_TXT == m_format)
	{
		if ((strlen(m_realPath) + m_FORMAT_LENGTH) < m_REALPATH_LENGTH)    //TODO: handle error case
			strncat_s(m_realPath, m_REALPATH_LENGTH, ".TXT", m_FORMAT_LENGTH);
	}
	else if(LOG_FMT_XML == m_format)
	{
		if ((strlen(m_realPath) + m_FORMAT_LENGTH) < m_REALPATH_LENGTH)
			strncat_s(m_realPath, m_REALPATH_LENGTH, ".XML", m_FORMAT_LENGTH);
	}

	fp[m_index] = fopen(m_realPath, "wb");
	if (!fp[m_index])
	{
		printf("\n\r RingBufferLogger::prepFileForWriting() - Opening FILE FAILED!");
		m_actualError = FO_ERROR;
	}
	else
	{
		if (LOG_FMT_XML == m_format)
		{
			fwrite(m_XMLSTARTTAG, sizeof(char), strlen(m_XMLSTARTTAG), fp[m_index]);
		}
		fclose(fp[m_index]);
	}

}

//*************************************************************************
/// \brief     inserts and xml node with a given value in a string
/// \param	   char* str, unsigned short len, char* nodeName, char* val
///
/// \return    void
//*************************************************************************

void RingBufferLogger::insertXmlNodeInString(char* str, unsigned short len, char* nodeName, char* val) {
	if (strncmp(nodeName, "timeStamp", len) == 0) 
	{
		snprintf(str, len, "<%s>%s<%s> ", nodeName, val, nodeName);
	}
	else if(strncmp(nodeName, "incident", len) == 0)
	{
		snprintf(str + (strlen(str)), len, "<%s>%s,<%s> ", nodeName, val, nodeName);
	}
}

//*************************************************************************
/// \brief     Writes the given string into the actually opened file
///            file for writing.
/// \param
///
/// \return    void
//*************************************************************************
void RingBufferLogger::writeEntry(char* buffer)
{
	printf("\n\r %s", buffer);
	//Open the file for appending
	fp[m_index] = fopen(m_realPath, "a");
	if (!fp[m_index])
	{
		printf("\n\r RingBufferLogger::writeEntry() - Opening FILE for appending FAILED!");
		m_actualError = FO_ERROR;
	}
	else
	{
		//get the UTC system time
		char outstr[200];
		time_t t;
		struct tm *tmp;
		const char* fmt = "%a, %d %b %y %T";

		t = time(NULL);
		tmp = gmtime(&t);
		if (tmp == NULL) {
			perror("gmtime error");
			exit(EXIT_FAILURE);
		}

		if (strftime(outstr, sizeof(outstr), fmt, tmp) == 0) {
			fprintf(stderr, "strftime returned 0");
			exit(EXIT_FAILURE);
		}

		//Writing entry with timestamp
		if (LOG_FMT_TXT == m_format)
		{
			snprintf(m_tmpEntry, m_TIMESTAMP_LENGTH + m_maxSizeofEntry, "%s - %s", outstr, buffer);
			fwrite(m_tmpEntry, sizeof(char), strlen(m_tmpEntry), fp[m_index]);
			fwrite("\n", sizeof(char), 1, fp[m_index]);
		}
		else if(LOG_FMT_XML == m_format)
		{
			insertXmlNodeInString(m_tmpEntry, (m_TIMESTAMP_LENGTH + m_maxSizeofEntry), "timeStamp", outstr);
			insertXmlNodeInString(m_tmpEntry, (m_maxSizeofEntry), "incident", buffer);
			fwrite("\n", sizeof(char), 1, fp[m_index]);
			fwrite(m_tmpEntry, sizeof(char), strlen(m_tmpEntry), fp[m_index]);
		}

		m_writtenBytes += strlen(m_tmpEntry) + 1;
		fclose(fp[m_index]);
	}
}


//*************************************************************************
/// \brief     Main function to save a LOG-entry.
/// \param
///
/// \return    void
//*************************************************************************
void RingBufferLogger::LOG(LOG_LEVEL level, LOG_FACILITY facility, unsigned short mask, const char *format, ...)
{
	//Check level, facility and mask
	if ((m_logLevel >= level) && (m_logFacilities & facility) && (FO_OK == m_actualError))        //TODO: MASK?
	{
		printf("\n\r New log entry!");
		memset(m_tmpEntry, 0, sizeof(m_maxSizeofEntry));

		//Taking care of the format string
		char buffer[256];        //TODO: 256?
		memset(buffer, 0, 256);
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, 256, format, args);
		va_end(args);

		if (m_maxStorageSpace / m_numOfLogFiles > (m_writtenBytes + m_maxSizeofEntry))//TODO: we may lose one entry space
		{
			writeEntry(buffer);
		}
		else
		{
			//Open a new file
			m_index == (m_numOfLogFiles - 1) ? m_index = 0 : m_index++;
			m_writtenBytes = 0;
			prepFileForWriting();
			writeEntry(buffer);
		}
	}
}


//*************************************************************************
/// \brief     Set the configuration for the logger - LEVEL, FACILITY, MASK
///
/// \param
///
/// \return    void
//*************************************************************************
void RingBufferLogger::setConfiguration(LOG_LEVEL level, LOG_FACILITY facility, unsigned short mask)
{
	m_logLevel = level;
	m_logFacilities = facility;
}

