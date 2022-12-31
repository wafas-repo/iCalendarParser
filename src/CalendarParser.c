#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "CalendarParser.h"
#include "HelperFunctions.h"

bool badFileName(char *fileName) {

	char *ext;
	if (fileName == NULL || strlen(fileName) == 0)
		return true;
	if ((ext = strrchr(fileName, '.')) != NULL) {
		if (strcmp(ext, ".ics") != 0) {
			return true;
		}
	}
	else
		return true;
	return false;
}

ICalErrorCode parseDateTime(char *str, Event** evt, bool creationDateTime) {

	DateTime dt;
	memset(&dt, 0, sizeof(dt));
	char *token = strtok(str, "T");

	if (token == NULL || strlen(token) != 8)
		return INV_CREATEDT;

	memcpy(&dt.date, token, 8);

	token = strtok(NULL, "T");

/*	if (token == NULL || strlen(token) != 8 || strlen(token) != 7)
		return INV_CREATEDT;
*/
	memcpy(&dt.time, token, 6);

	if (strlen(token) > 6 && token[6] == 'Z')
		dt.UTC = true;

	if(creationDateTime)
		(*evt)->creationDateTime = dt;
	else
		(*evt)->startDateTime = dt;

	return OK;
}

char *printProperty(void *ptr) {

	Property *p = (Property*)ptr;
	char *str = malloc(10000);
	memset(str, 0, 10000);

	strcpy(str, "- ");
	strcpy(str + 2, p->propName);
	strcpy(str + strlen(p->propName) + 2, ":");
	strcpy(str + strlen(p->propName) + 3, p->propDescr);

	return str;
}

void deleteProperty(void *ptr) {

	Property *p = (Property*)ptr;
	free(p);
}

int compareProperties(const void *ptr1, const void *ptr2) {
	Property *p1 = (Property*)ptr1;
	Property *p2 = (Property*)ptr2;
	return (strcmp(p1->propName, p2->propName) + strcmp(p1->propDescr, p2->propDescr) == 0 ? true : false);
}

bool equalProps(const void *first, const void *second) {
	Property *p1 = (Property *)first;
	Property *p2 = (Property *)second;

	if(strcmp(p1->propName, p2->propName) == 0 && strcmp(p1->propName, p2->propName) == 0)
		return true;
	return false;
}

char *printAlarm(void *aptr) {

	Alarm *a = (Alarm*)aptr;
	char *str = malloc(10000);
	char *ptr = str;
	memset(str, 0, 10000);

	strcpy(ptr, "Action: ");
	ptr += 8;
	strcpy(ptr, a->action);
	ptr += strlen(a->action);
	strcpy(ptr, "\nTrigger: ");
	ptr += 10;
	strcpy(ptr, a->trigger);
	ptr += strlen(a->trigger);
	strcpy(ptr, "\n");
	ptr++;
	
	ListIterator liter = createIterator(a->properties);
	Property *currProp;

	if (getFromFront(a->properties) != NULL) {
		strcpy(ptr, "Properties:\n\t");
		ptr += 13;
	}

	while ((currProp = nextElement(&liter)) != NULL) {
		char *s = printProperty(currProp);
		strcpy(ptr, s);
		ptr += strlen(s);
		free(s);
	}

	return str;
}

void deleteAlarm(void *aptr) {

	Alarm *a = (Alarm*)aptr;

	free(a->trigger);
	free(a);
}

int compareAlarms(const void *aptr1, const void *aptr2) {
	Alarm *a1 = (Alarm*)aptr1;
	Alarm *a2 = (Alarm*)aptr2;
	return strcmp(a1->action, a2->action) + strcmp(a1->trigger, a2->trigger);
}

char* printEvent(void *eptr){
	return "";
}

void deleteEvent(void *eptr){
	;
}

int compareEvents(const void *eptr1, const void *eptr2){
	return 0;
}

char *getContentLine(FILE* fp){
	char c;
	char *line, *ptr;
	char curr[10000];
	int skip = 0;

	line = malloc(10000);
	memset(line, 0, 10000);
	ptr = line;

	while( fgets(curr, 10000, fp) != NULL ){
		skip = 0;
		if((c = fgetc(fp)) != EOF){
			if(c == '\t' || c == ' ')
				skip++;
			else 
				ungetc(c, fp);
		}
		strncpy(ptr, curr, strlen(curr));
		ptr += (strlen(curr)-2);
		if(!skip){
			strcpy(ptr, "\r\n\0");
			return line;
		}
	}
	return NULL;
}

