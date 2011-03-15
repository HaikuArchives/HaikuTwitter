#ifndef _TWITCURL_H_
#define _TWITCURL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstring>
#include <string>
#include <sstream>
#include <curl/curl.h>

/* Default values used in twitcurl */
namespace twitCurlDefaults
{
    /* Constants */
    const int TWITCURL_DEFAULT_BUFFSIZE = 1024;
    const std::string TWITCURL_COLON = ":";
    const char TWITCURL_EOS = '\0';

    /* Miscellaneous data used to build twitter URLs*/
    const std::string TWITCURL_STATUSSTRING = "status=";
    const std::string TWITCURL_REPLYTOID = "in_reply_to_status_id=";
    const std::string TWITCURL_TEXTSTRING = "text=";
    const std::string TWITCURL_QUERYSTRING = "query=";  
    const std::string TWITCURL_SEARCHQUERYSTRING = "?q=";      
    const std::string TWITCURL_SCREENNAME = "?screen_name=";
    const std::string TWITCURL_USERID = "?user_id=";
    const std::string TWITCURL_EXTENSIONFORMAT = ".xml";
    const std::string TWITCURL_TARGETSCREENNAME = "?target_screen_name=";
    const std::string TWITCURL_TARGETUSERID = "?target_id=";
};

/* Default twitter URLs */
namespace twitterDefaults
{

    /* Search URLs */
    const std::string TWITCURL_SEARCH_URL = "http://search.twitter.com/search.atom";
    
    /* Trend URLs */
	const std::string TWITCURL_TRENDS_URL = "http://search.twitter.com/trends.json";
	const std::string TWITCURL_TRENDS_CURRENT_URL = "http://search.twitter.com/trends/current.json";
	const std::string TWITCURL_TRENDS_WEEKLY_URL = "http://search.twitter.com/trends/weekly.json";
	const std::string TWITCURL_TRENDS_DAILY_URL = "http://search.twitter.com/trends/daily.json";

    /* Status URLs */
    const std::string TWITCURL_STATUSUPDATE_URL = "http://twitter.com/statuses/update.xml";
    const std::string TWITCURL_STATUSSHOW_URL = "http://twitter.com/statuses/show/";
    const std::string TWITCURL_STATUDESTROY_URL = "http://twitter.com/statuses/destroy/";

    /* Timeline URLs */
    const std::string TWITCURL_HOME_TIMELINE_URL = "http://twitter.com/statuses/home_timeline.xml";
    const std::string TWITCURL_PUBLIC_TIMELINE_URL = "http://twitter.com/statuses/public_timeline.xml";
    const std::string TWITCURL_FEATURED_USERS_URL = "http://twitter.com/statuses/featured.xml";
    const std::string TWITCURL_FRIENDS_TIMELINE_URL = "http://twitter.com/statuses/friends_timeline.xml";
    const std::string TWITCURL_MENTIONS_URL = "http://twitter.com/statuses/mentions.xml";
    const std::string TWITCURL_USERTIMELINE_URL = "http://twitter.com/statuses/user_timeline.xml";

    /* Users URLs */
    const std::string TWITCURL_SHOWUSERS_URL = "http://twitter.com/users/show.xml";
    const std::string TWITCURL_SHOWFRIENDS_URL = "http://twitter.com/statuses/friends.xml";
    const std::string TWITCURL_SHOWFOLLOWERS_URL = "http://twitter.com/statuses/followers.xml";

    /* Direct messages URLs */
    const std::string TWITCURL_DIRECTMESSAGES_URL = "http://twitter.com/direct_messages.xml";
    const std::string TWITCURL_DIRECTMESSAGENEW_URL = "http://twitter.com/direct_messages/new.xml";
    const std::string TWITCURL_DIRECTMESSAGESSENT_URL = "http://twitter.com/direct_messages/sent.xml";
    const std::string TWITCURL_DIRECTMESSAGEDESTROY_URL = "http://twitter.com/direct_messages/destroy/";

