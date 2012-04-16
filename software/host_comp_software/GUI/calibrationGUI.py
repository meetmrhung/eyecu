from Tkinter import *
import tkMessageBox
import struct
import mmap
import os
import sys


class App:
	#  Status bar text
	step1Text = "Adjust the threshold until a red circle covers the pupil. Then adjust the boundaries to enclose only the pupil. Once this is done, click next!"
	step2Text = "Adjust the boundaries to include the maximum range of motion of your eye."
	step3Text = "For this part of the calibration, center yourself in front of the monitor. Then look at each black circle without moving your head and press space. Press space to start."
	widgetWidth = 400
	
	def __init__(self, master):
		#  Initialize Frame
		frame = Frame(master)
		frame.grid()
		self.w = Canvas(frame, width = self.widgetWidth, height = 90)
		self.w.grid(row=10, columnspan=3)
		self.statusLabel = Label(
			frame, 
			text = self.step1Text,
			font = ("Helvectica", "12"),
			wraplength = self.widgetWidth
			)
		self.statusLabel.grid(row = 10, sticky = N, columnspan=3, pady = 10)
		#  Initialize Scales
		self.scaleThreshold = Scale(
			frame,
			label="Threshold",
			orient=HORIZONTAL,
			from_=1,
			to=255, 
			command=self.writeParams,
			length = self.widgetWidth,
			relief=FLAT,
			cursor='gumby'
			)
		self.scaleThreshold.set(45)
		self.scaleThreshold.grid(row=0, columnspan=3)

		self.scaleiMin = Scale(
			frame,
			label="iMin",
			orient=HORIZONTAL,
			from_=0,
			to=480, 
			command=self.writeParams,
			length = self.widgetWidth,
			relief=FLAT,
			cursor='umbrella'
			)
		self.scaleiMin.set(170)
		self.scaleiMin.grid(row=1, columnspan=3)
		
		self.scaleiMax = Scale(
			frame,
			label="iMax",
			orient=HORIZONTAL,
			from_=0,
			to=480, 
			command=self.writeParams,
			length = self.widgetWidth,
			relief=FLAT,
			cursor='coffee_mug'
			)
		self.scaleiMax.set(270)
		self.scaleiMax.grid(row=2, columnspan=3)
		
		self.scalejMin = Scale(
			frame,
			label="jMin",
			orient=HORIZONTAL,
			from_=0,
			to=640, 
			command=self.writeParams,
			length = self.widgetWidth,
			relief=FLAT,
			cursor='boat'
			)
		self.scalejMin.set(270)
		self.scalejMin.grid(row=3, columnspan=3)
		
		self.scalejMax = Scale(
			frame,
			label="jMax",
			orient=HORIZONTAL,
			from_=0,
			to=640, 
			command=self.writeParams,
			length = self.widgetWidth,
			relief=FLAT,
			cursor='pirate'
			)
		self.scalejMax.set(420)
		self.scalejMax.grid(row=4, columnspan=3)
		
		#  Initialize Buttons
		self.button = Button(frame, text = "QUIT", command = frame.quit)
		self.button.grid(row=6,pady=2,column=0)
		
		self.button = Button(frame, text = "RESTART", command = self.restartCalibration0)
		self.button.grid(row=6,pady=2,column=1)
		
		self.nextbutton = Button(frame, text = "NEXT", command = self.startFirstStage)
		self.nextbutton.grid(row=6, pady=2, column=2)	
		

	#  Write threshold value from scale to file map
	def writeParams(self, val):
		map.seek(0)
		map.write(struct.pack("i", self.scaleThreshold.get()))
		map.write(struct.pack("i", self.scaleiMin.get()))
		map.write(struct.pack("i", self.scaleiMax.get()))
		map.write(struct.pack("i", self.scalejMin.get()))
		map.write(struct.pack("i", self.scalejMax.get()))
	
	#  Transition from stage zero to stage one
	def startFirstStage(self):
		self.stage = 1
		self.statusLabel['text'] = self.step2Text
		self.nextbutton['command'] = self.startSecondStage
		self.scaleThreshold['state'] = DISABLED
	
	#  Transition from stage one to stage two
	def startSecondStage(self):
		#  Create new window
		self.top = Toplevel()						#  toplevel = window
		self.top.overrideredirect(1)				#  Override Redirect Flag -- Use for fullscreen
		self.top.state("zoomed")					#  Fullscreen
		self.stage = 2;								#  					
		#  Get screen dimensions
		width = self.top.winfo_screenwidth()
		height  = self.top.winfo_screenheight()
		
		self.top.focus_set()						#  Set focus to new window
		
		#  Create large blue canvas
		self.w = Canvas(self.top,width=width, height = height, background="LightSkyBlue4", highlightbackground="LightSkyBlue4")
		self.w.grid(row=0,column=0)
		
		#  Create buttons
		self.top.button = Button(self.top, text = "PREVIOUS", command = self.restartDirectionalCalibration, bd=3, font=("Helvectica", "12", "bold"))
		self.top.button.grid(row=0,sticky = SE, padx=120, pady=20)
		self.top.button = Button(self.top, text = "RESTART", command = self.restartCalibration, bd=3, font=("Helvectica", "12", "bold"))
		self.top.button.grid(row=0,sticky = SE, padx=20, pady=20)
		
		
		#  Show directions
		self.directionText = Label(
			self.top, 
			width = 0,
			text=self.step3Text,
			font=("Helvectica", "24", "bold"),
			wraplength = 600,
			background="LightSkyBlue4"
			)
		self.directionText.grid(row=0, column=0)
		
		#  Draw circle
		self.t = Canvas(self.top,width=50,height=50, background="LightSkyBlue4", bd=0, highlightbackground="LightSkyBlue4")
		self.t.create_oval(2,2,50,50,fill = "black")
		self.top.bind("<KeyRelease-space>", self.LookMiddle)
		
	#  Transition to next step of calibration
	def LookMiddle(self,event):		
		self.t.grid(row=0,column=0)
		self.directionText['text'] = " "
		self.top.bind("<KeyRelease-space>", self.LookLeft)	
		self.calibDirection = 0;
		
	def LookLeft(self,event):
		self.t.grid(row=0,column=0,sticky = W)
		self.top.bind("<KeyRelease-space>", self.LookRight)	
		self.calibDirection = 1;
		
	def LookRight(self,event):
		self.t.grid(row=0,column=0,sticky = E)
		self.top.bind("<KeyRelease-space>", self.LookUp)	
		self.calibDirection = 2;
		
	def LookUp(self,event):
		self.t.grid(row=0,column=0,sticky = N)
		self.top.bind("<KeyRelease-space>", self.LookDown)	
		self.calibDirection = 3;
		
	def LookDown(self,event):
		self.t.grid(row=0,column=0,sticky = S)
		self.top.bind("<KeyRelease-space>", self.killWindow)
		self.statusLabel['text'] = "Calibration complete!"
		self.calibDirection = 4;
		
	#  Used for 'PREVIOUS' button event
	#  Restarts directional calibration
	def restartDirectionalCalibration(self):
		self.top.bind("<KeyRelease-space>", self.LookMiddle)
		self.t.grid_forget()
		self.directionText['text'] = self.step3Text
		self.directionText.grid()
		
	#  Restart for use from main window 
	def restartCalibration0(self):
		self.stage = 0
		self.statusLabel['text'] = self.step1Text
		self.nextbutton['command'] = self.startFirstStage
		self.scaleThreshold['state'] = ACTIVE
	
	#  Restart for use from direction calibration window
	def restartCalibration(self):
		self.top.destroy()
		self.stage = 0
		self.statusLabel['text'] = self.step1Text
		self.nextbutton['command'] = self.startFirstStage
		self.scaleThreshold['state'] = ACTIVE
		
	#  Close direction calibration window
	def killWindow(self,event):
		self.top.destroy()
		
root = Tk()			#  Initialize root
root.wm_title('eye CU - Gaze Tracker')			#  Set window title

#  Set window icon
scriptpath = os.path.dirname(sys.argv[0])
scriptpath += '\\eye.ico'
if os.path.exists(scriptpath):
	root.iconbitmap(default=scriptpath)
root.grid()

app = App(root)

map = mmap.mmap(0,256, tagname='Local\\GazeTrackerFileMapping')			#  Create file map
root.mainloop()

map.close()																#  Close file map