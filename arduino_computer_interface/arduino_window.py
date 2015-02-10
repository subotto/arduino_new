import socket
import traceback
import sys
import select
from gi.repository import Gtk
import threading
import time

# ARDUINO STANDARD
#TCP_IP = '192.168.6.250'

# TEST
# TCP_IP = '192.168.6.156'

# TCP_PORT = 2400
BUFFER_RCV_LENGTH = 2
# PWD = "VerySecret"

TIMEOUT = 0.001

EVENTS = ["VOID", "GOAL", "SUPERGOAL", "PLUS_ONE", "MINUS_ONE"]
TEAMS = ["RED", "BLUE"]

isConnected = False

# Debug
debugConsole = Gtk.ListStore(str)
def debugLog(message):
    debugConsole.append([message])
    print message


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# debugLog("Connecting to socket...")
# debugLog("TCP_IP: " + str(TCP_IP))
# debugLog("TCP_PORT: " + str(TCP_PORT))
# s.connect((TCP_IP, TCP_PORT))
# debugLog("Connected!")
# s.send(PWD)





# Send an int to the Arduino
def sendNumber(num):
    s.send(chr(num))
    
# Elaborate the data received from arduino
def dataFromBuff(rcv):
    rcv = map(ord, rcv)
    score = ((rcv[0] & 0xF)<<8) + rcv[1]
    team = "BLUE" if rcv[0] & 0x80 else "RED"
    return {
        "score": score,
        "team": team,
        "event": EVENTS[(rcv[0] & 0x70) >> 4]
    }

# Receive data from Arduino; return false as the second element if the socket is closed
def receiveData():
    try:
        rlist, _, _ = select.select([s], [], [], TIMEOUT)
    except:
        debugLog ("Socket closed.")
        return (None,False)
    if rlist:
        rcv = s.recv(BUFFER_RCV_LENGTH)
        while len(rcv) < BUFFER_RCV_LENGTH:
            rcv += s.recv(BUFFER_RCV_LENGTH)
        return (dataFromBuff(rcv),True)
    else:
        return (None,True)

# Send a score change message to the Arduino
def sendMessage ( team, score_change ):
    if team == "RED":
        msg = 0
    else:
        msg = 2
    if score_change > 0:
        msg += 1
    else:
        score_change = - score_change
    for i in range(score_change):
        sendNumber(msg)
        
# Asks Arduino the score
def askData(team):
    # print "Sending request for data..."
    if team == "RED":
        sendNumber(0x4)
    else:
        sendNumber(0x6)
    data = receiveData()
    if data["event"] != "VOID":
        debugLog("ERROR: non void event received")
    return data





### INTERFACE ###
builder = Gtk.Builder()
builder.add_from_file("arduino_window.glade")


class ScoreManager:    
    score = {"RED":0, "BLUE":0}
    lastToScore = ""

    TCP_IP = ''
    TCP_PORT = 0
    PWD = ""

    # object references
    mainWindow = builder.get_object("mainWindow")
    mainWindow.show_all()
    connectionWindow = builder.get_object("connectionWindow")
    connectionWindow.show_all()

    scoreTextView = {"RED":builder.get_object("redScore"),
                     "BLUE":builder.get_object("blueScore")}
    lastToScoreBar = {"RED":builder.get_object("redLastToScore"),
                      "BLUE":builder.get_object("blueLastToScore")}
    consoleView = builder.get_object("console")
    consoleView.set_model(debugConsole)
    consoleView.insert_column(Gtk.TreeViewColumn("Debug Log", Gtk.CellRendererText(), text=0),0)

    
    # connection management
    def onConnection(self,*args):
        global isConnected

        self.TCP_IP = builder.get_object("tcpipText").get_text()
        self.TCP_PORT = int(builder.get_object("tcpportText").get_text())
        self.PWD = builder.get_object("pwdText").get_text()
        debugLog("Connecting to socket...")
        debugLog("TCP_IP: " + self.TCP_IP)
        debugLog("TCP_PORT: " + str(self.TCP_PORT))
        s.connect((self.TCP_IP, self.TCP_PORT))
        debugLog("Connected!")
        s.send(self.PWD)
        isConnected = True
        self.connectionWindow.close()
    

    # console management
    def onSizeAllocate(self,*args):
        adj = self.consoleView.get_vadjustment()
        adj.set_value( adj.get_upper() - adj.get_page_size())


    # score management
    def updateScore(self,team,score):
        score_change = score - self.score[team]
        debugLog(team + "  " + str(score_change))
        self.score[team] = score
        self.printScore(team)
        if score_change > 0:
            self.setLastToScore(team)

    def changeScore(self,team,score_change):
        sendMessage(team,score_change)
        self.updateScore(team,self.score[team] + score_change)

    # events
    def onDestroyWindow(self, *args):
        s.close()
        Gtk.main_quit(*args)

    def printScore(self,team):
        self.scoreTextView[team].set_text(str(self.score[team]))

    def setLastToScore(self,team):
        self.lastToScore = team
        self.lastToScoreBar[team].set_fraction(1)
        for other in [t for t in TEAMS if t != team]:
            self.lastToScoreBar[other].set_fraction(0)


    def onAddRed(self,*args):
        self.changeScore("RED",1)
    def onSubRed(self,*args):
        self.changeScore("RED",-1)
    def onAddBlue(self,*args):
        self.changeScore("BLUE",1)
    def onSubBlue(self,*args):
        self.changeScore("BLUE",-1)

scoreManager = ScoreManager()




class ArduinoListener(threading.Thread):
    def run(self):
        global isConnected

        while not isConnected:
            time.sleep(1)
            debugLog("Not listening...")
        debugLog("Listening!")
        while True:
            (rcv,isSocketOpen) = receiveData()
            if not isSocketOpen:
                quit()
            if rcv != None:
                debugLog(str(rcv))
                self.updateScore(rcv)

    def updateScore(self,rcv):
        scoreManager.updateScore(rcv["team"],rcv["score"])

arduinoListener = ArduinoListener()




# Run
builder.connect_signals(ScoreManager())


arduinoListener.start()
Gtk.main()