    /* Friendships URLs */
    const std::string TWITCURL_FRIENDSHIPSCREATE_URL = "http://twitter.com/friendships/create.xml";
    const std::string TWITCURL_FRIENDSHIPSDESTROY_URL = "http://twitter.com/friendships/destroy.xml";
    const std::string TWITCURL_FRIENDSHIPSSHOW_URL = "http://twitter.com/friendships/show.xml";

    /* Social graphs URLs */
    const std::string TWITCURL_FRIENDSIDS_URL = "http://twitter.com/friends/ids.xml";
    const std::string TWITCURL_FOLLOWERSIDS_URL = "http://twitter.com/followers/ids.xml";

    /* Account URLs */
    const std::string TWITCURL_ACCOUNTRATELIMIT_URL = "http://twitter.com/account/rate_limit_status.xml";
    const std::string TWITCURL_VERIFYCREDENTIALS_URL = "http://api.twitter.com/1/account/verify_credentials.xml";

    /* Favorites URLs */
    const std::string TWITCURL_FAVORITESGET_URL = "http://twitter.com/favorites.xml";
    const std::string TWITCURL_FAVORITECREATE_URL = "http://twitter.com/favorites/create/";
    const std::string TWITCURL_FAVORITEDESTROY_URL = "http://twitter.com/favorites/destroy/";

    /* Block URLs */
    const std::string TWITCURL_BLOCKSCREATE_URL = "http://twitter.com/blocks/create/";
    const std::string TWITCURL_BLOCKSDESTROY_URL = "http://twitter.com/blocks/destroy/";
    
    /* Saved Search URLs */
    const std::string TWITCURL_SAVEDSEARCHGET_URL = "http://twitter.com/saved_searches.xml";
    const std::string TWITCURL_SAVEDSEARCHSHOW_URL = "http://twitter.com/saved_searches/show/";
    const std::string TWITCURL_SAVEDSEARCHCREATE_URL = "http://twitter.com/saved_searches/create.xml";
    const std::string TWITCURL_SAVEDSEARCHDESTROY_URL = "http://twitter.com/saved_searches/destroy/";
    
    /* OAuth URLs */
    const std::string TWITCURL_OAUTH_REQUEST_TOKEN_URL = "http://api.twitter.com/oauth/request_token";
  	const std::string TWITCURL_OAUTH_ACCESS_TOKEN_URL = "http://api.twitter.com/oauth/access_token";
  	const std::string TWITCURL_OAUTH_AUTHORIZE_URL = "https://twitter.com/oauth/authorize";
    
};

/* twitCurl class */
class twitCurl
{
public:
    twitCurl();
    ~twitCurl();

    /* Twitter login APIs: Deprecated after OAuth implementation */
    std::string& getTwitterUsername();
    std::string& getTwitterPassword();
    void setTwitterUsername( std::string& userName );
    void setTwitterPassword( std::string& passWord );
    
    /* OAuth access tokens */
    std::string& getAccessKey();
    std::string& getAccessSecret();
    void setAccessKey( std::string& accessKey );
    void setAccessSecret( std::string& accessSecret );

    /* Twitter search APIs */
    bool search( std::string& query );
    
    /* Twitter trends APIs */
    bool trendsGet();
    bool trendsCurrentGet();

    /* Twitter status APIs */
    bool statusUpdate( std::string& newStatus, const char* replyTo = ""  );
    bool statusShowById( std::string& statusId );
    bool statusDestroyById( std::string& statusId );

    /* Twitter timeline APIs */
    bool timelineHomeGet();
    bool timelinePublicGet();
    bool timelineFriendsGet();
    bool timelineUserGet( std::string userInfo = "", bool isUserId = false );
    bool featuredUsersGet();
    bool mentionsGet();

