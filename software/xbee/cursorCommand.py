from ctypes import*
import time
import cursorMovement
import win32con
import win32api

# Input to the function is the generated cursor command which will vary from 0 to 6
def result(cursorcomm):
	print int(cursorcomm)
	#define mapping to direction of movement
	mapping = [[0,0],[1,0],[-1,0],[0,-1],[0,1]]
	if int(cursorcomm) == 5:
		# User is blinking
		win32api.mouse_event(win32con.MOUSEEVENTF_LEFTDOWN, 0, 0)
		win32api.mouse_event(win32con.MOUSEEVENTF_LEFTUP,0, 0)
	else:
		cursorMovement.dp(mapping[int(cursorcomm)][0], mapping[int(cursorcomm)][1])

	
