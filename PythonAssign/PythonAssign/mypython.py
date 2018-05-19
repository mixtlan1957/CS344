#Program Filename: mypython.py
#Author: Mario Franco-Munoz
#Due Date: 5/20/2018
#Description: creates files with 10 character long random strings and generates two random numbers 
#and then displays product

import sys
import os
import random
import string


#creates three random files. Each file contain 10 random characters from lowercase alphabet with no spaces
def createFiles():
    
    #source: https://stackoverflow.com/questions/2823316/generate-a-random-letter-in-python

    #create three different files
    for i in range (0, 3):
        baseFileName = ""
        baseFileName = "file"
        baseFileName = baseFileName + str(i + 1)
        f = open(baseFileName, 'w')

        #create the random word
        randWord = ""
        for k in range (0, 10):
            randWord = randWord + random.choice(string.ascii_lowercase)
        

        #append the newline character
        randWord = randWord + "\n"

        #display the randomly generated word
        sys.stdout.write(randWord)

        #write randomly generated string to file
        f.write(randWord)

        f.close()

        

def randomAddition():
    random.seed(a=None, version=2)

    rand1 = random.randint(1, 42)
    rand2 = random.randint(1, 42)

    print(rand1)
    print(rand2)
    print(rand1 * rand2)




def main():

    createFiles()
    randomAddition()






if __name__ == '__main__':
    main()