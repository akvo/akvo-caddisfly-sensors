import serial
import time
import csv

f141=0
f235=0
f470=0
f1413=0
f3000=0
f12880=0
fTap=0

goTimes = 0


def writeID(idNumber):
    ser.open()
    if ser.isOpen():
    #    idNumber = raw_input("please enter the id: ")
        ser.write("set id " + idNumber + "\r\n")
        print("set id " + idNumber)

    ser.close()

def process():
    ser.open()
    ser.write("process" + "\r\n")
    time.sleep(0.5)
    print("process started")

    numOfLines = 0
    while True:
        response = ser.readline()
        print(response)
        numOfLines = numOfLines + 1
        if (numOfLines >= 4):
            break

    ser.close()

def go():
    global goTimes
    ser.open()
    ser.write("go" + "\r\n")
    time.sleep(0.5)
    #print("go")
    numOfLines = 0
    while True:
        response = ser.readline()
        if len(response)>0:
            print(response)
            numOfLines = numOfLines + 1
            if numOfLines==3:
                x = float(response)
                #for p in range(5):
                #    print x
                if goTimes==0:
                    global f141
                    f141=x
                elif goTimes==1:
                    global f235
                    f235=x
                elif goTimes==2:
                    global f470
                    f470=x
                elif goTimes==3:
                    global f1413
                    f1413=x
                elif goTimes==4:
                    global f3000
                    f3000=x
                elif goTimes==5:
                    global f12880
                    f12880=x
        if (goTimes==5):
            if (numOfLines >= 3):
                break
        if (numOfLines >= 8):
            break
    
    goTimes+=1
    ser.close()

def reading():
    ser.open()
    ser.write("READWITHOUTTEMP" + "\r\n")
    numOfLines = 0
    while True:
        response = ser.readline()
        if len(response)>0:
            print(response)   
            if numOfLines==0:
                x = float(response)
                global fTap
                fTap = x                
        numOfLines = numOfLines + 1
        if (numOfLines >= 1):
            break
    ser.close()



def itay():
    ser.open()
    ser.write("process" + "\r\n")
    time.sleep(0.5)
    while True:
        line = ser.readline()
        if line == "\"go\"":
            break
        else:
            print(line),
    ser.close()
                
comPort = '11'
comPort = raw_input("Hello, what is COM port? ")
ser = serial.Serial('COM'+comPort, 9600, timeout=None)
ser.close()
idNumber = raw_input("What is the SerialID? ")
writeID(idNumber)

time.sleep(1)
print "starting process"
process()

whileTimes = 0
while True:
    string = raw_input("press go to contunue: ")
    if string=="go" or string =="GO":
        if whileTimes<6:
            go()
        whileTimes+=1
    if string=="r" or string =="R":
        reading()
    if string == "q":
        break
    
        
with open ('test.csv','a') as fp:
    a = csv.writer(fp,delimiter = ',')
    data = [[idNumber,f141,f235,f470,f1413,f3000,f12880,fTap]]
    a.writerows(data)
ser.close()
