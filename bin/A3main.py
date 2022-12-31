from tkinter import *
import tkinter.filedialog as tkFileDialog
import tkinter.messagebox as tkMessageBox
from CalendarWrapper import *
import os
import time

class createCalendarDialog():
    def __init__(self, root):
        self.frame = Frame(root, bg="white", highlightbackground="black", highlightthickness="2")
        self.frame.grid(row=0, column=0)

        f1 = Frame(self.frame)
        f1.pack()
        Label(f1, text="Product ID").pack(side=LEFT)
        self.entry = Entry(f1)
        self.entry.pack(side=LEFT)

        f2 = Frame(self.frame)
        f2.pack()

        buttonConfirm = Button(f2, text="Confirm", command=self.create)
        buttonConfirm.pack(side=LEFT)
        buttonCancel = Button(f2, text="Cancel", command=lambda: self.frame.destroy())
        buttonCancel.pack(side=LEFT)

        self.frame.grab_set()

    def create(self):
        global loadedCalendar
        global loadedFile
        pid = self.entry.get()
        printInLogPanel("Remember to add an event to your calendar!!")

        if pid == "":
            printInLogPanel("Enter a product id...")
        else:
            calendar = createNewCalendarWrapper(pid)
            loadCalendar(None, calendar)
            self.frame.destroy()

class createEventDialog():
    def __init__(self, root):
        self.frame = Frame(root, bg="white", highlightbackground="black", highlightthickness="2")
        self.frame.grid(row=0, column=0)

        f1 = Frame(self.frame)
        f1.pack()
        Label(f1, text="User ID").pack(side=LEFT)
        self.userIdEntry = Entry(f1)
        self.userIdEntry.pack(side=LEFT)

        f3 = Frame(self.frame)
        f3.pack()
        Label(f3, text="Date").pack(side=LEFT)
        self.startDateEntry = Entry(f3)
        self.startDateEntry.pack(side=LEFT)

        f4 = Frame(self.frame)
        f4.pack()
        Label(f4, text="Time").pack(side=LEFT)
        self.startTimeEntry = Entry(f4)
        self.startTimeEntry.pack(side=LEFT)

        f5 = Frame(self.frame)
        f5.pack()
        Label(f5, text="UTC").pack(side=LEFT)
        self.utcEntry = Entry(f5)
        self.utcEntry.pack(side=LEFT)

        f2 = Frame(self.frame)
        f2.pack()

        buttonConfirm = Button(f2, text="Confirm", command=self.create)
        buttonConfirm.pack(side=LEFT)
        buttonCancel = Button(f2, text="Cancel", command=lambda: self.frame.destroy())
        buttonCancel.pack(side=LEFT)

        self.frame.grab_set()

    def create(self):
        global loadedCalendar
        global loadedFile
        uid = self.userIdEntry.get()
        date = self.startDateEntry.get()
        ltime = self.startTimeEntry.get()
        utc = self.utcEntry.get()

        if uid == "":
            printInLogPanel("Enter a user id...")
        elif date == "":
            printInLogPanel("Enter a valid date...")
        elif ltime == "":
            printInLogPanel("Enter a valid time...")
        elif utc == "":
            printInLogPanel("Enter if is UTC...")
        else:
            creationDate = DateTime()
            creationDate.date = time.strftime("%Y%m%d").encode()
            creationDate.time = time.strftime("%H%M%S").encode()
            creationDate.UTC = False
            startDate = DateTime()
            startDate.date = date.encode()
            startDate.time = ltime.encode()
            startDate.UTC = True if utc == "true" else False
            event = createNewEventWrapper(uid, creationDate, startDate)
            addEventToCalendar(loadedCalendar, event)
            printInLogPanel("Event Created Successfully!")
            loadCalendar(loadedFile, loadedCalendar)
            self.frame.destroy()
            
