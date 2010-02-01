APP = HaikuTwitter
INCS = xercesc/src
SVNDEV = -D'SVN_REV="$(shell svnversion -n .)"'

${APP} :: ${APP}.cpp twitcurl/twitcurl.cpp TimeLineParser.cpp HTGTweetTextView.cpp HTGTextView.cpp HTGTimeLineView.cpp HTTweet.cpp HTGMainWindow.cpp HTGNewTweetWindow.cpp HTGTweetItem.cpp HTGAccountSettingsWindow.cpp HTGTimeLineWindow.cpp
	c++ -static $(SVNDEV) -L /boot/common/lib/ -lbe -lcurl -ltranslation -L ${INCS}/.libs/ -lxerces-c -I${INCS} ${APP}.cpp twitcurl/twitcurl.cpp TimeLineParser.cpp HTGTweetTextView.cpp HTGAccountSettingsWindow.cpp HTGTweetItem.cpp HTTweet.cpp HTGTimeLineWindow.cpp HTGTimeLineView.cpp HTGTextView.cpp HTGMainWindow.cpp HTGNewTweetWindow.cpp -o ${APP}