ICalErrorCode parseAlarm(FILE* fp, Event** evt, uint64_t* flags) {

	char *line, *field, *value;
	
	Alarm *a = malloc(sizeof(Alarm));
	memset(a, 0, sizeof(Alarm));

	a->trigger = malloc(1000);
	memset(a->trigger, 0, 1000);

	while ( (line = getContentLine(fp)) != NULL ) {

		if (line[0] == ';')
			continue;

		if (line[strlen(line)-2] != '\r' || line[strlen(line)-1] != '\n')
			return INV_FILE;

		char *first = strpbrk(line, ":;");
		if(first){
			if(first[0] == ':'){
				field = strtok(line, ":");
			}
			else{
				field = strtok(line, ";");
			}
			value = field + strlen(field) + 1;
		}
		else{
			return INV_ALARM;
		}

		if (strcmp(field, "END") == 0) {
			if (strncmp(value, "VALARM", 6) == 0) {
				if ( ((*flags & (1 << VALARM)) == 0) || ((*flags & (1 << ACTION)) == 0) || ((*flags & (1 << TRIGGER)) == 0) ) {
					return INV_ALARM;
				}
				*flags &= ~(1 << VALARM);
				*flags &= ~(1 << ACTION);
				*flags &= ~(1 << TRIGGER);
				insertBack(&(*evt)->alarms, a);
				return OK;
			}
			else
				return INV_ALARM;
		}
		else if (strcmp(field, "BEGIN") == 0)
			return INV_ALARM;
		else if (strcmp(field, "ACTION") == 0) {
			if (*flags & (1 << ACTION)) {
				return INV_ALARM;
			}
			else if(value && strlen(value) < 3) {
				return INV_ALARM;
			}
			else {
				*flags |= (1 << ACTION);
				memcpy(a->action, value, strlen(value)-2);
			}
		}
		else if (strncmp(field, "TRIGGER", 7) == 0) {
			if (*flags & (1 << TRIGGER)) {
				return INV_ALARM;
			}
			else if(value && strlen(value) < 3) {
				return INV_ALARM;
			}
			else {
				*flags |= (1 << TRIGGER);
				memcpy(a->trigger, value, strlen(value)-2);
			}
		}
		else {
			// Optional properties
			if(!field || strlen(field) < 1)
				return INV_ALARM;
			if(!value || strlen(value) < 3)
				return INV_ALARM;
			if(strncmp(field, "X-", 2) == 0)
				return INV_ALARM;
			Property *p = malloc(sizeof(Property) + sizeof(char) * 10000);
			memset(p, 0, sizeof(Property) + sizeof(char) * 10000);
			memcpy(p->propName, field, strlen(field));
			memcpy(p->propDescr, value, strlen(value)-2);
			insertBack(&a->properties, p);
		}
		free(line);
	}
	return INV_ALARM;
}

