all: rooms adventure

rooms: francomm.buildrooms.c
	gcc -g -Wall -o rooms francomm.buildrooms.c 

adventure: francomm.adventure.c
	gcc -g -Wall -lpthread -o adventure francomm.adventure.c


clean:
	rm -f rooms
	rm -f Azcanta
	rm -f BandRoom
	rm -f Fire
	rm -f Ice
	rm -f Kolios
	rm -f Water
	rm -f Wind
