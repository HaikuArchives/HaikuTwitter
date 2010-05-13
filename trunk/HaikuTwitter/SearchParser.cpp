#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "SearchParser.h"

using namespace xercesc;
using namespace std;

// Error codes

enum {
   ERROR_ARGS = 1, 
   ERROR_XERCES_INIT,
   ERROR_PARSE,
   ERROR_EMPTY_DOCUMENT
};

/**
 *  Constructor initializes xerces-C libraries.
 *  The XML tags and attributes which we seek are defined.
 *  The xerces-C DOM parser infrastructure is initialized.
 */

SearchParser::SearchParser()
{
   try
   {
      XMLPlatformUtils::Initialize();  // Initialize Xerces infrastructure
   }
   catch( XMLException& e )
   {
      char* message = XMLString::transcode( e.getMessage() );
      cerr << "XML toolkit initialization error: " << message << endl;
      XMLString::release( &message );
      // throw exception here to return ERROR_XERCES_INIT
   }

   // Tags and attributes used in XML file.
   // Can't call transcode till after Xerces Initialize()
   TAG_root        = XMLString::transcode("feed");
   TAG_status = XMLString::transcode("entry");
   TAG_text = XMLString::transcode("title");
   TAG_user = XMLString::transcode("name");
   TAG_username = XMLString::transcode("uri"); //Using the <uri> instead of <author> get us around some encoding bugs:)
   TAG_image = XMLString::transcode("link");
   TAG_date = XMLString::transcode("published");
   TAG_id = XMLString::transcode("id");
   TAG_source = XMLString::transcode("source");
   TAG_error = XMLString::transcode("error");
   ATTR_href = XMLString::transcode("href");

   m_ConfigFileParser = new XercesDOMParser;
   tweetPtr = NULL;
   numberOfEntries = 0;
}

/**
 *  Class destructor frees memory used to hold the XML tag and 
 *  attribute definitions. It als terminates use of the xerces-C
 *  framework.
 */

SearchParser::~SearchParser()
{
   delete m_ConfigFileParser;
   XMLString::release( &TAG_root );
   XMLString::release( &TAG_status );
   XMLString::release( &TAG_text );
   XMLString::release( &TAG_username );
   XMLString::release( &TAG_user );
   XMLString::release( &TAG_image);
   XMLString::release( &TAG_date );
   XMLString::release( &TAG_id );
   XMLString::release( &TAG_source );
   XMLString::release( &TAG_error );
   try
   {
      XMLPlatformUtils::Terminate();  // Terminate Xerces
   }
   catch( xercesc::XMLException& e )
   {
      char* message = xercesc::XMLString::transcode( e.getMessage() );

      cerr << "XML ttolkit teardown error: " << message << endl;
      XMLString::release( &message );
   }
   
   for(int i = 0; i < numberOfEntries; i++) {
   		delete tweetPtr[i];
   }
   delete tweetPtr;
}

int SearchParser::count() {
	return numberOfEntries;
}

HTTweet** SearchParser::getTweets() {
	
	return tweetPtr;
}

/**
 *  This function:
 *  - Tests the access and availability of the XML configuration file.
 *  - Configures the xerces-c DOM parser.
 *  - Reads and extracts the pertinent information from the XML config file.
 *
 *  @param in configFile The text string name of the HLA configuration file.
 */

