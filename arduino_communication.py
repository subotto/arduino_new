import socket
import traceback

TCP_IP = '192.168.6.250'
TCP_PORT = 2400
BUFFER_RCV_LENGTH = 2
PWD = "VerySecret"

EVENTS = ["VOID", "GOAL", "SUPERGOAL", "PLUS_ONE", "MINUS_ONE"]


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print "Connecting to socket..."
print "TCP_IP: " + str(TCP_IP)
print "TCP_PORT: " + str(TCP_PORT)
s.connect((TCP_IP, TCP_PORT))
print "Connected!"
s.send(PWD)

# Send an int to the Arduino
def sendNumber(num):
    print "Sending message..."
    s.send(chr(num))
    print "Sent!"

    
#
def dataFromBuff(rcv):
    rcv = map(ord, rcv)
    score = ((rcv[0] & 0xF)<<8) + rcv[1]
    team = "BLUE" if rcv[0] & 0x80 else "RED"
    return {
        "score": score,
        "team": team,
        "event": EVENTS[(rcv[0] & 0x70) >> 4]
    }

# Receive data from Arduino
def receiveData():
    print "Receiving message..."
    rcv = ""
    while len(rcv) < BUFFER_RCV_LENGTH:
        rcv += s.recv(BUFFER_RCV_LENGTH)
    return dataFromBuff(rcv)


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
    print "Sending request for data..."
    if team == "RED":
        sendNumber(0x4)
    else:
        sendNumber(0x6)
    data = receiveData()
    if data["event"] != "VOID":
        print "ERROR: non void event received"
    return data

    
# Interactive tester
def run():
    while True:
        rcv = ""
        try:
            rcv = receiveData()
            print "Message recieved: " + rcv
        except:
            traceback.print_exc()


sendMessage("RED",1)
print askData("RED")
print askData("BLUE")
s.close()