ICalErrorCode parseEvent(FILE* fp, Calendar** obj, uint64_t* flags) {

	ICalErrorCode err;
	char *line, *field, *value;

	Event *e = malloc(sizeof(Event));
	memset(e, 0 , sizeof(Event));
	e->properties = initializeList(printProperty, deleteProperty, compareProperties);

	while ( (line = getContentLine(fp)) != NULL ) {
		
		if (line[0] == ';')
			continue;

		if (line[strlen(line)-2] != '\r' || line[strlen(line)-1] != '\n'){
			return INV_FILE;	
		}

		char *first = strpbrk(line, ":;");
		if(first){
			if(first[0] == ':'){
				field = strtok(line, ":");
			}
			else{
				field = strtok(line, ";");
			}
			value = field + strlen(field) + 1;
		}
		else{
			return INV_EVENT;
		}

		if (strcmp(field, "BEGIN") == 0) {
			if (strncmp(value, "VALARM", 6) == 0) {
				if (*flags & (1 << VALARM)) {
					return INV_EVENT;
				}
				
				*flags |= (1 << VALARM);

				if ( getFromFront(e->alarms) == NULL )
					e->alarms = initializeList(printAlarm, deleteAlarm, compareAlarms);

				if ((err = parseAlarm(fp, &e, flags)) != OK) {
					return err;
				}
			}
			else {
				return INV_EVENT;//continue;
			}
		}
		else if (strcmp(field, "END") == 0) {
			if (strncmp(value, "VEVENT", 6) == 0) {
				if ( ((*flags & (1 << VEVENT)) == 0) || ((*flags & (1 << UID)) == 0) 
				|| ((*flags & (1 << DTSTAMP)) == 0) ) {
					return INV_EVENT;
				}

				if((*flags & (1 << DTSTART)) == 0){
					*flags |= (1 << DTSTART);
					memcpy(&e->startDateTime, &e->creationDateTime, sizeof(DateTime));
				}

				if ((*flags & (1 << DTSTART)) == 0)
					return INV_EVENT;

				*flags &= ~(1 << VEVENT);
				*flags &= ~(1 << UID);
				*flags &= ~(1 << DTSTAMP);
				*flags &= ~(1 << DTSTART);

				insertBack(&(*obj)->events, e);
				return OK;
			}
			else
				return INV_EVENT;//continue;
		}
		else if (strcmp(field, "UID") == 0) {
			if (*flags & (1 << UID)) {
				return INV_EVENT;
			}
			else if (value && strlen(value) < 3){
				return INV_EVENT; 
			}
			else {
				*flags |= (1 << UID);
				memcpy(e->UID, value, strlen(value)-2);
			}
		}
		else if (strcmp(field, "DTSTAMP") == 0) {
			if (*flags & (1 << DTSTAMP)) {
				return INV_EVENT;
			}
			else if (value && strlen(value) < 3){
				return INV_EVENT;
			}
			else {
				*flags |= (1 << DTSTAMP);
				if ( (err = parseDateTime(value, &e, true)) != OK) {
					return err;
				}
			}
		}
		else if (strcmp(field, "DTSTART") == 0) {
			if (*flags & (1 << DTSTART)) {
				return INV_EVENT;
			}
			char *s = strpbrk(value, ":");
			if(s){
				value = s + 1;
			}
			if (value && strlen(value) < 3){
				return INV_EVENT;
			}
			else {
				*flags |= (1 << DTSTART);
				if ( (err = parseDateTime(value, &e, false)) != OK) {
					return err;
				}
			}
		}
		else {
			// Optional properties
			if(!field || strlen(field) < 1)
				return INV_EVENT;
			if(!value || strlen(value) < 3)
				return INV_EVENT;
			if(strncmp(field, "X-", 2) == 0)
				return INV_EVENT;
			if(strncmp(field, "DURATION", 8) == 0)
				return INV_EVENT;
			
			Property *p = malloc(sizeof(Property) + sizeof(char) * 10000);
			memset(p, 0, sizeof(Property) + sizeof(char) * 10000);
			memcpy(p->propName, field, strlen(field));
			memcpy(p->propDescr, value, strlen(value)-2);

			/*if(strncmp(field, "DTSTART", 7) == 0){
				char *tok = strpbrk(value, ":");	
				if(!tok){
					if( (err = parseDateTime(value, &e, false)) != OK) {
						return err;
					}
				}
				else{
					if( (err = parseDateTime(tok+1, &e, false)) != OK) {
						return err;
					}
				}
				*flags |= (1 << DTSTART);
				//continue;
			}*/
			insertBack(&e->properties, p);
		}
		free(line);
	}
	return INV_EVENT;
}

