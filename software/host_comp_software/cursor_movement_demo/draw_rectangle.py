import struct
import time
import cursorMovement

#  Convert integers into direction vectors
mapping = [[0,0],[0,0],[0,0],[1,0],[-1,0],[0,-1],[0,1]]

#  Generate file which sends cursor codes 3-6
#  for 60 frames each
myfile = open('input.txt', 'wb')
print 'Generating file...'
for i in [3,5,4,6]:
	for j in range(60):
		myfile.write(struct.pack("i", i))
myfile.close()
print 'done.'

#  Read in the file and slide the cursor accordingly
print 'Reading file...'
myfile = open('input.txt', 'rb')
while 1:
	data=myfile.read(4)
	if not data:
		break
	myint=struct.unpack('i', data)[0]
	cursorMovement.dp(mapping[myint][0], mapping[myint][1])
	print mapping[myint]
	time.sleep(0.0333333)
print 'done'
myfile.close()