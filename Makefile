# Uncomment this line to enable more debugging information (printed to console)
#DEBUG = -DDEBUG_ENABLED

### Nothing below this should be changed ###

APP = HaikuTwitter
CC = gcc
LIBS += -L/boot/common/lib/ -lbe -ltranslation -lcurl -lxerces-c -lstdc++ -lcrypto
SVNDEV = -D'SVN_REV="$(shell svnversion -n .)"'
CFLAGS = $(SVNDEV) ${DEBUG}
BDIR = bin
ODIR = ${BDIR}/obj

all: ${APP}

_OBJ = HaikuTwitter.o AnimationHelper.o HTGTweetMenuItem.o HTGErrorHandling.o HTGLogoView.o HTGAuthorizeWindow.o oauth.o oauth_http.o xmalloc.o twitcurl.o SearchParser.o SmartTabView.o HTGSearchForWindow.o TimeLineParser.o HTGTweetTextView.o HTGInfoPopperSettingsWindow.o HTGTextView.o HTGTimeLineView.o HTGGoToUserWindow.o HTTweet.o HTGMainWindow.o HTGNewTweetWindow.o HTGTweetItem.o HTGAccountSettingsWindow.o HTGTimeLineWindow.o DirectMessageParser.o HTStorage.o HTGTweetViewWindow.o HyperTextActions.o
OBJ =  $(patsubst %,$(ODIR)/%, $(_OBJ))

${APP}: ${OBJ}
	${CC} ${LIBS} $^ -o ${BDIR}/${APP}
	rc -o ${ODIR}/${APP}.rsrc ${APP}.rdef
	xres -o ${BDIR}/${APP} ${ODIR}/${APP}.rsrc
	
${ODIR}/twitcurl.o: twitcurl/twitcurl.cpp
	${CC} -c -o $@ $< ${CFLAGS}
	
${ODIR}/%.o: oauth/%.c
	${CC} -c -o $@ $< ${CFLAGS}

${ODIR}/%.o: %.cpp
	${CC} -c -o $@ $< ${CFLAGS}
	
clean:
	rm -rf ${ODIR}/*.o
	rm -rf ${BDIR}/${APP}
	rm -rf ${ODIR}/${APP}.rsrc

install:
	mkdir -p /boot/apps/${APP}
	cp ${BDIR}/HaikuTwitter /boot/apps/${APP}/
	cp LICENSE /boot/apps//${APP}/LICENSE.txt
	ln -s -f /boot/apps/${APP}/${APP} /boot/home/config/be/Applications/${APP}