ICalErrorCode parseCalendar(FILE* fp, Calendar** obj) {

	ICalErrorCode err = OK;
	char *line, *field, *value;
	uint64_t flags = 0;

	(*obj)->events = initializeList(printEvent, deleteEvent, compareEvents);

	while ( (line = getContentLine(fp)) != NULL ) {
		
		if (line[0] == ';')
			continue;

		if (line[strlen(line)-2] != '\r' || line[strlen(line)-1] != '\n')
			return INV_CAL; 

		char *first = strpbrk(line, ":;");
		if(first){
			if(first[0] == ':'){
				field = strtok(line, ":");
			}
			else{
				field = strtok(line, ";");
			}
			value = field + strlen(field) + 1;
		}
		else{
			return INV_CAL;
		}
		
		if (strcmp(field, "BEGIN") == 0) {
			if (strncmp(value, "VCALENDAR", 9) == 0) {
				if (flags & (1 << VCALENDAR)) {
					return INV_CAL;
				}
				else {
					flags |= (1 << VCALENDAR);
				}
			}
			else if (strncmp(value, "VEVENT", 6) == 0) {
				if (flags & (1 << VEVENT)) {
					return INV_CAL;
				}
				else {
					flags |= (1 << VEVENT);
					if ((err = parseEvent(fp, obj, &flags)) != OK) {
						return err;
					}
				}
			}
			else 
				return INV_CAL;//continue;
		}
		else if (strcmp(field, "END") == 0) {
			if (strncmp(value, "VCALENDAR", 9) == 0) {
				if ( (flags & (1 << VCALENDAR)) == 0) {
					return INV_CAL;
				}
				else {
					flags &= ~(1 << VCALENDAR);
				}
			}
			else
				return INV_CAL;//continue;
		}
		else if (strcmp(field, "VERSION") == 0) {
			if (flags & (1 << VERSION))
				return DUP_VER;
			else if (strncmp(value, "2.0", 3) != 0){
				return INV_VER;
			}else{
				flags |= (1 << VERSION);
				(*obj)->version = atof(value);
			}
		}
		else if (strcmp(field, "PRODID") == 0) {
			
			if (flags & (1 << PRODID))
				return DUP_PRODID;
			else if(value && strlen(value) < 3) {
				return INV_PRODID;
			}
			else {
				if(value && strlen(value) > 1) {
					flags |= (1 << PRODID);
					memcpy((*obj)->prodID, value, strlen(value)-2);
				}
			}
		}
		else{
			//additional properties;
			if(!field || strlen(field) < 1)
				return INV_CAL;
			if(!value || strlen(value) < 3)
				return INV_CAL;
			if(strncmp(field, "X-", 2) == 0)
				return INV_CAL;
			Property *p = malloc(sizeof(Property) + sizeof(char) * 10000);
			memset(p, 0, sizeof(Property) + sizeof(char) * 10000);
			memcpy(p->propName, field, strlen(field));
			memcpy(p->propDescr, value, strlen(value)-2);

			if(findElement((*obj)->properties, equalProps, p) != NULL)
				return INV_CAL;

			insertBack(&(*obj)->properties, p);
		}
		free(line);
	}
	if ((flags & (1<<VERSION)) == 0 ||(flags & (1<<PRODID)) == 0 || (flags & (1<<VCALENDAR)) )
		return INV_CAL;
	if ( getLength((*obj)->events) < 1 ) 
		return INV_CAL;
	return OK;
}
/** Function to create a Calendar object based on the contents of an iCalendar file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ics extension.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid calendar has been created, its address was stored in the variable obj, and OK was returned
		or 
		An error occurred, the calendar was not created, all temporary memory was freed, obj was set to NULL, and the 
		appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param fileName - a string containing the name of the iCalendar file
 *@param a double pointer to a Calendar struct
**/
ICalErrorCode createCalendar(char* fileName, Calendar** obj)
{
	 FILE* fp;
	 ICalErrorCode err = OK;

	 if (badFileName(fileName) || (fp = fopen(fileName, "r")) == NULL) {
		 obj = NULL;
		 return INV_FILE;
	 }
	 
	 *obj = malloc(sizeof(Calendar));
	 memset(*obj, 0, sizeof(Calendar));
	 //(*obj)->events = malloc(sizeof(Event));
	 //memset((*obj)->event, 0, sizeof(Event));

	 if ( (err = parseCalendar(fp, obj)) != OK) {
		 deleteCalendar(*obj);
	 }
	 fclose(fp);
	 
	 return err;
}


/** Function to delete all calendar content and free all the memory.
 *@pre Calendar object exists, is not null, and has not been freed
 *@post Calendar object had been freed
 *@return none
 *@param obj - a pointer to a Calendar struct
**/
void deleteCalendar(Calendar* obj) {

	//clearList(&obj->event->alarms);
	//clearList(&obj->event->properties);
	//free(obj->event->UID);
	//free(obj->event);	
	free(obj);
	//obj = NULL;
}


