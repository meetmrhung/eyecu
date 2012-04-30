#-------------------------------------------------------------------------------
# Name:        Duck Hunt
# Author:      Khashi Xiong
#-------------------------------------------------------------------------------

import pygame
from pygame.locals import*
from sys import exit
from random import randint

pygame.init()

# Define black and white
black = (0, 0, 0)
white = (255, 255, 255)

# Set screen size to 640x480 and go fullscreen
screen = pygame.display.set_mode((640, 480),pygame.FULLSCREEN)

# Window Title
pygame.display.set_caption("eyeCU Duck Hunt")

# Mouse cursor position variables
x_position = 0
y_position = 0

# Mouse cursor click variable
x_click = 0
y_click = 0

# Duck position variables
x_duck = 0
y_duck = randint(10, 350)

points = 0
speed = 2
missed = False
notmissed = True
restart = False

# Player starts
pygame.mixer.init(44100, -16, 2, 1024)

# Music volume
pygame.mixer.music.set_volume(0.8)

game_running = True

while game_running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            game_running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                game_running = False
            elif event.key == pygame.K_UP:
                restart = True
        # Used by the reticle
        elif event.type == MOUSEMOTION:
            x_position, y_position = pygame.mouse.get_pos()
        # This is used to register the mouse click
        elif event.type == MOUSEBUTTONDOWN:
            x_click, y_click = pygame.mouse.get_pos()
    # Starting position of the reticle
    position = (x_position - 50, y_position - 50)
    # Move the duck forward
    x_duck += 1
    # Defines the end position for the duck to end the game
    if x_duck * speed > 640 and not missed:
        x_duck = 0
        y_duck = randint(10, 350)
        # Play game over music if duck is missed
        pygame.mixer.music.load("gameover.mp3")
        pygame.mixer.music.play()
        missed = True
        notmissed = False
        

    # Make the background black
    screen.fill(black)
    pygame.mouse.set_visible(False)

    # Render the other screen images
    screen.blit(pygame.image.load("background.png"), (0, 0))
    screen.blit(pygame.font.SysFont("tahoma", 20).render("Points: " + str(points), True, white), (450, 10))

    # Threshold for duck to be in the reticle to be a hit
    if notmissed:
        if x_click in range(x_duck * speed - 20, x_duck * speed + 20) and y_click in range(y_duck - 30, y_duck + 30):
            # Play hit audio
            pygame.mixer.music.load("hit.mp3")
            pygame.mixer.music.play()
            # Increase point by 1 if duck is hit
            points += 1
            # New duck position
            x_duck = 0
            y_duck = randint(10, 350)
        # Draw the new duck
        screen.blit(pygame.image.load("duck.gif"), (x_duck * speed, y_duck))

    if missed:
        # If the duck is missed, then load the dog image
        x_duck = -50
        y_duck = -50
        screen.blit(pygame.image.load("dog.gif"), (320, 300))
        # Tell player to restart or quit program
        screen.blit(pygame.font.SysFont("tahoma", 20).render("Press UP to Restart or ESC to Quit", True, white), (175, 175))
        # Initiate restart if up is pressed
        if restart:
            missed = False
            notmissed = True
            points = 0
            x_duck = 0
            y_duck = randint(10, 350)
            restart = False
 
    screen.blit(pygame.image.load("reticle.gif").convert(), position)
    pygame.display.update()

pygame.quit()
