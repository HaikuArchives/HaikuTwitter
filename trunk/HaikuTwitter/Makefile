APP = HaikuTwitter
INCS = xercesc/src

${APP} :: ${APP}.cpp twitcurl/twitcurl.cpp TimeLineParser.cpp HTGTextView.cpp HTGTimeLineView.cpp HTTweet.cpp HTGTimeLineWindow.cpp HTGNewTweetWindow.cpp HTGTweetItem.cpp HTGAccountSettingsWindow.cpp HTGAboutWindow.cpp
	c++ -static -L /boot/common/lib/ -lbe -lcurl -ltranslation -L ${INCS}/.libs/ -lxerces-c -I${INCS} ${APP}.cpp twitcurl/twitcurl.cpp TimeLineParser.cpp HTGAccountSettingsWindow.cpp HTGAboutWindow.cpp HTGTweetItem.cpp HTTweet.cpp HTGTimeLineView.cpp HTGTextView.cpp HTGTimeLineWindow.cpp HTGNewTweetWindow.cpp -o ${APP}