/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calndar contents has been created
 *@return a string contaning a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
**/
char* printCalendar(const Calendar* obj) {

	ListIterator itera, iterp, itere;
	Alarm *currAlarm;
	Event *currEvent;
	Property *currProp;

	if (obj == NULL)
		return "";

	char *str = malloc(sizeof(char) * 100000);
	char *ptr = str;

	memset(str, 0, 100000);

	strcpy(ptr, "Calendar: version = ");
	ptr += 20;
	snprintf(ptr, 4, "%f", obj->version);
	ptr += 3; //one less because snprintf appends a null character
	strcpy(ptr, ", prodID = ");
	ptr += 11;
	strcpy(ptr, obj->prodID);
	ptr += strlen(obj->prodID);


	itere = createIterator(obj->events);

	while ((currEvent = nextElement(&itere)) != NULL) {
		strcpy(ptr, "\nEvent\n\tUID = ");
		ptr += 14;
		strcpy(ptr, currEvent->UID);
		ptr += strlen(currEvent->UID);
		strcpy(ptr, "\n\tcreationDateTime = ");
		ptr += 21;
		strncpy(ptr, currEvent->creationDateTime.date, 8);
		ptr += strlen(currEvent->creationDateTime.date);
		strcpy(ptr, ":");
		ptr += 1;
		strcpy(ptr, currEvent->creationDateTime.time);
		ptr += strlen(currEvent->creationDateTime.time);
		strcpy(ptr, ", UTC=");
		ptr += 6;
		snprintf(ptr, 2, "%d\n", currEvent->creationDateTime.UTC);
		ptr += 1;
		
		if (getFromFront(currEvent->alarms)) {
			strcpy(ptr, "\n\tAlarms:");
			ptr += 9;
		}

		itera = createIterator(currEvent->alarms);

		while ((currAlarm = nextElement(&itera)) != NULL) {

			strcpy(ptr, "\n\t\tAction: ");
			ptr += 11;
			strcpy(ptr, currAlarm->action);
			ptr += strlen(currAlarm->action);
			strcpy(ptr, "\n\t\tTrigger:");
			ptr += 11;
			strcpy(ptr, currAlarm->trigger);
			ptr += strlen(currAlarm->trigger);

			if (getFromFront(currAlarm->properties)) {
				strcpy(ptr, "\n\t\tProperties:");
				ptr += 14;
			}

			iterp = createIterator(currAlarm->properties);

			while ((currProp = nextElement(&iterp)) != NULL) {
				char *s = printProperty(currProp);
				strcpy(ptr, "\n\t\t");
				ptr += 3;
				strcpy(ptr, s);
				ptr += strlen(s);
				free(s);
			}
			strcpy(ptr, "\n");
			ptr += 1;
		}

		if(getFromFront(currEvent->properties) == NULL)
			return str;

		strcpy(ptr, "\n\tOther propertries:");
		ptr += 20;

		iterp = createIterator(currEvent->properties);

		while ((currProp = nextElement(&iterp)) != NULL) {
			char *s = printProperty(currProp);
			strcpy(ptr, "\n\t\t");
			ptr += 3;
			strcpy(ptr, s);
			ptr += strlen(s);
			free(s);
		}
	}
	
	if(getFromFront(obj->properties) == NULL)
		return str;

	strcpy(ptr, "\nCalendar Properties:");
	ptr += 21;

	iterp = createIterator(obj->properties);

	while ((currProp = nextElement(&iterp)) != NULL) {
		char *s = printProperty(currProp);
		strcpy(ptr, "\n\t");
		ptr += 2;
		strcpy(ptr, s);
		ptr += strlen(s);
		free(s);
	}

	return str;
}

/** Function to "convert" the ErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into 
          the descr array using rhe error code enum value as an index
 *@param err - an error code
**/
char* printError(ICalErrorCode err) {

	char *error = malloc(100);
	memset(error, 0, 100);

	switch (err) {
		case INV_FILE:	
			error = "Invalid File";
			break;
		case INV_CAL:
			error = "Invalid Calendar";
			break;
		case INV_VER:
			error = "Invalid Version";
			break;
		case DUP_VER:
			error = "Duplicate Version";
			break;
		case INV_PRODID:
			error = "Invalid Product ID";
			break;
		case DUP_PRODID:
			error = "Duplicate Product ID";
			break;
		case INV_EVENT:
			error = "Invalid Event";
			break;
		case INV_CREATEDT:
			error = "Invalid Create Date-Time";
			break;
		case OK:
			error = "Okay";
			break;
		default: 
			error = "Other Error";
	}

	return error;
}

