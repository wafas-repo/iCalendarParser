from ctypes import *

calendarAPI = CDLL("./bin/main.so")

class DateTime(Structure):
    _fields_ = [
        ("date", c_char * 9),
        ("time", c_char * 7),
        ("UTC", c_bool)
    ]

class UIEventInfo(Structure):
    _fields_ = [
        ("numProps", c_int),
        ("numAlarms", c_int),
        ("summary", c_char_p),
        ("event", c_void_p)
    ]

class UIDBInfo(Structure):
    _fields_=[
        ("organizer", c_char_p),
        ("summary", c_char_p),
        ("location", c_char_p),
        ("numAlarms", c_int),
        ("startTime"), DateTime,
        ("event", c_void_p)
    ]

calendarAPI.createCalendarForUI.restype = c_int
calendarAPI.createCalendarForUI.argtypes = [c_char_p, POINTER(c_void_p)]

calendarAPI.getEventsListForUI.restype = POINTER(UIEventInfo)
calendarAPI.getEventsListForUI.argtypes = [c_void_p]

calendarAPI.getDBTablePropertiesForUI.restype = POINTER(UIDBInfo)
calendarAPI.getDBTablePropertiesForUI.argtypes = [c_void_p]

calendarAPI.printAlarmsForEventUI.restype = c_char_p
calendarAPI.printAlarmsForEventUI.argtypes = [c_void_p]

calendarAPI.printPropertiesForEventUI.restype = c_char_p
calendarAPI.printPropertiesForEventUI.argtypes = [c_void_p]

calendarAPI.validateCalendarForUI.restype = c_int
calendarAPI.validateCalendarForUI.argtypes = [c_void_p]

calendarAPI.createNewCalendarForUI.restype = c_void_p
calendarAPI.createNewCalendarForUI.argtypes = [c_char_p]

calendarAPI.createNewEventForUI.restype = c_void_p
calendarAPI.createNewEventForUI.argtypes = [c_char_p, DateTime, DateTime]

calendarAPI.addEventToCalendarForUI.restype = c_void_p
calendarAPI.addEventToCalendarForUI.argtypes = [c_void_p, c_void_p]

calendarAPI.writeCalendarForUI.restype = c_int
calendarAPI.writeCalendarForUI.argtypes = [c_char_p, c_void_p]

def createCalendar(fileName):
    icalendar = c_void_p()
    errCode = calendarAPI.createCalendarForUI(fileName.encode(), pointer(icalendar))
    return (errCode, icalendar)

def getEventsList(calendar):
    length = c_int()
    events = POINTER(UIEventInfo)()
    calendarAPI.getEventsListForUI(calendar, pointer(events), pointer(length))
    pyEvents = []
    i = 0
    while i < length.value:
        pyEvents.append((i + 1, events[i].numProps, events[i].numAlarms, events[i].summary.decode(), events[i].event))
        i += 1
    return pyEvents

def getDBTablePropertiesForUI(calendar):
    length = c_int()
    events = POINTER(UIDBInfo)()
    calendarAPI.getDBTablePropertiesForUI(calendar, pointer(events), pointer(length))
    pyEvents = []
    i=0
    while i < length.value:
        pyEvents.append((i+1, events[i].organizer.decode(), events[i].summary.decode(), events[i].location.decode(), events[i].numAlarms, events[i].startTime, events[i].event))
        i += 1
    return pyEvents

def printAlarmsForEvent(event):
    return calendarAPI.printAlarmsForEventUI(event).decode("utf-8", "ignore")

def printPropertiesForEvent(event):
    return calendarAPI.printPropertiesForEventUI(event).decode("utf-8", "ignore") 

def validateCalendar(calendar):
    return calendarAPI.validateCalendarForUI(calendar)

def createNewCalendarWrapper(pid):
    return calendarAPI.createNewCalendarForUI(pid.encode())

def createNewEventWrapper(uid, creationDate, startDate):
    return calendarAPI.createNewEventForUI(uid.encode(), creationDate, startDate)

def addEventToCalendar(calendar, event):
    return calendarAPI.addEventToCalendarForUI(calendar, event)

def writeCalendar(fileName, calendar):
    return calendarAPI.writeCalendarForUI(fileName.encode(), calendar)