loadedCalendar = None
loadedFile = None
def loadCalendar(fileName, calendar):
    global loadedCalendar
    global loadedFile
    global createMenu
    global root

    if fileName == None:
        title = "Untitled"
        loadedFile = None
    else:
        title = os.path.basename(fileName)
        loadedFile = fileName

    loadedCalendar = calendar
    populateFileViewPanel(getEventsList(calendar))

    createMenu.entryconfig("Create event", state=NORMAL)
    printInLogPanel(title + " - File Loaded Successfully!")
    root.title("iCalGUI - " + title)
    return

def openFile():
    fileName = tkFileDialog.askopenfilename(initialdir = "./wafaA2/testFiles",title = "Select file")
    if len(fileName) == 0:
        log.insert(INSERT, "No file selected\n")
        return
    if fileName != "":
        retVal = createCalendar(fileName)
        if retVal[0] == 0:
            loadCalendar(fileName, retVal[1])
        else:
            printFileErrorInLogPanel(retVal[0], os.path.basename(fileName))
    return

def onFrameConfigure(canvas):
    canvas.configure(scrollregion=canvas.bbox("all"))
    return

def populateFileViewPanel(events):
    global fileViewPanel
    global fileViewPanelEvents
    global selectedEventWidget

    selectedEventWidget = None
    selectedEvent = None
    fileViewPanelEvents.destroy()
    fileViewPanelEvents = Frame(fileViewPanel)
    fileViewPanelEvents.pack(fill=X, expand=True, anchor=NE)

    sizing = (7, 7, 7)

    headings = Frame(fileViewPanelEvents)
    headings.pack(fill=X, expand=True)
    Label(headings, text="Event No", width=sizing[0]).pack(side=LEFT)
    Label(headings, text="Props", width=sizing[1]).pack(side=LEFT)
    Label(headings, text="Alarms", width=sizing[2]).pack(side=LEFT)
    Label(headings, text="Summary").pack(side=LEFT, padx=30)

    canvas = Canvas(fileViewPanelEvents)
    eventsContainer = Frame(canvas)
    vsb = Scrollbar(fileViewPanelEvents, orient="vertical", command=canvas.yview)
    canvas.configure(yscrollcommand=vsb.set)

    vsb.pack(side="right", fill="y")
    canvas.pack(side="left", fill="both", expand=True)
    canvas.create_window((0,0), window=eventsContainer, anchor="nw")

    eventsContainer.bind("<Configure>", lambda event, canvas=canvas: onFrameConfigure(canvas))

    if events != None:
        for event in events:
            createEventWidget(eventsContainer, event, sizing, True)
    if 8 - len(events) > 0:
        for i in range(8 - len(events)):
            createEventWidget(eventsContainer, ("-", "-", "-", "-", None), sizing, False)

    return

def createEventWidget(parent, event, sizing, selectable):
    if event == None:
        eventWidget = Frame(parent, height=40, bg="red").pack(fill=X, expand=True)
    else:
        eventWidget = Frame(parent, height=40)
        eventWidget.pack(fill=X, expand=True)
        l1 = Label(eventWidget, text=str(event[0]), width=sizing[0])
        l1.pack(side=LEFT)
        l2 = Label(eventWidget, text=str(event[1]), width=sizing[1])
        l2.pack(side=LEFT)
        l3 = Label(eventWidget, text=str(event[2]), width=sizing[2])
        l3.pack(side=LEFT)
        l4 = Label(eventWidget, text=str(event[3]))
        l4.pack(side=LEFT, padx=30)
        if selectable:
            l1.bind("<Button-1>", lambda e: selectEventWidget(eventWidget, event[4]))
            l2.bind("<Button-1>", lambda e: selectEventWidget(eventWidget, event[4]))
            l3.bind("<Button-1>", lambda e: selectEventWidget(eventWidget, event[4]))
            l4.bind("<Button-1>", lambda e: selectEventWidget(eventWidget, event[4]))
            eventWidget.bind("<Button-1>", lambda e: selectEventWidget(eventWidget, event[4]))
    return

