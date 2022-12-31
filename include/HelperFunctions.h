/*
 * Wafa Qazi: 0932477
 */

#include <stdint.h>
#include <stdio.h>

typedef enum flags { VCALENDAR, VEVENT, VERSION, PRODID, CALSCALE, METHOD, VALARM, 
					 CLASS, CREATED, DESCRIPTION, DTEND, DTSTART, DURATION, GEO, LASTMOD, LOCATION, ORGANIZER,
					 PRIORITY, DTSTAMP, SEQ, STATUS, SUMMARY, TRANSP, UID, URL,	RECURID, ACTION, TRIGGER } Flag;


typedef struct uiEventInfo {
	int numProps;
	int numAlarms;
	char* summary;
	void* event;
} UIEventInfo;

typedef struct uiDBInfo {
	char* organizer;
	char* summary;
	char* location;
	int numAlarms;
	DateTime startTime;
	void* event; 
} UIDBInfo;

bool badFileName(char *fileName);

char* getContentLine(FILE*);

ICalErrorCode parseDateTime(char *str, Event** evt, bool);
ICalErrorCode parseAlarm(FILE* fp, Event** evt, uint64_t* flags);
ICalErrorCode parseEvent(FILE* fp, Calendar** obj, uint64_t* flags);
ICalErrorCode parseCalendar(FILE* fp, Calendar** obj);

char *printAlarm(void *);
char *printProperty(void *);
char *printEvent(void *);

void deleteAlarm(void *);
void deleteProperty(void *);
void deleteEvent(void *);

int compareAlarms(const void *, const void *);
int compareProperties(const void *, const void *);
int compareEvents(const void *, const void *);

bool equalProps(const void *, const void *);
bool equalEvents(const void *, const void *);
bool equalAlarms(const void *, const void *);

void getDBTablePropertiesForUI(void* voidCalendar, UIEventInfo** uiarray, int* uilength);
int createCalendarForUI(char* fileName, void** h);
void getEventsListForUI(void* voidCalendar, UIEventInfo** uiarray, int* uilength);
char* printAlarmsForEventUI(void* event);
char* printPropertiesForEventUI(void* event);
int validateCalendarForUI(void* c);
void* createNewCalendarForUI(char* pid);
void* createNewEventForUI(char* uid, DateTime creationDate, DateTime startDate);
void* addEventToCalendarForUI(void* calendar, void* event);
int writeCalendarForUI(char* fileName, void* calendar);
