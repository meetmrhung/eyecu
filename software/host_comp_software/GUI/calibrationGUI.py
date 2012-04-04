from Tkinter import *
import tkMessageBox
import struct
import mmap
import os
import sys

class App:
	def __init__(self, master):
		#  Initialize Frame
		frame = Frame(master)
		frame.grid()
						
		#  Initialize Scales
		self.scaleThreshold = Scale(
			frame,
			label="Threshold",
			orient=HORIZONTAL,
			from_=1,
			to=255, 
			command=self.writeParams,
			length = 300,
			relief=FLAT,
			cursor='gumby'
			)
		self.scaleThreshold.set(45)
		self.scaleThreshold.grid(row=0)

		self.scaleiMin = Scale(
			frame,
			label="iMin",
			orient=HORIZONTAL,
			from_=0,
			to=480, 
			command=self.writeParams,
			length = 300,
			relief=FLAT,
			cursor='umbrella'
			)
		self.scaleiMin.set(170)
		self.scaleiMin.grid(row=1)
		
		self.scaleiMax = Scale(
			frame,
			label="iMax",
			orient=HORIZONTAL,
			from_=0,
			to=480, 
			command=self.writeParams,
			length = 300,
			relief=FLAT,
			cursor='coffee_mug'
			)
		self.scaleiMax.set(270)
		self.scaleiMax.grid(row=2)
		
		self.scalejMin = Scale(
			frame,
			label="jMin",
			orient=HORIZONTAL,
			from_=0,
			to=640, 
			command=self.writeParams,
			length = 300,
			relief=FLAT,
			cursor='boat'
			)
		self.scalejMin.set(270)
		self.scalejMin.grid(row=3)
		
		self.scalejMax = Scale(
			frame,
			label="jMax",
			orient=HORIZONTAL,
			from_=0,
			to=640, 
			command=self.writeParams,
			length = 300,
			relief=FLAT,
			cursor='pirate'
			)
		self.scalejMax.set(420)
		self.scalejMax.grid(row=4)


		#  Initialize Buttons
		self.button = Button(frame, text = "QUIT", command = frame.quit)
		self.button.grid(row=10, pady=10, sticky=S)

				
	#  Write threshold value from scale to file map
	def writeParams(self, val):
		map.seek(0)
		map.write(struct.pack("i", self.scaleThreshold.get()))
		map.write(struct.pack("i", self.scaleiMin.get()))
		map.write(struct.pack("i", self.scaleiMax.get()))
		map.write(struct.pack("i", self.scalejMin.get()))
		map.write(struct.pack("i", self.scalejMax.get()))
		
		
root = Tk()			#  Initialize root
root.wm_title('Gaze Tracker')			#  Set window title

#  Set window icon
scriptpath = os.path.dirname(sys.argv[0])
scriptpath += '\\eye.ico'
root.iconbitmap(default=scriptpath)
root.grid()

app = App(root)

map = mmap.mmap(0,256, tagname='Local\\GazeTrackerFileMapping')			#  Create file map
root.mainloop()
map.close()																#  Close file map