selectedEventWidget = None
selectedEvent = None
def selectEventWidget(eventWidget, event):
    global selectedEventWidget
    global selectedEvent
    if selectedEventWidget != None:
        selectedEventWidget.config(highlightthickness=0)
    if selectedEventWidget != eventWidget:
        eventWidget.config(highlightthickness=2, highlightbackground="blue")
        selectedEventWidget = eventWidget
        selectedEvent = event
        alarmsButton.config(state=NORMAL)
        propsButton.config(state=NORMAL)
    else:
        selectedEventWidget = None
        selectedEvent = None
        alarmsButton.config(state=DISABLED)
        propsButton.config(state=DISABLED)
    return

firstPrint = True
def printInLogPanel(message):
    global log
    global firstPrint
    log.config(state=NORMAL)
    if firstPrint:
        log.insert('1.0', message + "\n")
        firstPrint = False
    else:
        log.insert('1.0', message + "\n")
    log.see(1.0)
    #log.config(state=DISABLED)
    return

def printFileErrorInLogPanel(errCode, fileName):
	
        if errCode == 1:
            printInLogPanel(fileName + " - Error: Invalid File must be .ics.")
        elif errCode == 2:
            printInLogPanel(fileName + " - Error: Invalid Calendar!")
        elif errCode == 3:
            printInLogPanel(fileName + " - Error: Invalid Verion!")
        elif errCode == 4:
            printInLogPanel(fileName + " - Error: File contains Duplicate Versions.")
        elif errCode == 5:
            printInLogPanel(fileName + " - Error: Invalid Product ID!")
        elif errCode == 6:
            printInLogPanel(fileName + " - Error: File contains Duplicate Product ID.")
        elif errCode == 7:
            printInLogPanel(fileName + " - Error: Invalid Event!")
        elif errCode == 8:
            printInLogPanel(fileName + " - Error: Invalid Date-Time!")	
        elif errCode == 9:
            printInLogPanel(fileName + " - Error: Invalid Alarm!")
        elif errCode == 11:
            printInLogPanel(fileName + " - Error: Other Error.")
        return

def showAlarms():
    global selectedEvent
    if selectedEvent == None:
        printInLogPanel("Select an event first.")
    else:
        printInLogPanel(printAlarmsForEvent(selectedEvent))
    return

def showProps():
    global selectedEvent
    if selectedEvent == None:
        printInLogPanel("Select an event first.")
    else:
        printInLogPanel(printPropertiesForEvent(selectedEvent))
    return

def exitAll(event=None):
	userResp = tkMessageBox.askyesno("Exit?", "Are you sure you would like to quit?")
	if userResp:
		root.quit()

def createNewCalendar():
    global root
    c = createCalendarDialog(root)
    return

def createEvent():
    global root
    c = createEventDialog(root)
    return

def saveFile():
    global loadedFile
    global loadedCalendar
    if loadedCalendar == None:
        tkMessageBox.showerror("Error", "Please create or open a calendar first!")
        return
    if loadedFile == None:
        outputFilename = tkFileDialog.asksaveasfile(title="Save file as:")
        record = writeCalendar(outputFilename, loadedCalendar)    
    else:
        retVal = writeCalendar(loadedFile, loadedCalendar)
        if retVal != 0:
            printFileErrorInLogPanel(retVal, loadedFile)
    return

def saveAsFile():
    global loadedFile
    global loadedCalendar
    if loadedCalendar == None:
        tkMessageBox.showerror("Error", "Please create or open a calendar first!")
        return
    fileName = tkFileDialog.asksaveasfilename(title = "Save as..")
    retVal = writeCalendar(fileName, loadedCalendar)
    log.insert('1.0', "File saved successfully! \n")
    if retVal != 0:
        printFileErrorInLogPanel(retVal, loadedFile)
    else:
	    loadedFile = fileName
	    loadCalendar(loadedFile, loadedCalendar)
    return
    
