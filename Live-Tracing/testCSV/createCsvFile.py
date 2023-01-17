import os
from datetime import datetime, timezone, timedelta                                
from random import randint, shuffle, choice
import csv

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

startingTimeInFuture = 2
numberOfIterations = 10
secondsBetweenIterations = 30
numberOfThreads = 10

writtenBytes = [20,40,60,80,100]
functionnames = ["write", "fwrite", "writev"]
threads = []
startingThread = 1000
for i in range(numberOfThreads):
    thread = startingThread + i
    threads.append(thread)

local_time = datetime.now(timezone.utc).astimezone().replace(second=0, microsecond=0) + timedelta(minutes=startingTimeInFuture)
local_time.isoformat()

with open('csv_file.csv', 'w', newline='') as file:
    writer = csv.writer(file)
     
    ## head
    writer.writerow(["#datatype","string","long","dateTime:RFC3339","unsignedLong","string","string","string","string","string","string","string"])
    writer.writerow(["#group","false","false","false","false","true","true","true","true","true","true","true"])
    writer.writerow(["#default","","","","","","","","","","",""])
    writer.writerow(["","result","table","_time","_value","_field","_measurement","functionname","hostname","jobname","processid","thread"])

    ## body
    index = 0
    td = 0
    for i in range(numberOfIterations):
        shuffle(threads)
        for j in range(randint(4,numberOfThreads-2)):
            writer.writerow(["","",index,local_time + timedelta(seconds=td),writtenBytes[randint(0,4)],"function_data_written_bytes","libiotrace",functionnames[randint(0,2)],"host","test",threads[j],threads[j]])
            index += 1
        td += secondsBetweenIterations