ICalErrorCode writeCalendar(char* fileName, const Calendar* obj) {

	ListIterator itera, iterp, itere;
	Alarm *currAlarm;
	Property *currProp;
	Event *currEvent;
	FILE* fp;

	char *str = malloc(sizeof(char) * 1000000);
	char *ptr = str;

	memset(str, 0, 1000000);


	if ((fp = fopen(fileName, "w")) == NULL)
		return INV_FILE; 

	
	strcpy(ptr, "BEGIN:VCALENDAR\r\nPRODID:");
	ptr += 24;
	strcpy(ptr, obj->prodID);
	ptr += strlen(obj->prodID);
	strcpy(ptr, "\r\nVERSION:");
	ptr += 10;
	snprintf(ptr, 6, "%.1f\r\n", obj->version);
	ptr += 5; //one less because snprintf appends a null character


	iterp = createIterator(obj->properties);
	while ((currProp = nextElement(&iterp)) != NULL) {

		strcpy(ptr, currProp->propName);
		ptr += strlen(currProp->propName);
		strcpy(ptr, ":");
		ptr += 1;
		strcpy(ptr, currProp->propDescr);
		ptr += strlen(currProp->propDescr);
		strcpy(ptr, "\r\n");
		ptr += 2;
	}

	itere = createIterator(obj->events);
	while((currEvent = nextElement(&itere)) != NULL) {
		strcpy(ptr, "BEGIN:VEVENT\r\n");
		ptr += 14;
		strcpy(ptr, "UID:");
		ptr += 4;
		strcpy(ptr, currEvent->UID);
		ptr += strlen(currEvent->UID);
		strcpy(ptr, "\r\nDTSTAMP:");
		ptr += 10;
		strncpy(ptr, currEvent->creationDateTime.date, 8);
		ptr += 8;
		strcpy(ptr, "T");
		ptr += 1;
		strncpy(ptr, currEvent->creationDateTime.time, 6);
		ptr += 6;
		if(currEvent->creationDateTime.UTC){
			strcpy(ptr, "Z");
			ptr += 1;
		}
		strcpy(ptr, "\r\n");
		ptr += 2;

		/*strcpy(ptr, "DTSTART:");
		ptr += 8;
		strncpy(ptr, currEvent->startDateTime.date, 8);
		ptr += 8;
		strcpy(ptr, "T");
		ptr += 1;
		strncpy(ptr, currEvent->startDateTime.time, 6);
		ptr += 6;
		if(currEvent->startDateTime.UTC){
			strcpy(ptr, "Z");
			ptr += 1;
		}
		strcpy(ptr, "\r\n");
		ptr += 2;*/

		//Write additional Event properties
		iterp = createIterator(currEvent->properties);

		while ((currProp = nextElement(&iterp)) != NULL) {
			
			if(strncmp(currProp->propName, "ORGANIZER", 9) == 0){
				strcpy(ptr, "ORGANIZER;");
				ptr += 10;
			}
			else{	
				strcpy(ptr, currProp->propName);
				ptr += strlen(currProp->propName);
				strcpy(ptr, ":");
				ptr += 1;
			}
			strcpy(ptr, currProp->propDescr);
			ptr += strlen(currProp->propDescr);
			strcpy(ptr, "\r\n");
			ptr += 2;
		}

		//Write all alarms 
		itera = createIterator(currEvent->alarms);

		while ((currAlarm = nextElement(&itera)) != NULL) {

			strcpy(ptr, "BEGIN:VALARM\r\n");
			ptr += 14;
			if(strncmp(currAlarm->trigger, "VALUE=DATE-TIME", 15) == 0){
				strcpy(ptr, "TRIGGER;VALUE=DATE-TIME:");
				ptr += 24;
				currAlarm->trigger += 16;
			}
			else{
				strcpy(ptr, "TRIGGER:");
				ptr += 8;
				
			}
			strcpy(ptr, currAlarm->trigger);
			ptr += strlen(currAlarm->trigger);
			strcpy(ptr, "\r\nACTION:");
			ptr += 9;
			strcpy(ptr, currAlarm->action);
			ptr += strlen(currAlarm->action);
			strcpy(ptr, "\r\n");
			ptr += 2;

			if (getFromFront(currAlarm->properties)) {

				iterp = createIterator(currAlarm->properties);

				while ((currProp = nextElement(&iterp)) != NULL) {
					strcpy(ptr, currProp->propName);
					ptr += strlen(currProp->propName);
					strcpy(ptr, ":");
					ptr += 1;
					strcpy(ptr, currProp->propDescr);
					ptr += strlen(currProp->propDescr);
					strcpy(ptr, "\r\n");
					ptr += 2;
				}
			}

			strcpy(ptr, "END:VALARM\r\n");
			ptr += 12;
		}
	
		strcpy(ptr, "END:VEVENT\r\n");
		ptr += 12;
	}

	strcpy(ptr, "END:VCALENDAR\r\n");

	if( fprintf(fp, str) < 0 )
		return WRITE_ERROR;

	free(str);	
	fclose(fp);

	return OK;
}