void SearchParser::readData(const char *xmlData)
        throw( std::runtime_error )
{
   // Configure DOM parser.

   m_ConfigFileParser->setValidationScheme( XercesDOMParser::Val_Never );
   m_ConfigFileParser->setDoNamespaces( false );
   m_ConfigFileParser->setDoSchema( false );
   m_ConfigFileParser->setLoadExternalDTD( false );

   try
   {
   		// Creating a memory buffer inputsource for the parser
   		const char *chId = std::string("TimeLineData").c_str();
   		MemBufInputSource memSource((XMLByte *)xmlData, (XMLSize_t)strlen(xmlData)*sizeof(char), chId);
   		
   		std::cout << "Passing buffer to parser" << std::endl;
      m_ConfigFileParser->parse( memSource );

      // no need to free this pointer - owned by the parent parser object
      DOMDocument* xmlDoc = m_ConfigFileParser->getDocument();

      // Get the top-level element: NAme is "root". No attributes for "root"
      
      DOMElement* elementRoot = xmlDoc->getDocumentElement();
      if( !elementRoot ) {
      		string theName("HaikuTwitter");
      		string theText("Could not retrieve data. Please check your internet connection.");
      		string theUrl("error");
      		string theDate("");
      		tweetPtr = new HTTweet*[1];
      		tweetPtr[0] = new HTTweet(theName, theText, theUrl, theDate);
      		numberOfEntries++;
      		return;
      }
      
      // Parse XML file for tags of interest: "error"
		DOMNodeList* statusNodes = elementRoot->getElementsByTagName(TAG_error);
		
		for(XMLSize_t i = 0; i < statusNodes->getLength(); i++) {
			DOMNode* currentNode = statusNodes->item(i);
         	if( currentNode->getNodeType() &&  // true is not NULL
            	currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
         	{
            	// Found node which is an Element. Re-cast node as element
            	DOMElement* currentElement
                	        = dynamic_cast< xercesc::DOMElement* >( currentNode );
            	if( XMLString::equals(currentElement->getTagName(), TAG_error))
            	{
            		DOMText* textNode
            				= dynamic_cast< xercesc::DOMText* >( currentElement->getChildNodes()->item(0) );
            		
            		/*Transcode to UTF-8*/
            		XMLTransService::Codes resValue;
            		XMLByte utf8String[150];
            		XMLSize_t blabla = 0;
            		XMLTranscoder *t = XMLPlatformUtils::fgTransService->makeNewTranscoderFor("UTF-8", resValue, 150);
            		t->transcodeTo(textNode->getWholeText(), (XMLSize_t)150, utf8String, (XMLSize_t)150, blabla, XMLTranscoder::UnRep_Throw);
            		delete t;
            		string textString((char *)utf8String);
					string theName("HaikuTwitter");
      				string theUrl("error");
      				string theDate("");
      				tweetPtr = new HTTweet*[1];
      				tweetPtr[0] = new HTTweet(theName, textString, theUrl, theDate);
      				numberOfEntries++;
      				return;
            	}
         	}
		}
		
      // Parse XML file for tags of interest: "text"
		statusNodes = elementRoot->getElementsByTagName(TAG_text);
		XMLSize_t nodeCount = statusNodes->getLength();
		tweetPtr = new HTTweet*[nodeCount];
		
		for(XMLSize_t i = 1; i < nodeCount; i++) { //We skip first item (item 0), not interested in atom main title
			DOMNode* currentNode = statusNodes->item(i);
         	if( currentNode->getNodeType() &&  // true is not NULL
            	currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
         	{
            	// Found node which is an Element. Re-cast node as element
            	DOMElement* currentElement
                	        = dynamic_cast< xercesc::DOMElement* >( currentNode );
            	if( XMLString::equals(currentElement->getTagName(), TAG_text))
            	{
            		DOMText* textNode
            				= dynamic_cast< xercesc::DOMText* >( currentElement->getChildNodes()->item(0) );
            		
            		/*Transcode to UTF-8*/
            		XMLTransService::Codes resValue;
            		XMLByte utf8String[150];
            		XMLSize_t blabla = 0;
            		XMLTranscoder *t = XMLPlatformUtils::fgTransService->makeNewTranscoderFor("UTF-8", resValue, 150);
            		t->transcodeTo(textNode->getWholeText(), (XMLSize_t)150, utf8String, (XMLSize_t)150, blabla, XMLTranscoder::UnRep_Throw);
            		delete t;
            		            		
            		string textString((char *)utf8String);
            		textString.erase(textString.length()-5, 5); //Last three chars is garbage!
            		tweetPtr[numberOfEntries] = new HTTweet();
            		tweetPtr[numberOfEntries]->setText(textString);
            		numberOfEntries++;
            	}
         	}
		}

		nodeCount--; //Because we skipped item(0)
		
		// Parse XML file for tags of interest: "id"
		/*statusNodes = elementRoot->getElementsByTagName(TAG_id);
		
		for(XMLSize_t i = 0; i < nodeCount*2; i+=2) {
			DOMNode* currentNode = statusNodes->item(i);
         	if( currentNode->getNodeType() &&  // true is not NULL
            	currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
         	{
            	// Found node which is an Element. Re-cast node as element
            	DOMElement* currentElement
                	        = dynamic_cast< xercesc::DOMElement* >( currentNode );
            	if( XMLString::equals(currentElement->getTagName(), TAG_id))
            	{
            		DOMText* textNode
            				= dynamic_cast< xercesc::DOMText* >( currentElement->getChildNodes()->item(0) );
            		
            		string rawString(XMLString::transcode(textNode->getWholeText()));
            		tweetPtr[i/2]->setId(rawString.c_str());
            	}
         	}
		}*/

		// Parse XML file for tags of interest: "screen_name"
		statusNodes = elementRoot->getElementsByTagName(TAG_username);
		
		for(XMLSize_t i = 0; i < nodeCount; i++) {
			DOMNode* currentNode = statusNodes->item(i);
         	if( currentNode->getNodeType() &&  // true is not NULL
            	currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
         	{
            	// Found node which is an Element. Re-cast node as element
            	DOMElement* currentElement
                	        = dynamic_cast< xercesc::DOMElement* >( currentNode );
            	if( XMLString::equals(currentElement->getTagName(), TAG_username))
            	{
            		DOMText* textNode
            				= dynamic_cast< xercesc::DOMText* >( currentElement->getChildNodes()->item(0) );
            		
            		char *rawString = XMLString::transcode(textNode->getWholeText());
            		
            		/*Remove last 11 or 9 characters, junk.*/
            		if(i < nodeCount-1)
            			rawString[strlen(rawString)-11] = '\0';
            		else
            			rawString[strlen(rawString)-9] = '\0';
            		
            		string textString(rawString+19); //Skip "http://twitter.com/"... and we've got the username:)
            		
            		tweetPtr[i]->setScreenName(textString);
            		delete rawString;
            	}
         	}
		}

		// Parse XML file for tags of interest: "link"
		statusNodes = elementRoot->getElementsByTagName(TAG_image);
		//Result will give us 5 links in header that we don't care about.
		int skipCount = 5;
		
		for(XMLSize_t i = skipCount; i < (nodeCount*2)+skipCount; i+=2) {
			DOMNode* currentNode = statusNodes->item(i);
         	if( currentNode->getNodeType() &&  // true is not NULL
            	currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
         	{
            	// Found node which is an Element. Re-cast node as element
            	DOMElement* currentElement
                	        = dynamic_cast< xercesc::DOMElement* >( currentNode );
            	if( XMLString::equals(currentElement->getTagName(), TAG_image))
            	{
            		const XMLCh* href = currentElement->getAttribute(ATTR_href);
            		char *rawString = XMLString::transcode(href);
            		
            		string textString(rawString);
            		if(textString.find("statuses", 0) != std::string::npos) { //There was an extra link the header
            			i--; //Step back
            			skipCount++; //Skip the extra header link.
            		}
            		else
            			tweetPtr[(i-skipCount)/2]->setProfileImageUrl(textString);
            		delete rawString;
            	}
         	}
		}
		
		// Parse XML file for tags of interest: "published"
		statusNodes = elementRoot->getElementsByTagName(TAG_date);
		
		for(XMLSize_t i = 0; i < nodeCount; i++) {
			DOMNode* currentNode = statusNodes->item(i);
         	if( currentNode->getNodeType() &&  // true is not NULL
            	currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
         	{
            	// Found node which is an Element. Re-cast node as element
            	DOMElement* currentElement
                	        = dynamic_cast< xercesc::DOMElement* >( currentNode );
            	if( XMLString::equals(currentElement->getTagName(), TAG_date))
            	{
            		DOMText* textNode
            				= dynamic_cast< xercesc::DOMText* >( currentElement->getChildNodes()->item(0) );
            		
            		char *rawString = XMLString::transcode(textNode->getWholeText());
            		//Remove last character, holds ugly symbol.
            		rawString[strlen(rawString)-3] = '\0';
            		
            		string textString(rawString);
            		tweetPtr[i]->setPublishedDate(textString);
            		delete rawString;
            	}
         	}
		}
   }
   catch( xercesc::XMLException& e )
   {
      char* message = xercesc::XMLString::transcode( e.getMessage() );
      ostringstream errBuf;
      errBuf << "Error parsing file: " << message << flush;
      XMLString::release( &message );
   }
   std::cout << "Done parsing XML" << std::endl;
}