    /* Twitter user APIs */
    bool userGet( std::string& userInfo, bool isUserId = false );
    bool friendsGet( std::string userInfo = "", bool isUserId = false );
    bool followersGet( std::string userInfo = "", bool isUserId = false );

    /* Twitter direct message APIs */
    bool directMessageGet();
    bool directMessageSend( std::string& userInfo, std::string& dMsg, bool isUserId = false );
    bool directMessageGetSent();
    bool directMessageDestroyById( std::string& dMsgId );

    /* Twitter friendships APIs */
    bool friendshipCreate( std::string& userInfo, bool isUserId = false );
    bool friendshipDestroy( std::string& userInfo, bool isUserId = false );
    bool friendshipShow( std::string& userInfo, bool isUserId = false );

    /* Twitter social graphs APIs */
    bool friendsIdsGet( std::string& userInfo, bool isUserId = false );
    bool followersIdsGet( std::string& userInfo, bool isUserId = false );

    /* Twitter account APIs */
    bool accountRateLimitGet();
    bool verifyCredentials();

    /* Twitter favorites APIs */
    bool favoriteGet();
    bool favoriteCreate( std::string& statusId );
    bool favoriteDestroy( std::string& statusId );

    /* Twitter block APIs */
    bool blockCreate( std::string& userInfo );
    bool blockDestroy( std::string& userInfo );

    /* Twitter search APIs */
    bool savedSearchGet();
    bool savedSearchCreate( std::string& query );
    bool savedSearchShow( std::string& searchId );
    bool savedSearchDestroy( std::string& searchId );
    
    /* cURL APIs */
    bool isCurlInit();
    void getLastWebResponse( std::string& outWebResp );
    void getLastCurlError( std::string& outErrResp );

    /* Internal cURL related methods */
    int saveLastWebResponse( char*& data, size_t size );
    static int curlCallback( char* data, size_t size, size_t nmemb, twitCurl* pTwitCurlObj );

    /* cURL proxy APIs */
    std::string& getProxyServerIp();
    std::string& getProxyServerPort();
    std::string& getProxyUserName();
    std::string& getProxyPassword();
    void setProxyServerIp( std::string& proxyServerIp );
    void setProxyServerPort( std::string& proxyServerPort );
    void setProxyUserName( std::string& proxyUserName );
    void setProxyPassword( std::string& proxyPassword );
    
    /* OAuth methods */
    std::string oauthGetAuthorizeUrl();
    bool oauthAuthorize(std::string oauth_verify);

private:
    /* cURL data */
    CURL* m_curlHandle;
    char m_errorBuffer[twitCurlDefaults::TWITCURL_DEFAULT_BUFFSIZE];
    std::string m_callbackData;

    /* cURL flags */
    bool m_curlProxyParamsSet;
    bool m_curlLoginParamsSet;
    bool m_curlCallbackParamsSet;

    /* cURL proxy data */
    std::string m_proxyServerIp;
    std::string m_proxyServerPort;
    std::string m_proxyUserName;
    std::string m_proxyPassword;

    /* Twitter data */
    std::string m_twitterUsername;
    std::string m_twitterPassword;

    /* Private methods */
    void clearCurlCallbackBuffers();
    void prepareCurlProxy();
    void prepareCurlCallback();
    void prepareCurlUserPass();
    void prepareStandardParams();
    bool performGet( const std::string& getUrl );
    bool performDelete( const std::string& deleteUrl );
    bool performPost( const std::string& postUrl, std::string dataStr = "" );
    
    /* OAuth data */
    std::string oauthAccessKey;
    std::string oauthAccessSecret;
    
    /* OAuth methods */
    bool oauthParseReply(const char *reply, char **token, char** secret);
};


/* Private functions */
void utilMakeCurlParams( std::string& outStr, std::string& inParam1, std::string& inParam2 );
void utilMakeUrlForUser( std::string& outUrl, const std::string& baseUrl, std::string& userInfo, bool isUserId );

#endif // _TWITCURL_H_