ICalErrorCode validateCalendar(const Calendar *obj){

	ListIterator itera, iterp, itere;
	Alarm *currAlarm;
	Property *currProp;
	Event *currEvent;
	List props = initializeList(printProperty, deleteProperty, compareProperties);
	
	printf(printCalendar(obj));

	if( obj == NULL )
		return INV_CAL;
	if( obj->version < 1.99999 || obj->version > 2.00001 )
		return INV_CAL;
	if( !obj->prodID )
		return INV_CAL;
	if( strlen(obj->prodID) < 1 )
		return INV_CAL;
	if( getLength(obj->events) < 1 )
		return INV_CAL;


	iterp = createIterator(obj->properties);
	while ((currProp = nextElement(&iterp)) != NULL) {
		if(strcmp(currProp->propName, "PRODID") == 0)
			return INV_CAL;
		if(strcmp(currProp->propName, "VERSION") == 0)
			return INV_CAL;
		
		//insertBack(&props, currProp);
		if(getLength(props) > 0 && findElement(props, equalProps, currProp) != NULL){
			return INV_CAL;
		}
		insertBack(&props, currProp);
	}

	itere = createIterator(obj->events);
	while ((currEvent = nextElement(&itere)) != NULL) {

		if( !currEvent->UID )
			return INV_EVENT;
		if( strlen(currEvent->UID) < 1 )
			return INV_EVENT;
		if( !currEvent->creationDateTime.date )
			return INV_EVENT;
		if( strlen(currEvent->creationDateTime.date) < 1 )
			return INV_EVENT;
		if( !currEvent->creationDateTime.time )
			return INV_EVENT;
		if( strlen(currEvent->creationDateTime.time) < 1 )
			return INV_EVENT;
			
		memset(&props, 0, sizeof(List));
		
		iterp = createIterator(currEvent->properties);
		while ((currProp = nextElement(&iterp)) != NULL) {
			if(strcmp(currProp->propName, "DTSTAMP") == 0)
				return INV_EVENT;
			if(strcmp(currProp->propName, "UID") == 0)
				return INV_EVENT;
			if(strcmp(currProp->propName, "DURATION") == 0)
				return INV_EVENT;
			if(getLength(props) > 0 && findElement(props, equalProps, currProp) != NULL)
				return INV_EVENT;
			insertBack(&props, currProp);
		}

		itera = createIterator(currEvent->alarms);
		while ((currAlarm = nextElement(&itera)) != NULL) {	
			
			if( !currAlarm->action )
				return INV_ALARM;
			if( strlen(currAlarm->action) < 1 )
				return INV_ALARM;
			if( currAlarm->trigger )
				return INV_ALARM;
			if( strlen(currAlarm->trigger) < 1 )
				return INV_ALARM;

			memset(&props, 0, sizeof(List));

			iterp = createIterator(currAlarm->properties);
			while ((currProp = nextElement(&iterp)) != NULL) {
				if(strncmp(currProp->propName, "ACTION", 6) == 0)
					return INV_ALARM;
				if(strncmp(currProp->propName, "TRIGGER", 7) == 0)
					return INV_ALARM;
				
				if(getLength(props) > 0 && findElement(props, equalProps, currProp) != NULL)
					return INV_ALARM;
				insertBack(&props, currProp);
			}
		}
	}
	return OK;
}

