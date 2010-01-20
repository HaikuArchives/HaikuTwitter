#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "TimeLineParser.h"

using namespace xercesc;
using namespace std;

/**
 *  Constructor initializes xerces-C libraries.
 *  The XML tags and attributes which we seek are defined.
 *  The xerces-C DOM parser infrastructure is initialized.
 */

TimeLineParser::TimeLineParser()
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
   TAG_root        = XMLString::transcode("statuses");
   TAG_status = XMLString::transcode("status");
   TAG_text = XMLString::transcode("text");
   TAG_user = XMLString::transcode("user");
   TAG_username = XMLString::transcode("screen_name");
   TAG_image = XMLString::transcode("profile_image_url");
   TAG_date = XMLString::transcode("created_at");

   m_ConfigFileParser = new XercesDOMParser;
   tweetPtr = NULL;
   numberOfEntries = 0;
}

/**
 *  Class destructor frees memory used to hold the XML tag and 
 *  attribute definitions. It als terminates use of the xerces-C
 *  framework.
 */

TimeLineParser::~TimeLineParser()
{
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

   try
   {
		//Freeing these makes the app crash WHY???
      //XMLString::release( &TAG_root );
      //XMLString::release( &TAG_status );
      //XMLString::release( &TAG_text );
      //XMLString::release( &TAG_username );
   }
   catch( ... )
   {
      cerr << "Unknown exception encountered in TagNamesdtor" << endl;
   }
}

int TimeLineParser::count() {
	return numberOfEntries;
}

HTTweet** TimeLineParser::getTweets() {
	
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

void TimeLineParser::readData(const char *xmlData)
        throw( std::runtime_error )
{
   // Test to see if the file is ok.

   /*struct stat fileStatus;

   int iretStat = stat(configFile.c_str(), &fileStatus);
   if( iretStat == ENOENT )
      throw ( std::runtime_error("Path file_name does not exist, or path is an empty string.") );
   else if( iretStat == ENOTDIR )
      throw ( std::runtime_error("A component of the path is not a directory."));
   else if( iretStat == ELOOP )
      throw ( std::runtime_error("Too many symbolic links encountered while traversing the path."));
   else if( iretStat == EACCES )
      throw ( std::runtime_error("Permission denied."));
   else if( iretStat == ENAMETOOLONG )
      throw ( std::runtime_error("File can not be read\n"));
    */

   // Configure DOM parser.

   m_ConfigFileParser->setValidationScheme( XercesDOMParser::Val_Never );
   m_ConfigFileParser->setDoNamespaces( false );
   m_ConfigFileParser->setDoSchema( false );
   m_ConfigFileParser->setLoadExternalDTD( false );

   try
   {
   		// Creating a memory buffer inputsource for the parser
   		std::cout << "Creating buffer" << std::endl;
   		const char *chId = std::string("TimeLineData").c_str();
   		MemBufInputSource memSource((XMLByte *)xmlData, (XMLSize_t)strlen(xmlData)*sizeof(char), chId);
   		
   		std::cout << "Passing buffer to parser" << std::endl;
      m_ConfigFileParser->parse( memSource );

      // no need to free this pointer - owned by the parent parser object
      DOMDocument* xmlDoc = m_ConfigFileParser->getDocument();

      // Get the top-level element: NAme is "root". No attributes for "root"
      
      DOMElement* elementRoot = xmlDoc->getDocumentElement();
      if( !elementRoot ) throw(std::runtime_error( "empty XML document" ));
				
      // Parse XML file for tags of interest: "text"
		DOMNodeList* statusNodes = elementRoot->getElementsByTagName(TAG_text);
		const XMLSize_t nodeCount = statusNodes->getLength();
		tweetPtr = new HTTweet*[nodeCount];
		
		for(XMLSize_t i = 0; i < nodeCount; i++) {
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
            		tweetPtr[numberOfEntries] = new HTTweet();
            		tweetPtr[numberOfEntries]->setText(textString);
            		numberOfEntries++;
            	}
         	}
		}
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
            		/*Remove last character, holds ugly symbol.*/
            		rawString[strlen(rawString)-5] = '\0';
            		
            		string textString(rawString);
            		tweetPtr[i]->setScreenName(textString);
            	}
         	}
		}
		// Parse XML file for tags of interest: "profile_image_url"
		statusNodes = elementRoot->getElementsByTagName(TAG_image);
		
		for(XMLSize_t i = 0; i < nodeCount; i++) {
			DOMNode* currentNode = statusNodes->item(i);
         	if( currentNode->getNodeType() &&  // true is not NULL
            	currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
         	{
            	// Found node which is an Element. Re-cast node as element
            	DOMElement* currentElement
                	        = dynamic_cast< xercesc::DOMElement* >( currentNode );
            	if( XMLString::equals(currentElement->getTagName(), TAG_image))
            	{
            		DOMText* textNode
            				= dynamic_cast< xercesc::DOMText* >( currentElement->getChildNodes()->item(0) );
            		
            		char *rawString = XMLString::transcode(textNode->getWholeText());
            		/*Remove last character, holds ugly symbol.*/
            		rawString[strlen(rawString)-5] = '\0';
            		
            		string textString(rawString);
            		tweetPtr[i]->setProfileImageUrl(textString);
            	}
         	}
		}
		// Parse XML file for tags of interest: "created_at"
		statusNodes = elementRoot->getElementsByTagName(TAG_date);
		
		for(XMLSize_t i = 0; i < nodeCount*2; i+=2) {
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
            		/*Remove last character, holds ugly symbol.*/
            		rawString[strlen(rawString)-5] = '\0';
            		
            		string textString(rawString);
            		tweetPtr[i/2]->setDate(textString);
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
