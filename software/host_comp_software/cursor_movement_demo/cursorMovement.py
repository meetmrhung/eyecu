from ctypes import*
import time
class POINT(Structure):
	_fields_ = [("x", c_ulong), ("y", c_ulong)]

#  Slides cursor to the specified coordinates
def slide(a,b,speed=0):
	while True:
		if speed == 'slow':
			time.sleep(0.005)
			Tspeed = 2
		if speed == 'fast':
			time.sleep(0.001)
			Tspeed = 5
		if speed == 0:
			time.sleep(0.001)
			Tspeed = 3
		
		
		mypt = POINT(0,0)
		windll.user32.GetCursorPos(byref(mypt))
		x = mypt.x
		y = mypt.y
		
		if abs(x-a) < 5:
			if abs(y-b) < 5:
				break
		   
		if a < x:
			x -= Tspeed
		if a > x:
			x += Tspeed
		if b < y:
			y -= Tspeed
		if b > y:
			y += Tspeed
		windll.user32.SetCursorPos(x,y)
		
#  Computes absolute screen coordinates from relative ones
#  and slides the cursor to the corresponding location		
def dp(x=0, y=0, speed=0):
	mypt = POINT(0,0)
	windll.user32.GetCursorPos(byref(mypt))
	print mypt.x, mypt.y
	upx = mypt.x+10*x
	upy = mypt.y+10*y
	if upx < 0:
		upx = 0
	if upy < 0:
		upy = 0
	print upx, upy
	slide(upx,upy,speed)