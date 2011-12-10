# Uncomment this line to enable more debugging information (printed to console)
#DEBUG = -DDEBUG_ENABLED

### Nothing below this should be changed ###
APP = HaikuTwitter
CC = gcc
LIBS += -L/boot/common/lib/ -lbe -ltranslation -lcurl -lstdc++ -lcrypto
SVNDEV = -D'SVN_REV="$(shell svnversion -n .)"'
CFLAGS = -g $(SVNDEV) ${DEBUG}
BDIR = bin
ODIR = ${BDIR}/obj

all: ${APP}

_OBJ = HaikuTwitter.o AnimationHelper.o HTGTweetMenuItem.o HTGErrorHandling.o HTGLogoView.o HTGAuthorizeWindow.o
_OBJ += oauth.o oauth_http.o xmalloc.o twitcurl.o SmartTabView.o HTGSearchForWindow.o HTGTweetTextView.o
_OBJ += HTGInfoPopperSettingsWindow.o HTGTextView.o HTGTimeLineView.o HTGGoToUserWindow.o HTTweet.o HTGMainWindow.o
_OBJ += HTGNewTweetWindow.o HTGTweetItem.o HTGAccountSettingsWindow.o HTGTimeLineWindow.o HTStorage.o HTGTweetViewWindow.o
_OBJ += HyperTextActions.o HTGAvatarView.o HTGStatusBar.o HTAccountCredentials.o HTGListView.o HTTimelineParser.o
_OBJ += HTSearchParser.o entities.o

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
	
${ODIR}/%.o: %.c
	${CC} -c -std=c99 -o $@ $< ${CFLAGS}
	
clean:
	rm -rf ${ODIR}/*.o
	rm -rf ${BDIR}/${APP}
	rm -rf ${ODIR}/${APP}.rsrc

install:
	mkdir -p /boot/apps/${APP}
	cp ${BDIR}/HaikuTwitter /boot/apps/${APP}/
	cp LICENSE /boot/apps//${APP}/LICENSE.txt
	ln -s -f /boot/apps/${APP}/${APP} /boot/home/config/be/Applications/${APP}
