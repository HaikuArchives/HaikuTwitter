# Uncomment these two lines to enable InfoPopper support
 LIBS = -linfopopper
 INFOPOPPER = -DINFOPOPPER_SUPPORT

### Nothing below this should be changed ###

APP = HaikuTwitter
CC = c++
LIBS += -L /boot/common/lib/ -lbe -ltranslation -lcurl -lxerces-c
SVNDEV = -D'SVN_REV="$(shell svnversion -n .)"'
CFLAGS = $(SVNDEV) ${INFOPOPPER}
BDIR = bin
ODIR = ${BDIR}/obj

all: ${APP}

_OBJ = HaikuTwitter.o twitcurl.o SearchParser.o SmartTabView.o HTGSearchForWindow.o TimeLineParser.o HTGTweetTextView.o HTGInfoPopperSettingsWindow.o HTGTextView.o HTGTimeLineView.o HTGGoToUserWindow.o HTTweet.o HTGMainWindow.o HTGNewTweetWindow.o HTGTweetItem.o HTGAccountSettingsWindow.o HTGTimeLineWindow.o
OBJ =  $(patsubst %,$(ODIR)/%, $(_OBJ))

${APP}: ${OBJ}
	${CC} ${LIBS} $^ -o ${BDIR}/${APP}

${ODIR}/twitcurl.o: twitcurl/twitcurl.cpp
	${CC} -c -o $@ $< ${CFLAGS}

${ODIR}/%.o: %.cpp
	${CC} -c -o $@ $< ${CFLAGS}
	
clean:
	rm -rf ${ODIR}/*.o
	rm -rf ${BDIR}/${APP}

install:
	mkdir -p /boot/apps/${APP}
	cp ${BDIR}/HaikuTwitter /boot/apps/${APP}/
	ln -s -f /boot/apps/${APP}/${APP} /boot/home/config/be/Applications/${APP}
