# level50
My final project for cs50 is "LEVEL50", a text-based command line Pokemon battle simulator written 
entirely in C.

C is a really cool language, and the initial reason for beginning to write LEVEL50 was to learn 
more about it. During the early weeks of cs50, I wrote some code that simulated a VERY stripped-
down Pokemon battle as a way for me to better understand the use of pointers in C. I continued.
adding bits and pieces to the sim to hone my skills, and eventually decided to expand the scope 
of the simulator and work on it as my final project. For a while I believed I'd bitten off more 
than I could chew, but I just kept debugging and adding features until I was satisfied. Maybe one 
day I'll write a full Pokemon-style game, but I probably won't use C for it lol. This project 
would likely have been a lot easier using Python alongside a database or file reader (instead of 
initializing the entire Pokedex and every move at the beginning of the battle lol) but using C
has been amazing practice and is quite fulfilling.

MANY hours have gone into this course and this project, and I'm quite proud of my work on both. 
At the beginning of cs50 I had never written a "Hello world" program, and had never taken any 
sort of computer science class. Today, I have enough knowledge to create something like LEVEL50, 
and I have experience using C, Python, SQL, HTTP, CSS, Javascript, and Flask. THANK YOU SO MUCH
to the cs50 team and to Professer Malan for your amazing contribution to the world. You guys have
an incredible course model and you are all educators of the highest caliber. 



LEVEL50 info dump:

LEVEL50 is text-based and can be played from a command line terminal.
You must be able to compile C to play the game.
Simply download LEVEL50.c, compile it, and run the compiled program.
All fully-evolved Pokemon from the orginal 151 are in the game, minus Ditto, plus Pikachu.
The game uses a mix of mechanics. The moves, types, and base stats are drawn from 5th gen.
No EVs, IVs all assumed to be 31.
Physical/Special split exists. Move priority exists. STAB exists. Super-effective etc. exists.
Status effects and stat mods exist.
There are no abilities (like gens 1 and 2), and no items.
Each Pokemon has a premade moveset (kind of like Pokemon Stadium).
The damage formula is my own tweak on the one from the games.
The battle format is 3v3 single battle. All Pokemon are set to Level 50.
User selects which Pokemon they want, and which ones they want to fight against.
The battle ends when either all user mons faint, all foe mons faint, or the user Escapes.
User may fight, switch Pokemon, escape, and view party stats.
Sleep/Freeze have a chance of ending every turn, even for Rest users.
No critical hits. Status moves don't check type (you can Poison Muk).
You can change a monster's status even if it already has a condition, but will lose the original status.
Roost doesnt remove flying type. Steel wing doesn't raise defense. Weather doesn't exist.
The code is 6000+ lines of the finest hand-coded megaspaghetti.

I hope LEVEL50 can be illuminating, entertaining, instructional, or otherwise beneficial.
Enjoy, download, and share as you like.
If you have any questions, you can email me at brysonmcarter@gmail.com.


//BRYSONCARTER2020
