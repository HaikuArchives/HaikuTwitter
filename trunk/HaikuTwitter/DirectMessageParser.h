#ifndef DIRECTMESSAGE_PARSER_HPP
#define DIRECTMESSAGE_PARSER_HPP
/**
 *  @file
 *  Class "DirectMessageParser" provides the functions to read the XML data.
 *  @version 1.0
 */
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLUTF8Transcoder.hpp>

#include <xercesc/framework/MemBufInputSource.hpp>

#include <string>
#include <stdexcept>
#include <locale.h>

#include "HTTweet.h"

using namespace std;

class DirectMessageParser
{
public:
	DirectMessageParser();
  	virtual~DirectMessageParser();
	void readData(const char *) throw(std::runtime_error);
	
	HTTweet** getTweets();
	int count();

private:
	xercesc::XercesDOMParser *m_ConfigFileParser;
	
	HTTweet **tweetPtr;
	int numberOfEntries;
	
   // Internal class use only. Hold Xerces data in UTF-16 SMLCh type.

	XMLCh* TAG_root;

	XMLCh* TAG_status;
	XMLCh* TAG_text;
	XMLCh* TAG_username;
	XMLCh* TAG_source;
	XMLCh* TAG_user;
	XMLCh* TAG_image;
	XMLCh* TAG_date;
	XMLCh* TAG_id;
	XMLCh* TAG_error;
};
#endif
