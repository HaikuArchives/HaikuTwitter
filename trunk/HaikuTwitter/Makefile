# Uncomment these two lines to enable InfoPopper support
# LIBS = -linfopopper
# INFOPOPPER = -DINFOPOPPER_SUPPORT

### Nothing below this should be changed ###

APP = HaikuTwitter
CC = c++
LIBS += -L /boot/common/lib/ -lbe -ltranslation -lcurl -lxerces-c
SVNDEV = -D'SVN_REV="$(shell svnversion -n .)"'

CFLAGS = $(SVNDEV) ${INFOPOPPER}

all: ${APP}

${APP}: HaikuTwitter.o twitcurl.o TimeLineParser.o HTGTweetTextView.o HTGInfoPopperSettingsWindow.o HTGTextView.o HTGTimeLineView.o HTGGoToUserWindow.o HTTweet.o HTGMainWindow.o HTGNewTweetWindow.o HTGTweetItem.o HTGAccountSettingsWindow.o HTGTimeLineWindow.o
	${CC} ${LIBS} HaikuTwitter.o twitcurl.o TimeLineParser.o HTGTweetTextView.o HTGInfoPopperSettingsWindow.o HTGTextView.o HTGTimeLineView.o HTGGoToUserWindow.o HTTweet.o HTGMainWindow.o HTGNewTweetWindow.o HTGTweetItem.o HTGAccountSettingsWindow.o HTGTimeLineWindow.o -o ${APP}

HaikuTwitter.o: HaikuTwitter.cpp
	${CC} -c HaikuTwitter.cpp ${CFLAGS}

twitcurl.o: twitcurl/twitcurl.cpp
	${CC} -c twitcurl/twitcurl.cpp ${CFLAGS}

TimeLineParser.o: TimeLineParser.cpp
	${CC} -c TimeLineParser.cpp ${CFLAGS}
	
HTGTweetTextView.o: HTGTweetTextView.cpp
	${CC} -c HTGTweetTextView.cpp ${CFLAGS}
	
HTGInfoPopperSettingsWindow.o: HTGInfoPopperSettingsWindow.cpp
	${CC} -c HTGInfoPopperSettingsWindow.cpp ${CFLAGS}

HTGTextView.o: HTGTextView.cpp
	${CC} -c HTGTextView.cpp ${CFLAGS}

HTGTimeLineView.o: HTGTimeLineView.cpp
	${CC} -c HTGTimeLineView.cpp ${CFLAGS}
	
HTGGoToUserWindow.o: HTGGoToUserWindow.cpp
	${CC} -c HTGGoToUserWindow.cpp ${CFLAGS}
	
HTTweet.o: HTTweet.cpp
	${CC} -c HTTweet.cpp ${CFLAGS}

HTGMainWindow.o: HTGMainWindow.cpp
	${CC} -c HTGMainWindow.cpp ${CFLAGS}
	
HTGNewTweetWindow.o: HTGNewTweetWindow.cpp
	${CC} -c HTGNewTweetWindow.cpp ${CFLAGS}
	
HTGTweetItem.o: HTGTweetItem.cpp
	${CC} -c HTGTweetItem.cpp ${CFLAGS}
	
HTGAccountSettingsWindow.o: HTGAccountSettingsWindow.cpp
	${CC} -c HTGAccountSettingsWindow.cpp ${CFLAGS}
	
HTGTimeLineWindow.o: HTGTimeLineWindow.cpp
	${CC} -c HTGTimeLineWindow.cpp ${CFLAGS}

clean:
	rm -rf *.o

install:
	mkdir -p /boot/apps/HaikuTwitter
	cp HaikuTwitter /boot/apps/HaikuTwitter/
	ln -s /boot/apps/HaikuTwitter/HaikuTwitter /boot/home/be/Applications/HaikuTwitter