def Clear():
	log.delete(1.0, 'end')
	return
	
def showHelpMenu():
	userResp = tkMessageBox.showinfo(
		"About ICalendar",
		"ICalendar\nVersion 2.0\nCreated by Wafa Qazi 2017\n\niCalendar is a computer file format which allows Internet users to send meeting requests and tasks to other Internet users by sharing or sending files in this format through various methods.")

root = Tk()
root.title("ICalGUI")
root.protocol('WM_DELETE_WINDOW', exitAll)

fileViewPanel = Frame(root)
fileViewPanel.grid(row=0, column=0, sticky=(N, W, E))
fileViewPanelEvents = Frame(fileViewPanel)
fileViewPanelEvents.pack()

buttonsContainer = Frame(root)
buttonsContainer.grid(row=1, column=0, sticky=(W))
alarmsButton = Button(buttonsContainer, text="Show alarms", command=showAlarms)
alarmsButton.pack(side=LEFT)
propsButton = Button(buttonsContainer, text="Extract optional props", command=showProps)
propsButton.pack(side=LEFT)
alarmsButton.config(state=DISABLED)
propsButton.config(state=DISABLED)

logPanel = Frame(root, bg="black")
logPanel.grid(row=2, column=0, sticky=(N, W, E, S))

clrButtonPanel = Frame(root, bg="light grey")
clrButtonPanel.grid(row=3,column=0, sticky=(N,E,S,W))
clearButton = Button(clrButtonPanel, text="Clear", command=Clear)
clearButton.pack(side=LEFT)

status = Label(root, text="ICalGUI V2.0", bd=1, relief=SUNKEN, anchor=W)
status.grid(row=4, column=0, sticky=(N, W, E, S))

root.columnconfigure(0, weight=1)
root.rowconfigure(0, weight=0)
root.rowconfigure(1, weight=0)
root.rowconfigure(2, weight=1)
root.rowconfigure(3, weight=2)
root.rowconfigure(4, weight=0)

scroll = Scrollbar(logPanel)
scroll.pack(side=RIGHT, fill="both", pady=0, padx=0)

log = Text(logPanel, height=8, wrap=WORD, yscrollcommand=scroll.set)
#log.config(state=DISABLED)
log.pack(side=LEFT, fill="both", padx=2, pady=2, expand=True)
scroll.config(command=log.yview)

tmenu = Menu(root)
root.config(menu=tmenu)

fileMenu = Menu(tmenu)
tmenu.add_cascade(label="File", menu=fileMenu)
fileMenu.add_command(label="Open", underline = 0, command = openFile, accelerator="Ctrl+O")
fileMenu.add_command(label="Save", underline = 0, command = saveFile, accelerator="Ctrl+S")
fileMenu.add_command(label="Save as...", underline = 0, command = saveAsFile)
fileMenu.add_command(label="Exit", underline = 0, command = exitAll, accelerator="Ctrl+X")

createMenu = Menu(tmenu)
tmenu.add_cascade(label="Create", menu=createMenu)
createMenu.add_command(label="Create calendar", underline = 0, command = createNewCalendar)
createMenu.add_command(label="Create event", underline = 0, command = createEvent)
createMenu.entryconfig("Create event", state=DISABLED)

helpMenu = Menu(tmenu)
tmenu.add_cascade(label="Help", menu=helpMenu)
helpMenu.add_command(label="About ICalGUI...", command=showHelpMenu)

root.bind_all("<Control-o>", lambda e: openFile())
root.bind_all("<Control-s>", lambda e: saveFile())
root.bind_all("<Control-x>", lambda e: exitAll())

populateFileViewPanel([])

root.update()
root.minsize(root.winfo_width(), root.winfo_height())
root.mainloop()
