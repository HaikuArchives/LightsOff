#ifndef PREFERENCES_H_
#define PREFERENCES_H_

#include <Locker.h>
#include <Message.h>
#include <File.h>

extern BLocker prefsLock;
extern BMessage preferences;

#define PREFERENCES_PATH "/boot/home/config/settings/LightsOff"

status_t SavePreferences(const char *path);
status_t LoadPreferences(const char *path);

#endif