char* getProperty(List l, char* propName) {
	Node* currNode = l.head;
	while (currNode != NULL) {
		int retVal = strcmp(propName, ((Property*)currNode->data)->propName);
		if (retVal == 0) {
			return ((Property*)currNode->data)->propDescr;
		}
		currNode = currNode->next;
	}
	return NULL;
}

int createCalendarForUI(char* fileName, void** h) {
	Calendar* cal;
	int errCode = createCalendar(fileName, &cal);
	*h = (void*)cal;
	return errCode;
}

void getEventsListForUI(void* voidCalendar, UIEventInfo** uiarray, int* uilength) {
	Calendar* calendar = (Calendar*)voidCalendar;
	UIEventInfo* events = (UIEventInfo*) malloc(sizeof(UIEventInfo) * calendar->events.length);
	Node* curr = calendar->events.head;
	for (int i = 0; i < calendar->events.length; ++i) {
		Event* e = (Event*)curr->data;
		char* summary = getProperty(e->properties, "SUMMARY");
		if (summary == NULL) {
			summary = "None";
		}
		UIEventInfo ei = {e->properties.length, e->alarms.length, summary, (void*)e};
		events[i] = ei;
		curr = curr->next;
	}
	*uiarray = events;
	*uilength = calendar->events.length;
}

void getDBTablePropertiesForUI(void* voidCalendar, UIEventInfo** uiarray, int* uilength) {
	Calendar* calendar = (Calendar*)voidCalendar;
	UIDBInfo* events = (UIDBInfo*) malloc(sizeof(UIEventInfo) * calendar->events.length);
	Node* curr = calendar->events.head;
	for (int i =0; i < calendar->events.length; ++i) {
		Event* e = (Event*)curr->data;
		char* summary = getProperty(e->properties, "SUMMARY");
		char* location = getProperty(e->properties, "LOCATION");
		char* organizer = getProperty(e->properties, "ORGANIZER");
		if(summary == NULL) {
			summary = "None";
		}
		if(location == NULL) {
			location = "None";
		}
		if(organizer == NULL) {
			organizer = "None";
		}

		UIDBInfo ei = {organizer, summary, location, e->alarms.length, e->startDateTime, (void)e};
		events[i] = ei;
		curr = curr->next;
	}

}

char* printAlarmsForEventUI(void* event) {
	Event* e = (void*) event;
	return toString(e->alarms);
}

char* printPropertiesForEventUI(void* event) {
	Event* e = (void*) event;
	return toString(e->properties);
}

int validateCalendarForUI(void* c) {
	Calendar* calendar = (Calendar*) c;
	return validateCalendar(calendar);
}

void* createNewCalendarForUI(char* pid) {
	Calendar* calendar = (Calendar*) malloc(sizeof(Calendar));
	calendar->version = 2.0;
	memcpy(calendar->prodID, pid, strlen(pid));
	calendar->events = initializeList(printEvent, deleteEvent, compareEvents);
	calendar->properties = initializeList(printProperty, deleteProperty, compareProperties);
	return (void*) calendar;
}

void* createNewEventForUI(char* uid, DateTime creationDate, DateTime startDate) {
	Event* e = (Event*) malloc(sizeof(Event));
	memcpy(e->UID, uid, strlen(uid));
	e->creationDateTime = creationDate;
	e->startDateTime = startDate;
	e->properties = initializeList(printProperty, deleteProperty, compareProperties);
	e->alarms = initializeList(printAlarm, deleteAlarm, compareAlarms);
	return (void*) e;
}

void* addEventToCalendarForUI(void* calendar, void* event) {
	Calendar* c = (Calendar*) calendar;
	insertFront(&c->events, event);
	return (void*) c;
}

int writeCalendarForUI(char* fileName, void* calendar) {
	Calendar* c = (Calendar*) calendar;
	return writeCalendar(fileName, c);
}
