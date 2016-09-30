/*
 * main.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: Burak Mandira
 */

#include <iostream>
#include <string.h>		// memcpy()
#include <algorithm>	// atoi()
#include <curl/curl.h>

using namespace std;

string getClassValue( const char* className, string& buffer, size_t& afterPos)
{
	string str = "class=\"", result = "";
	str.append( className);
	str += "\"";
	size_t pos = buffer.find( str, afterPos);
	if( pos == string::npos)
		return "";
	pos += str.length();

	pos = buffer.find( '>', pos);
	while( buffer[pos+1] == '<')
		pos = buffer.find( '>', pos + 2);

	// get the row number
	size_t start = pos + 1;
	size_t end = buffer.find( '<', start + 1);
	result += buffer.substr( start, end - start);
	result += ".";
	result += string( 4 - (end - start), ' ');

	// get the category
	pos = buffer.find( "class", end);
	pos = buffer.find( '>', pos);
	while( buffer[pos+1] == '<')
		pos = buffer.find( '>', pos + 2);
	start = pos + 1;
	end = buffer.find( '<', start + 1);
	result += buffer.substr( start, end - start);
	result += string( 20 - (end - start), ' ');

//	// get type
//	pos = buffer.find( "<i>", end) + 3;
//	while( buffer[pos] != '<')
//		result += buffer[pos++];

	// get the word
	pos = buffer.find( "class", end);
	pos = buffer.find( '>', pos);
	while( buffer[pos+1] == '<')
		pos = buffer.find( '>', pos + 2);
	start = pos + 1;
	end = buffer.find( '<', start + 1);
	int totalChar = min( 60, (int)(end - start));
	result += buffer.substr( start, totalChar);
	result += string( 60 - totalChar, ' ');

	// get the meaning
	pos = buffer.find( "class", pos);
	pos = buffer.find( '>', pos);
	while( buffer[pos+1] == '<')
		pos = buffer.find( '>', pos + 2);
	start = pos + 1;
	end = buffer.find( '<', start + 1);
	result += " ";
	result += buffer.substr( start, end - start);

	afterPos = end;
	return result;
}

static size_t write_to_mem_callback( void *contents, size_t size, size_t nmemb, string *stream)
{
	size_t actualSize = size * nmemb;
	stream->append( (char*) contents, actualSize);
	return actualSize;
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to)
{
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}


int main( int argc, char **argv)
{
	if( argc < 2)
	{
		cerr << "Use: tureng [WORD]\n";
		return -1;
	}

	// 1. initialize
	curl_global_init( CURL_GLOBAL_ALL);
	CURL* curl = curl_easy_init();

	if( curl == NULL)
	{
		cerr << "CURL couldn't be initialised." << endl;
		curl_global_cleanup();
		return -1;
	}

	// 2. set options

	/*
	 * CURLcode curl_easy_setopt(CURL *handle, CURLoption option, parameter);
	 * the parameter can be a long, a function pointer, an object pointer or a curl_off_t
	 */

	string buffer;
	string url = "http://tureng.com/tr/turkce-ingilizce/";

	// replace spaces as "%20" which means space in HTML
	url.append( replaceAll( argv[1], " ", "%20"));

	const char *URL = url.c_str();

	curl_easy_setopt( curl, CURLOPT_URL, URL);
//	curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L);	// verbose output is ON
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, write_to_mem_callback);
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, &buffer);

	// 3. perform
	CURLcode result = curl_easy_perform( curl);
	if( result != CURLE_OK)
	{
		cerr << "CURL error occured: " << curl_easy_strerror( result) << endl;
		return -1;
	}

	// 4. get info
	long responseCode;
	curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &responseCode);
	if( (int)responseCode / 100 != 2)
	{
		cerr << "HTTP status code: " << responseCode << endl;
		return -1;
	}

	// replace UNICODE letters -ü, ö, ç, Ü, Ö, Ç, ', nonbreak space-
	size_t current = buffer.find( "&#");
	while(  current != string::npos)
	{
		string unicodeNumber = "";
		int p = current + 1;
		while( buffer[++p] != ';')
			unicodeNumber += buffer[p];
		switch( atoi( unicodeNumber.c_str()))
		{
		case 39:	// &#39;
			buffer.replace( current, 5, "'");
			break;
		case 160:	// &#160;
			buffer.replace( current, 6, " ");
			break;
		case 199:	// &#199;
			buffer.replace( current, 6, "Ç");
			break;
		case 214:	// &#214;
			buffer.replace( current, 6, "Ö");
			break;
		case 220:	// &#220;
			buffer.replace( current, 6, "Ü");
			break;
		case 231:	// &#231;
			buffer.replace( current, 6, "ç");
			break;
		case 246:	// &#246;
			buffer.replace( current, 6, "ö");
			break;
		case 252:	// &#252;
			buffer.replace( current, 6, "ü");
			break;
		default:
			cerr << "WARNING - Encountered with an unknown UNICODE character:" << unicodeNumber << endl;
			break;
		}
		current = buffer.find( "&#", current + 1);
	}

	// list the results
	size_t afterPos = 0;
	string val = getClassValue( "rc0 hidden-xs", buffer, afterPos);
	while( val != "")
	{
		cout << val << endl;
		val = getClassValue( "rc0 hidden-xs", buffer, afterPos);
	}

//	cout << buffer << endl;

	// 5. clean up
	curl_easy_cleanup( curl);
	curl_global_cleanup();

	return 0;
};



