#include <stdio.h>
#include <cs50.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



//LEVEL50, a text-based command-line pokemon battle simulator written entirely in C
//By Bryson Carter
//cs50 final project

//https://github.com/BRYSONCARTER/level50







//UPDATES:
//i think i have recoil and absorb hp worked out. see fight() and deplete() and their foeforms
//pretty sure speed and priority check works now
//pretty sure major status infliction and retention works now
//u can type in lowercase letters now lol
//u can check ur party stats and moves and hp
//implemented status checks at beginning of fight and foe fight. if paralyzed, roll. if confused, roll. if asleep, roll. if flinched, roll
//all moves added (that i want to add)
//all fully-evolved pokemon added (minus ditto plus pikachu)
//pretty sure all end of turn checks are in place
//pretty sure status solution (awake, unconfuse, unfreeze) works using fight()
//p sure secondary effects of attacking moves all work now. flinching, status, etc. test each status on both sides befopre u rolll out tho
//stat mods work in damage foprmulas
//speed ties work now
//alll mons have thier moves
//pretty sure status moves work now, test them for bugs tho
//ai doesnt attack twice
//hp uses global pointer system
//speed checks use global pointer system
//speed mods work, agility works,
//u can request mons by name now B) hecc ya
//p sure i fixed the toxic/burn not killing and the extra switching and fainting






//first, struct and array definitions------------------------------------------------------------------------------------------------------------------------------------------------------------------


//this is the struct for one move. ------------------------------------------------------------------------------------------------------------------

struct move{
    string name;
    int basepower;
    int acc;
    string type;
    int typeint;
    string effect; //poison burn etc. will have an effect function with a bunch of "if string == flinch, then set foe's flinch counter to 1, if string == burn, then burn them" also wrap goes here
    int effectchance;
    int effectint;
    int priority;
    int physicalorspecial; //0 is physical, 1 is special
    int attackorstatus; //0 is attacking move, 1 is a status/non-attacking move
    int recoilordrain; //0 is none, -1 is 1/8 recoil, +1 is 1/8 drain, +4 is 4/8 drain
    int hitcount; //1 for most moves, 2 for double kick, 5 for fury attack
    int target; // 0 for other mon, 1 for self target
};

//we will reference moves by their index in the movedex array

struct move movedex[100];
struct move moveset[4]; //when we send in a pokemon, we write their moveset to this array. that way we can reference them as moveset[n]
struct move foemoveset[4];

//this is the struct for one pokedex entry. contains data shared by all members of a species---------------------------------------------------------

struct dex{
    string name;
    int baseatk;
    int basedef;
    int basespatk;
    int basespdef;
    int basespeed;
    int basehp;
    int growthmod;
    int expyield;
    int evolveat;
    int evolveto;
    string type1;
    string type2;
    int typeint1; //you will notice type integers throughout. why? because passing strings around in c is a FUCKING NIGHTMARE lmao, much easier to pass integers around
    int typeint2;
    struct move move1;
    struct move move2;
    struct move move3;
    struct move move4;
};

//we will reference dex structs as part of the pokedex[151] array

struct dex pokedex[151];



//this contains the information for one individual pokemon-------------------------------------------------------------------------------------------

struct pokemon{
    int dexno; //pokedex[dexno] contains the species info
    string nickname;
    string name;
    int xp;
    int growthmod;
    int lvl;
    int atk;
    int def;
    int maxhp;
    int currenthp;
    int spatk;
    int spdef;
    int speed;
    string type1;
    string type2;
    int typeint1;
    int typeint2;
    int statusint; //0 for healthy 1 burn 2 freeze 3 sleep 4 poison 5 paralyze 6 fainted
    int *statusintpointer;
    string status; //make this a function of statusint
    struct move move1;
    struct move move2;
    struct move move3;
    struct move move4;
};

//we will reference individuals by their index number in either party or foeparty. change to 6 for final version?

struct pokemon party[3];
struct pokemon foeparty[3];

//probably won't end up using this. hypothetical storage for mons for many different trainers
struct pokemon trainermons[360];

//probably won't use this either. in overworld, this would be pc pokemon storage
struct pokemon box[100];


//this contains stat mods and stuff for the pokemon currently in battle------------------------------------------------------------------------------

struct combatmon{
    int dexnumber; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
    string nickname;
    struct pokemon individualdata; //derive all stats from relevant indivual's struct. can access pokedex struct through indiv struct as well.
    int atkmod; // combatants[0].hp = combatants[0].individualdata.currenthp
    int defmod;  //combatants[0].type1 = pokedex[ dexnumber ].type1
    int spatkmod;
    int spdefmod;
    int speedmod;
    int evasionmod;
    int accuracymod;
    int critmod;
    int protectcounter;
    string status;
    int statusint; //0 for healthy 1 burn 2 freeze 3 sleep 4 poison 5 paralyze 6 fainted
    int confused;
    int disabled;
    int disabledmove; //1 thru 4, denotes which move in a moveset it disabled
    int toxiccounter; //0 if no toxic,
    int leechseeded;
    int flinched;
    int substitute;
    int substitutehp;
    string hyperskyskull; //if this is hyperbeam, cant move. if this is sky attack or skull bash, cant move. same with dig and fly
    int invulnerable; //protect, fly, dig
    int caughtinwrap; //if theyre being wrapped or fire spun. subtract 1/16 hp and print "user is caught in foe's attack!"
    int cursed;
    int biding; //0 if not biding, 1 if biding
    int bidedamagetracker; // if pokemon is biding, track damage it recieves
    int countering;
    int counterdamagetracker;
    int thrashcounter; //set to 2 or 3 when poke uses thrash or petal dance. decrease each turn. when it hits 0, confuse the user
    int whichpartymemberami; //set to 0, 1 or 2 so we can do &party[whichpartymemberami].currenthp
};

//we will reference active combatants as either index 0 or 1 in the combatants array

struct combatmon combatants[2];


//below are effects that stay on the field even if we switch pokemon

struct fieldeffects{
    int reflect;
    int lightscreen;
    int safeguard;
    int mist;
    int lastmoveused; //for mirror move. the int will n for be movedex[n]
};

//0 is our side, 1 is their side. no weather 1st gen lol

struct fieldeffects field[2];

//the below struct should be used as a container for move info after user and ai are queried for moves. then speed check will look at priority and speed to see whether fight or foefight goes first
struct moveselection{
    struct move movechoice;
    int STAB;
    struct combatmon attacker;
    struct combatmon defender;
};

//0 is our side, 1 is their side
struct moveselection bothmoves[2];

int typechart[18][18]; //contains type matchups

//here are a couple toggles.

int isover = 0; //if this is 0, the battle doesnt end. if this is 1, the battle ends.
int *isoverpointer = &isover; //make a pointer so we can change this from anywhere
int run = 0;
int *runpointer = &run; //same deal but with E to ESCAPE
int didfoejustfaint = 0; //0 if no 1 if yes. this is so that opponent doesnt get a free turn after getting kod
int *foefaint = &didfoejustfaint;
int diduserjustfaint = 0;
int *userfaint = &diduserjustfaint;
int userswitch = 0;
int *userswitchpointer = &userswitch;
int combatcurrenthp = 1;
int *combathppointer = &combatcurrenthp;
int foecombatcurrenthp = 1;
int *foecombathppointer = &foecombatcurrenthp;
int combatspeed = 1;
int *combatspeedpointer = &combatspeed;
int foecombatspeed = 1;
int *foecombatspeedpointer = &foecombatspeed;
int loopcount = 0;
int *loopcountpointer = &loopcount;

int urmon1;
int urmon2;
int urmon3;
int *urmon1ptr = &urmon1;
int *urmon2ptr = &urmon2;
int *urmon3ptr = &urmon3;

int theirmon1;
int theirmon2;
int theirmon3;
int *theirmon1ptr = &theirmon1;
int *theirmon2ptr = &theirmon2;
int *theirmon3ptr = &theirmon3;



//then, function declarations go here ----------------------------------------------------------------------------------------------------------------------------------------------------------------

int query(string option, struct combatmon ourfighter); //gee why do u have dual functions for self and foe?
int foequery(struct combatmon theirfighter);          // because hp pointers
int fight(struct move moveused, int STAB, struct combatmon attacker, struct combatmon defender);
int foefight(struct move moveused, int STAB, struct combatmon attacker, struct combatmon defender);
int attack(struct combatmon attacker, struct combatmon defender, struct move movestruct, int STAB);
int foeattack(struct combatmon attacker, struct combatmon defender, struct move movestruct, int STAB);
int deplete(int *hppoint, int damage, struct pokemon victim);
int foedeplete(int *hppoint, int damage, struct pokemon victim);
float speedchange(int statmod, int statusint); //factors in paralysis
float attackchange(int atkmod, int statusint); //factors in burn
float statchange(int statmod); //use for all other stats
string statinttostring(int g);

void userselection1(void);
void userselection2(void);
void userselection3(void);

void foeselection1(void);
void foeselection2(void);
void foeselection3(void);





//now, main function-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int main(void)
{
    //seed rng
    srand( time(NULL) );

    printf("\n\n                   LEVEL50\nA Pokemon battle simulator written entirely in C\n             --cs50 Final Project--\n             --BRYSON CARTER 2020--\n\n\n\n");

    //first initialize movedex--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //we do this first because the pokemon structs have moves in them so we gotta initialize these b4 those

    movedex[0].name = "Tackle";
    movedex[0].basepower = 40;
    movedex[0].acc = 95;
    movedex[0].type = "Normal";
    movedex[0].typeint = 1;
    movedex[0].effect = "None"; //poison burn etc. will have an effect function with a bunch of "if string == flinch, then set foe's flinch counter to 1, if string == burn, then burn them"
    movedex[0].effectchance = 0;
    movedex[0].priority = 0;
    movedex[0].attackorstatus = 0;
    movedex[0].physicalorspecial = 0;
    movedex[0].recoilordrain = 0;
    movedex[0].hitcount = 1;
    movedex[0].target = 0;

    movedex[1].name = "Vine Whip";
    movedex[1].basepower = 40;
    movedex[1].acc = 95;
    movedex[1].type = "Grass";
    movedex[1].typeint = 5;
    movedex[1].effect = "None";
    movedex[1].effectchance = 0;
    movedex[1].priority = 0;
    movedex[1].attackorstatus = 0;
    movedex[1].physicalorspecial = 0;
    movedex[1].recoilordrain = 0;
    movedex[1].hitcount = 1;
    movedex[1].target = 0;

    movedex[2].name = "Ember";
    movedex[2].basepower = 40;
    movedex[2].acc = 95;
    movedex[2].type = "Fire";
    movedex[2].typeint = 2;
    movedex[2].effect = "Burn";
    movedex[2].effectint = 1; //ok so i have effectint here but i might just use strcmp its not that much different
    movedex[2].effectchance = 100;
    movedex[2].priority = 0;
    movedex[2].attackorstatus = 0;
    movedex[2].physicalorspecial = 1;
    movedex[2].recoilordrain = 0;
    movedex[2].hitcount = 1;
    movedex[2].target = 0;

    movedex[3].name = "Water Gun";
    movedex[3].basepower = 40;
    movedex[3].acc = 95;
    movedex[3].type = "Water";
    movedex[3].typeint = 3;
    movedex[3].effect = "None";
    movedex[3].effectchance = 0;
    movedex[3].priority = 0;
    movedex[3].attackorstatus = 0;
    movedex[3].physicalorspecial = 1;
    movedex[3].recoilordrain = 0;
    movedex[3].hitcount = 1;
    movedex[3].target = 0;

    movedex[4].name = "Absorb";
    movedex[4].basepower = 40;
    movedex[4].acc = 100;
    movedex[4].type = "Grass";
    movedex[4].typeint = 5;
    movedex[4].effect = "None";
    movedex[4].effectchance = 0;
    movedex[4].priority = 0;
    movedex[4].attackorstatus = 0;
    movedex[4].physicalorspecial = 1;
    movedex[4].recoilordrain = 4;
    movedex[4].hitcount = 1;
    movedex[4].target = 0;

    movedex[5].name = "Acid";
    movedex[5].basepower = 40;
    movedex[5].acc = 100;
    movedex[5].type = "Poison";
    movedex[5].typeint = 8;
    movedex[5].effect = "-def"; //i might actually just string compare for the effect
    movedex[5].effectchance = 10; ///like a bunch of if strcmp(movedex[5].effect, "-def") == 0) then do this
    movedex[5].priority = 0;
    movedex[5].attackorstatus = 0;
    movedex[5].physicalorspecial = 1;
    movedex[5].recoilordrain = 0;
    movedex[5].hitcount = 1;
    movedex[5].target = 0;

    movedex[6].name = "Acid Armor";
    movedex[6].basepower = 0;
    movedex[6].acc = 100;
    movedex[6].type = "Poison";
    movedex[6].typeint = 8;
    movedex[6].effect = "+2def";
    movedex[6].effectchance = 100;
    movedex[6].priority = 0;
    movedex[6].attackorstatus = 1;
    movedex[6].physicalorspecial = 1;
    movedex[6].recoilordrain = 0;
    movedex[6].hitcount = 1;
    movedex[6].target = 1;

    movedex[7].name = "Agility";
    movedex[7].basepower = 0;
    movedex[7].acc = 100;
    movedex[7].type = "Normal";
    movedex[7].typeint = 1;
    movedex[7].effect = "+2speed";
    movedex[7].effectchance = 100;
    movedex[7].priority = 0;
    movedex[7].attackorstatus = 1;
    movedex[7].physicalorspecial = 1;
    movedex[7].recoilordrain = 0;
    movedex[7].hitcount = 1;
    movedex[7].target = 1;

    movedex[8].name = "Amnesia";
    movedex[8].basepower = 0;
    movedex[8].acc = 100;
    movedex[8].type = "Psychic";
    movedex[8].typeint = 11;
    movedex[8].effect = "+2spdef";
    movedex[8].effectchance = 100;
    movedex[8].priority = 0;
    movedex[8].attackorstatus = 1;
    movedex[8].physicalorspecial = 1;
    movedex[8].recoilordrain = 4;
    movedex[8].hitcount = 1;
    movedex[8].target = 1;

    movedex[9].name = "Aurora Beam";
    movedex[9].basepower = 65;
    movedex[9].acc = 100;
    movedex[9].type = "Ice";
    movedex[9].typeint = 6;
    movedex[9].effect = "-atk";
    movedex[9].effectchance = 10;
    movedex[9].priority = 0;
    movedex[9].attackorstatus = 0;
    movedex[9].physicalorspecial = 1;
    movedex[9].recoilordrain = 0;
    movedex[9].hitcount = 1;
    movedex[9].target = 0;

    movedex[10].name = "Barrage";
    movedex[10].basepower = 15;
    movedex[10].acc = 85;
    movedex[10].type = "Normal";
    movedex[10].typeint = 1;
    movedex[10].effect = "None";
    movedex[10].effectchance = 0;
    movedex[10].priority = 0;
    movedex[10].attackorstatus = 0;
    movedex[10].physicalorspecial = 0;
    movedex[10].recoilordrain = 0;
    movedex[10].hitcount = 5;
    movedex[10].target = 0;

    movedex[11].name = "Barrier";
    movedex[11].basepower = 0;
    movedex[11].acc = 100;
    movedex[11].type = "Psychic";
    movedex[11].typeint = 11;
    movedex[11].effect = "+2def";
    movedex[11].effectchance = 100;
    movedex[11].priority = 0;
    movedex[11].attackorstatus = 1;
    movedex[11].physicalorspecial = 1;
    movedex[11].recoilordrain = 0;
    movedex[11].hitcount = 1;
    movedex[11].target = 1;

    movedex[12].name = "Bide";

    movedex[13].name = "Bind";

    movedex[14].name = "Bite";
    movedex[14].basepower = 60;
    movedex[14].acc = 100;
    movedex[14].type = "Dark";
    movedex[14].typeint = 16;
    movedex[14].effect = "Flinch";
    movedex[14].effectchance = 30;
    movedex[14].priority = 0;
    movedex[14].attackorstatus = 0;
    movedex[14].physicalorspecial = 0;
    movedex[14].recoilordrain = 0;
    movedex[14].hitcount = 1;
    movedex[14].target = 0;

    movedex[15].name = "Blizzard";
    movedex[15].basepower = 120;
    movedex[15].acc = 70;
    movedex[15].type = "Ice";
    movedex[15].typeint = 6;
    movedex[15].effect = "Freeze";
    movedex[15].effectchance = 10;
    movedex[15].priority = 0;
    movedex[15].attackorstatus = 0;
    movedex[15].physicalorspecial = 1;
    movedex[15].recoilordrain = 0;
    movedex[15].hitcount = 1;
    movedex[15].target = 0;

    movedex[16].name = "Body Slam";
    movedex[16].basepower = 85;
    movedex[16].acc = 100;
    movedex[16].type = "Normal";
    movedex[16].typeint = 1;
    movedex[16].effect = "Paralyze";
    movedex[16].effectchance = 30;
    movedex[16].priority = 0;
    movedex[16].attackorstatus = 0;
    movedex[16].physicalorspecial = 0;
    movedex[16].recoilordrain = 0;
    movedex[16].hitcount = 1;
    movedex[16].target = 0;

    movedex[17].name = "Bone Club";
    movedex[17].basepower = 65;
    movedex[17].acc = 85;
    movedex[17].type = "Ground";
    movedex[17].typeint = 9;
    movedex[17].effect = "Flinch";
    movedex[17].effectchance = 10;
    movedex[17].priority = 0;
    movedex[17].attackorstatus = 0;
    movedex[17].physicalorspecial = 0;
    movedex[17].recoilordrain = 0;
    movedex[17].hitcount = 1;

    movedex[18].name = "Bonemarang";
    movedex[18].basepower = 50;
    movedex[18].acc = 90;
    movedex[18].type = "Ground";
    movedex[18].typeint = 9;
    movedex[18].effect = "None";
    movedex[18].effectchance = 0;
    movedex[18].priority = 0;
    movedex[18].attackorstatus = 0;
    movedex[18].physicalorspecial = 0;
    movedex[18].recoilordrain = 0;
    movedex[18].hitcount = 2;
    movedex[18].target = 0;

    movedex[19].name = "Confuse Ray";
    movedex[19].basepower = 0;
    movedex[19].acc = 100;
    movedex[19].type = "Ghost";
    movedex[19].typeint = 14;
    movedex[19].effect = "Confuse";
    movedex[19].effectchance = 100;
    movedex[19].priority = 0;
    movedex[19].attackorstatus = 1;
    movedex[19].physicalorspecial = 1;
    movedex[19].recoilordrain = 0;
    movedex[19].hitcount = 1;
    movedex[19].target = 0;

    movedex[20].name = "Counter";
    movedex[20].basepower = 0;
    movedex[20].acc = 100;
    movedex[20].type = "Fighting";
    movedex[20].typeint = 7;
    movedex[20].effect = "Counter";
    movedex[20].effectchance = 100;
    movedex[20].priority = -5;
    movedex[20].attackorstatus = 1;
    movedex[20].physicalorspecial = 0;
    movedex[20].recoilordrain = 0;
    movedex[20].hitcount = 1;
    movedex[20].target = 0;

    movedex[21].name = "Crabhammer";
    movedex[21].basepower = 100;
    movedex[21].acc = 90;
    movedex[21].type = "Water";
    movedex[21].typeint = 3;
    movedex[21].effect = "Crit";
    movedex[21].effectchance = 6;
    movedex[21].priority = 0;
    movedex[21].attackorstatus = 0;
    movedex[21].physicalorspecial = 0;
    movedex[21].recoilordrain = 0;
    movedex[21].hitcount = 1;
    movedex[21].target = 0;

    movedex[22].name = "Disable";
    movedex[22].basepower = 0;
    movedex[22].acc = 100;
    movedex[22].type = "Normal";
    movedex[22].typeint = 1;
    movedex[22].effect = "Disable";
    movedex[22].effectchance = 100;
    movedex[22].priority = 0;
    movedex[22].attackorstatus = 1;
    movedex[22].physicalorspecial = 1;
    movedex[22].recoilordrain = 0;
    movedex[22].hitcount = 1;
    movedex[22].target = 0;

    movedex[23].name = "Double Kick";
    movedex[23].basepower = 30;
    movedex[23].acc = 100;
    movedex[23].type = "Fighting";
    movedex[23].typeint = 7;
    movedex[23].effect = "None";
    movedex[23].effectchance = 0;
    movedex[23].priority = 0;
    movedex[23].attackorstatus = 0;
    movedex[23].physicalorspecial = 0;
    movedex[23].recoilordrain = 0;
    movedex[23].hitcount = 2;
    movedex[23].target = 0;

    movedex[24].name = "Double Team";
    movedex[24].basepower = 0;
    movedex[24].acc = 100;
    movedex[24].type = "Normal";
    movedex[24].typeint = 1;
    movedex[24].effect = "+1eva";
    movedex[24].effectchance = 100;
    movedex[24].priority = 0;
    movedex[24].attackorstatus = 1;
    movedex[24].physicalorspecial = 1;
    movedex[24].recoilordrain = 0;
    movedex[24].hitcount = 1;
    movedex[24].target = 1;

    movedex[25].name = "Double Edge";
    movedex[25].basepower = 120;
    movedex[25].acc = 100;
    movedex[25].type = "Normal";
    movedex[25].typeint = 1;
    movedex[25].effect = "None";
    movedex[25].effectchance = 0;
    movedex[25].priority = 0;
    movedex[25].attackorstatus = 0;
    movedex[25].physicalorspecial = 0;
    movedex[25].recoilordrain = -2;
    movedex[25].hitcount = 1;
    movedex[25].target = 0;

    movedex[26].name = "Flamethrower"; //
    movedex[26].basepower = 95;
    movedex[26].acc = 100;
    movedex[26].type = "Fire";
    movedex[26].typeint = 2;
    movedex[26].effect = "Burn";
    movedex[26].effectchance = 10;
    movedex[26].priority = 0;
    movedex[26].attackorstatus = 0;
    movedex[26].physicalorspecial = 1;
    movedex[26].recoilordrain = 0;
    movedex[26].hitcount = 1;
    movedex[26].target = 0;

    movedex[27].name = "Surf";
    movedex[27].basepower = 95;
    movedex[27].acc = 100;
    movedex[27].type = "Water";
    movedex[27].typeint = 3;
    movedex[27].effect = "None";
    movedex[27].effectchance = 0;
    movedex[27].priority = 0;
    movedex[27].attackorstatus = 0;
    movedex[27].physicalorspecial = 1;
    movedex[27].recoilordrain = 0;
    movedex[27].hitcount = 1;
    movedex[27].target = 0;

    movedex[28].name = "Giga Drain";
    movedex[28].basepower = 75;
    movedex[28].acc = 100;
    movedex[28].type = "Grass";
    movedex[28].typeint = 5;
    movedex[28].effect = "None";
    movedex[28].effectchance = 0;
    movedex[28].priority = 0;
    movedex[28].attackorstatus = 0;
    movedex[28].physicalorspecial = 1;
    movedex[28].recoilordrain = 4;
    movedex[28].hitcount = 1;
    movedex[28].target = 0;

    movedex[29].name = "Sludge Bomb";
    movedex[29].basepower = 90;
    movedex[29].acc = 100;
    movedex[29].type = "Poison";
    movedex[29].typeint = 8;
    movedex[29].effect = "Poison";
    movedex[29].effectchance = 10;
    movedex[29].priority = 0;
    movedex[29].attackorstatus = 0;
    movedex[29].physicalorspecial = 1;
    movedex[29].recoilordrain = 0;
    movedex[29].hitcount = 1;
    movedex[29].target = 0;

    movedex[30].name = "Earthquake";
    movedex[30].basepower = 100;
    movedex[30].acc = 100;
    movedex[30].type = "Ground";
    movedex[30].typeint = 9;
    movedex[30].effect = "None";
    movedex[30].effectchance = 0;
    movedex[30].priority = 0;
    movedex[30].attackorstatus = 0;
    movedex[30].physicalorspecial = 0;
    movedex[30].recoilordrain = 0;
    movedex[30].hitcount = 1;
    movedex[30].target = 0;

    movedex[31].name = "Toxic";
    movedex[31].basepower = 0;
    movedex[31].acc = 90;
    movedex[31].type = "Poison";
    movedex[31].typeint = 8;
    movedex[31].effect = "Toxic";
    movedex[31].effectchance = 100;
    movedex[31].priority = 0;
    movedex[31].attackorstatus = 1;
    movedex[31].physicalorspecial = 1;
    movedex[31].recoilordrain = 0;
    movedex[31].hitcount = 1;
    movedex[31].target = 0;

    movedex[32].name = "Air Slash";
    movedex[32].basepower = 75;
    movedex[32].acc = 95;
    movedex[32].type = "Flying";
    movedex[32].typeint = 10;
    movedex[32].effect = "Flinch";
    movedex[32].effectchance = 30;
    movedex[32].priority = 0;
    movedex[32].attackorstatus = 0;
    movedex[32].physicalorspecial = 1;
    movedex[32].recoilordrain = 0;
    movedex[32].hitcount = 1;
    movedex[32].target = 0;

    movedex[33].name = "Fire Blast";
    movedex[33].basepower = 120;
    movedex[33].acc = 85;
    movedex[33].type = "Fire";
    movedex[33].typeint = 2;
    movedex[33].effect = "Burn";
    movedex[33].effectchance = 10;
    movedex[33].priority = 0;
    movedex[33].attackorstatus = 0;
    movedex[33].physicalorspecial = 1;
    movedex[33].recoilordrain = 0;
    movedex[33].hitcount = 1;
    movedex[33].target = 0;

    movedex[34].name = "Ice Beam";
    movedex[34].basepower = 95;
    movedex[34].acc = 100;
    movedex[34].type = "Ice";
    movedex[34].typeint = 6;
    movedex[34].effect = "Freeze";
    movedex[34].effectchance = 10;
    movedex[34].priority = 0;
    movedex[34].attackorstatus = 0;
    movedex[34].physicalorspecial = 1;
    movedex[34].recoilordrain = 0;
    movedex[34].hitcount = 1;
    movedex[34].target = 0;

    movedex[35].name = "Bug Buzz";
    movedex[35].basepower = 90;
    movedex[35].acc = 100;
    movedex[35].type = "Bug";
    movedex[35].typeint = 12;
    movedex[35].effect = "-spdef";
    movedex[35].effectchance = 10;
    movedex[35].priority = 0;
    movedex[35].attackorstatus = 0;
    movedex[35].physicalorspecial = 1;
    movedex[35].recoilordrain = 0;
    movedex[35].hitcount = 1;
    movedex[35].target = 0;

    movedex[36].name = "Psychic";
    movedex[36].basepower = 90;
    movedex[36].acc = 100;
    movedex[36].type = "Psychic";
    movedex[36].typeint = 11;
    movedex[36].effect = "-spdef";
    movedex[36].effectchance = 10;
    movedex[36].priority = 0;
    movedex[36].attackorstatus = 0;
    movedex[36].physicalorspecial = 1;
    movedex[36].recoilordrain = 0;
    movedex[36].hitcount = 1;
    movedex[36].target = 0;

    movedex[37].name = "Sleep Powder";
    movedex[37].basepower = 0;
    movedex[37].acc = 75;
    movedex[37].type = "Grass";
    movedex[37].typeint = 5;
    movedex[37].effect = "Sleep";
    movedex[37].effectchance = 100;
    movedex[37].priority = 0;
    movedex[37].attackorstatus = 1;
    movedex[37].physicalorspecial = 1;
    movedex[37].recoilordrain = 0;
    movedex[37].hitcount = 1;
    movedex[37].target = 0;

    movedex[38].name = "Poisonpowder";
    movedex[38].basepower = 0;
    movedex[38].acc = 75;
    movedex[38].type = "Grass";
    movedex[38].typeint = 5;
    movedex[38].effect = "Poison";
    movedex[38].effectchance = 100;
    movedex[38].priority = 0;
    movedex[38].attackorstatus = 1;
    movedex[38].physicalorspecial = 1;
    movedex[38].recoilordrain = 0;
    movedex[38].hitcount = 1;
    movedex[38].target = 0;

    movedex[39].name = "Poison Jab";
    movedex[39].basepower = 80;
    movedex[39].acc = 100;
    movedex[39].type = "Poison";
    movedex[39].typeint = 8;
    movedex[39].effect = "Poison";
    movedex[39].effectchance = 30;
    movedex[39].priority = 0;
    movedex[39].attackorstatus = 0;
    movedex[39].physicalorspecial = 0;
    movedex[39].recoilordrain = 0;
    movedex[39].hitcount = 1;
    movedex[39].target = 0;

    movedex[40].name = "X-Scissor";
    movedex[40].basepower = 80;
    movedex[40].acc = 100;
    movedex[40].type = "Bug";
    movedex[40].typeint = 12;
    movedex[40].effect = "None";
    movedex[40].effectchance = 0;
    movedex[40].priority = 0;
    movedex[40].attackorstatus = 0;
    movedex[40].physicalorspecial = 0;
    movedex[40].recoilordrain = 0;
    movedex[40].hitcount = 1;
    movedex[40].target = 0;

    movedex[41].name = "Quick Attack";
    movedex[41].basepower = 40;
    movedex[41].acc = 100;
    movedex[41].type = "Normal";
    movedex[41].typeint = 1;
    movedex[41].effect = "None";
    movedex[41].effectchance = 0;
    movedex[41].priority = 1;
    movedex[41].attackorstatus = 0;
    movedex[41].physicalorspecial = 0;
    movedex[41].recoilordrain = 0;
    movedex[41].hitcount = 1;
    movedex[41].target = 0;

    movedex[42].name = "Roost";
    movedex[42].basepower = 0;
    movedex[42].acc = 100;
    movedex[42].type = "Flying";
    movedex[42].typeint = 10;
    movedex[42].effect = "Restore";
    movedex[42].effectchance = 100;
    movedex[42].priority = 0;
    movedex[42].attackorstatus = 1;
    movedex[42].physicalorspecial = 1;
    movedex[42].recoilordrain = 0;
    movedex[42].hitcount = 1;
    movedex[42].target = 0;

    movedex[43].name = "Thunderbolt";
    movedex[43].basepower = 95;
    movedex[43].acc = 100;
    movedex[43].type = "Electric";
    movedex[43].typeint = 4;
    movedex[43].effect = "Paralyze";
    movedex[43].effectchance = 10;
    movedex[43].priority = 0;
    movedex[43].attackorstatus = 0;
    movedex[43].physicalorspecial = 1;
    movedex[43].recoilordrain = 0;
    movedex[43].hitcount = 1;
    movedex[43].target = 0;

    movedex[44].name = "Thunderwave";
    movedex[44].basepower = 0;
    movedex[44].acc = 100;
    movedex[44].type = "Electric";
    movedex[44].typeint = 5;
    movedex[44].effect = "Paralyze";
    movedex[44].effectchance = 100;
    movedex[44].priority = 0;
    movedex[44].attackorstatus = 1;
    movedex[44].physicalorspecial = 1;
    movedex[44].recoilordrain = 0;
    movedex[44].hitcount = 1;
    movedex[44].target = 0;

    movedex[45].name = "Drill Peck";
    movedex[45].basepower = 80;
    movedex[45].acc = 100;
    movedex[45].type = "Flying";
    movedex[45].typeint = 10;
    movedex[45].effect = "None";
    movedex[45].effectchance = 0;
    movedex[45].priority = 0;
    movedex[45].attackorstatus = 0;
    movedex[45].physicalorspecial = 0;
    movedex[45].recoilordrain = 0;
    movedex[45].hitcount = 1;
    movedex[45].target = 0;

    movedex[46].name = "Agility";
    movedex[46].basepower = 0;
    movedex[46].acc = 100;
    movedex[46].type = "Psychic";
    movedex[46].typeint = 11;
    movedex[46].effect = "+2speed";
    movedex[46].effectchance = 100;
    movedex[46].priority = 0;
    movedex[46].attackorstatus = 1;
    movedex[46].physicalorspecial = 1;
    movedex[46].recoilordrain = 0;
    movedex[46].hitcount = 1;
    movedex[46].target = 1;

    movedex[47].name = "Crunch";
    movedex[47].basepower = 80;
    movedex[47].acc = 100;
    movedex[47].type = "Dark";
    movedex[47].typeint = 16;
    movedex[47].effect = "-def";
    movedex[47].effectchance = 20;
    movedex[47].priority = 0;
    movedex[47].attackorstatus = 0;
    movedex[47].physicalorspecial = 0;
    movedex[47].recoilordrain = 0;
    movedex[47].hitcount = 1;
    movedex[47].target = 0;

    movedex[48].name = "Thunder";
    movedex[48].basepower = 120;
    movedex[48].acc = 70;
    movedex[48].type = "Electric";
    movedex[48].typeint = 4;
    movedex[48].effect = "Paralyze";
    movedex[48].effectchance = 30;
    movedex[48].priority = 0;
    movedex[48].attackorstatus = 0;
    movedex[48].physicalorspecial = 1;
    movedex[48].recoilordrain = 0;
    movedex[48].hitcount = 1;
    movedex[48].target = 0;

    movedex[49].name = "Nasty Plot";
    movedex[49].basepower = 0;
    movedex[49].acc = 100;
    movedex[49].type = "Dark";
    movedex[49].typeint = 16;
    movedex[49].effect = "+2spatk";
    movedex[49].effectchance = 100;
    movedex[49].priority = 0;
    movedex[49].attackorstatus = 1;
    movedex[49].physicalorspecial = 1;
    movedex[49].recoilordrain = 0;
    movedex[49].hitcount = 1;
    movedex[49].target = 1;

    movedex[50].name = "Swords Dance";
    movedex[50].basepower = 0;
    movedex[50].acc = 100;
    movedex[50].type = "Normal";
    movedex[50].typeint = 1;
    movedex[50].effect = "+2atk";
    movedex[50].effectchance = 100;
    movedex[50].priority = 0;
    movedex[50].attackorstatus = 1;
    movedex[50].physicalorspecial = 1;
    movedex[50].recoilordrain = 0;
    movedex[50].hitcount = 1;
    movedex[50].target = 1;

    movedex[51].name = "Brick Break";
    movedex[51].basepower = 75;
    movedex[51].acc = 100;
    movedex[51].type = "Fighting";
    movedex[51].typeint = 7;
    movedex[51].effect = "None";
    movedex[51].effectchance = 0;
    movedex[51].priority = 0;
    movedex[51].attackorstatus = 0;
    movedex[51].physicalorspecial = 0;
    movedex[51].recoilordrain = 0;
    movedex[51].hitcount = 1;
    movedex[51].target = 0;

    movedex[52].name = "Energy Ball";
    movedex[52].basepower = 80;
    movedex[52].acc = 100;
    movedex[52].type = "Grass";
    movedex[52].typeint = 5;
    movedex[52].effect = "-spdef";
    movedex[52].effectchance = 10;
    movedex[52].priority = 0;
    movedex[52].attackorstatus = 0;
    movedex[52].physicalorspecial = 1;
    movedex[52].recoilordrain = 0;
    movedex[52].hitcount = 1;
    movedex[52].target = 0;

    movedex[53].name = "Dark Pulse";
    movedex[53].basepower = 80;
    movedex[53].acc = 100;
    movedex[53].type = "Dark";
    movedex[53].typeint = 16;
    movedex[53].effect = "Flinch";
    movedex[53].effectchance = 20;
    movedex[53].priority = 0;
    movedex[53].attackorstatus = 0;
    movedex[53].physicalorspecial = 1;
    movedex[53].recoilordrain = 0;
    movedex[53].hitcount = 1;
    movedex[53].target = 0;

    movedex[54].name = "Spore";
    movedex[54].basepower = 0;
    movedex[54].acc = 100;
    movedex[54].type = "Grass";
    movedex[54].typeint = 5;
    movedex[54].effect = "Sleep";
    movedex[54].effectchance = 100;
    movedex[54].priority = 0;
    movedex[54].attackorstatus = 1;
    movedex[54].physicalorspecial = 1;
    movedex[54].recoilordrain = 0;
    movedex[54].hitcount = 1;
    movedex[54].target = 0;

    movedex[55].name = "Rock Slide";
    movedex[55].basepower = 75;
    movedex[55].acc = 90;
    movedex[55].type = "Rock";
    movedex[55].typeint = 13;
    movedex[55].effect = "Flinch";
    movedex[55].effectchance = 30;
    movedex[55].priority = 0;
    movedex[55].attackorstatus = 0;
    movedex[55].physicalorspecial = 0;
    movedex[55].recoilordrain = 0;
    movedex[55].hitcount = 1;
    movedex[55].target = 0;

    movedex[56].name = "Shadow Claw";
    movedex[56].basepower = 70;
    movedex[56].acc = 100;
    movedex[56].type = "Ghost";
    movedex[56].typeint = 14;
    movedex[56].effect = "None";
    movedex[56].effectchance = 0;
    movedex[56].priority = 0;
    movedex[56].attackorstatus = 0;
    movedex[56].physicalorspecial = 0;
    movedex[56].recoilordrain = 0;
    movedex[56].hitcount = 1;
    movedex[56].target = 0;

    movedex[57].name = "Night Slash";
    movedex[57].basepower = 70;
    movedex[57].acc = 100;
    movedex[57].type = "Dark";
    movedex[57].typeint = 16;
    movedex[57].effect = "None";
    movedex[57].effectchance = 0;
    movedex[57].priority = 0;
    movedex[57].attackorstatus = 0;
    movedex[57].physicalorspecial = 0;
    movedex[57].recoilordrain = 0;
    movedex[57].hitcount = 1;
    movedex[57].target = 0;

    movedex[58].name = "Calm Mind";
    movedex[58].basepower = 0;
    movedex[58].acc = 100;
    movedex[58].type = "Psychic";
    movedex[58].typeint = 11;
    movedex[58].effect = "+spatk+spdef";
    movedex[58].effectchance = 100;
    movedex[58].priority = 0;
    movedex[58].attackorstatus = 1;
    movedex[58].physicalorspecial = 1;
    movedex[58].recoilordrain = 0;
    movedex[58].hitcount = 1;
    movedex[58].target = 1;

    movedex[59].name = "Bulk Up";
    movedex[59].basepower = 0;
    movedex[59].acc = 100;
    movedex[59].type = "Fighting";
    movedex[59].typeint = 7;
    movedex[59].effect = "+atk+def";
    movedex[59].effectchance = 100;
    movedex[59].priority = 0;
    movedex[59].attackorstatus = 1;
    movedex[59].physicalorspecial = 1;
    movedex[59].recoilordrain = 0;
    movedex[59].hitcount = 1;
    movedex[59].target = 1;

    movedex[60].name = "Fire Punch";
    movedex[60].basepower = 75;
    movedex[60].acc = 100;
    movedex[60].type = "Fire";
    movedex[60].typeint = 2;
    movedex[60].effect = "Burn";
    movedex[60].effectchance = 10;
    movedex[60].priority = 0;
    movedex[60].attackorstatus = 0;
    movedex[60].physicalorspecial = 0;
    movedex[60].recoilordrain = 0;
    movedex[60].hitcount = 1;
    movedex[60].target = 0;

    movedex[61].name = "Extremespeed";
    movedex[61].basepower = 80;
    movedex[61].acc = 100;
    movedex[61].type = "Normal";
    movedex[61].typeint = 1;
    movedex[61].effect = "None";
    movedex[61].effectchance = 0;
    movedex[61].priority = 2;
    movedex[61].attackorstatus = 0;
    movedex[61].physicalorspecial = 0;
    movedex[61].recoilordrain = 0;
    movedex[61].hitcount = 1;
    movedex[61].target = 0;

    movedex[62].name = "Shadow Ball";
    movedex[62].basepower = 80;
    movedex[62].acc = 100;
    movedex[62].type = "Ghost";
    movedex[62].typeint = 14;
    movedex[62].effect = "-spdef";
    movedex[62].effectchance = 20;
    movedex[62].priority = 0;
    movedex[62].attackorstatus = 0;
    movedex[62].physicalorspecial = 1;
    movedex[62].recoilordrain = 0;
    movedex[62].hitcount = 1;
    movedex[62].target = 0;

    movedex[63].name = "Recover";
    movedex[63].basepower = 0;
    movedex[63].acc = 100;
    movedex[63].type = "Normal";
    movedex[63].typeint = 1;
    movedex[63].effect = "Restore";
    movedex[63].effectchance = 100;
    movedex[63].priority = 0;
    movedex[63].attackorstatus = 1;
    movedex[63].physicalorspecial = 1;
    movedex[63].recoilordrain = 0;
    movedex[63].hitcount = 1;
    movedex[63].target = 1;

    movedex[64].name = "Synthesis";
    movedex[64].basepower = 0;
    movedex[64].acc = 100;
    movedex[64].type = "Grass";
    movedex[64].typeint = 5;
    movedex[64].effect = "Restore";
    movedex[64].effectchance = 100;
    movedex[64].priority = 0;
    movedex[64].attackorstatus = 1;
    movedex[64].physicalorspecial = 1;
    movedex[64].recoilordrain = 0;
    movedex[64].hitcount = 1;
    movedex[64].target = 1;

    movedex[65].name = "Thunderpunch";
    movedex[65].basepower = 75;
    movedex[65].acc = 100;
    movedex[65].type = "Electric";
    movedex[65].typeint = 4;
    movedex[65].effect = "Paralyze";
    movedex[65].effectchance = 10;
    movedex[65].priority = 0;
    movedex[65].attackorstatus = 0;
    movedex[65].physicalorspecial = 0;
    movedex[65].recoilordrain = 0;
    movedex[65].hitcount = 1;
    movedex[65].target = 0;

    movedex[66].name = "Megahorn";
    movedex[66].basepower = 120;
    movedex[66].acc = 100;
    movedex[66].type = "Bug";
    movedex[66].typeint = 12;
    movedex[66].effect = "None";
    movedex[66].effectchance = 0;
    movedex[66].priority = 0;
    movedex[66].attackorstatus = 0;
    movedex[66].physicalorspecial = 0;
    movedex[66].recoilordrain = 0;
    movedex[66].hitcount = 1;
    movedex[66].target = 0;

    movedex[67].name = "Morning Sun";
    movedex[67].basepower = 0;
    movedex[67].acc = 100;
    movedex[67].type = "Fire";
    movedex[67].typeint = 2;
    movedex[67].effect = "Restore";
    movedex[67].effectchance = 100;
    movedex[67].priority = 0;
    movedex[67].attackorstatus = 1;
    movedex[67].physicalorspecial = 1;
    movedex[67].recoilordrain = 0;
    movedex[67].hitcount = 1;
    movedex[67].target = 1;

    movedex[68].name = "Flash Cannon";
    movedex[68].basepower = 80;
    movedex[68].acc = 100;
    movedex[68].type = "Steel";
    movedex[68].typeint = 17;
    movedex[68].effect = "-spdef";
    movedex[68].effectchance = 10;
    movedex[68].priority = 0;
    movedex[68].attackorstatus = 0;
    movedex[68].physicalorspecial = 1;
    movedex[68].recoilordrain = 0;
    movedex[68].hitcount = 1;
    movedex[68].target = 0;

    movedex[69].name = "Iron Defense";
    movedex[69].basepower = 0;
    movedex[69].acc = 100;
    movedex[69].type = "Steel";
    movedex[69].typeint = 17;
    movedex[69].effect = "+2def";
    movedex[69].effectchance = 100;
    movedex[69].priority = 0;
    movedex[69].attackorstatus = 1;
    movedex[69].physicalorspecial = 1;
    movedex[69].recoilordrain = 0;
    movedex[69].hitcount = 1;
    movedex[69].target = 1;

    movedex[70].name = "Brave Bird";
    movedex[70].basepower = 120;
    movedex[70].acc = 100;
    movedex[70].type = "Flying";
    movedex[70].typeint = 10;
    movedex[70].effect = "None";
    movedex[70].effectchance = 0;
    movedex[70].priority = 0;
    movedex[70].attackorstatus = 0;
    movedex[70].physicalorspecial = 0;
    movedex[70].recoilordrain = -2;
    movedex[70].hitcount = 1;
    movedex[70].target = 0;

    movedex[71].name = "Steel Wing";
    movedex[71].basepower = 70;
    movedex[71].acc = 100;
    movedex[71].type = "Steel";
    movedex[71].typeint = 17;
    movedex[71].effect = "None";
    movedex[71].effectchance = 0;
    movedex[71].priority = 0;
    movedex[71].attackorstatus = 0;
    movedex[71].physicalorspecial = 0;
    movedex[71].recoilordrain = 0;
    movedex[71].hitcount = 1;
    movedex[71].target = 0;

    movedex[72].name = "Rest";
    movedex[72].basepower = 0;
    movedex[72].acc = 100;
    movedex[72].type = "Normal";
    movedex[72].typeint = 1;
    movedex[72].effect = "Rest";
    movedex[72].effectchance = 100;
    movedex[72].priority = 0;
    movedex[72].attackorstatus = 1;
    movedex[72].physicalorspecial = 1;
    movedex[72].recoilordrain = 0;
    movedex[72].hitcount = 1;
    movedex[72].target = 1;

    movedex[73].name = "Ice Shard";
    movedex[73].basepower = 40;
    movedex[73].acc = 100;
    movedex[73].type = "Ice";
    movedex[73].typeint = 6;
    movedex[73].effect = "None";
    movedex[73].effectchance = 0;
    movedex[73].priority = 1;
    movedex[73].attackorstatus = 0;
    movedex[73].physicalorspecial = 0;
    movedex[73].recoilordrain = 0;
    movedex[73].hitcount = 1;
    movedex[73].target = 0;

    movedex[74].name = "Stone Edge";
    movedex[74].basepower = 100;
    movedex[74].acc = 80;
    movedex[74].type = "Rock";
    movedex[74].typeint = 13;
    movedex[74].effect = "None";
    movedex[74].effectchance = 0;
    movedex[74].priority = 0;
    movedex[74].attackorstatus = 0;
    movedex[74].physicalorspecial = 0;
    movedex[74].recoilordrain = 0;
    movedex[74].hitcount = 1;
    movedex[74].target = 0;

    movedex[75].name = "Rock Polish";
    movedex[75].basepower = 0;
    movedex[75].acc = 100;
    movedex[75].type = "Rock";
    movedex[75].typeint = 13;
    movedex[75].effect = "+2speed";
    movedex[75].effectchance = 100;
    movedex[75].priority = 0;
    movedex[75].attackorstatus = 1;
    movedex[75].physicalorspecial = 1;
    movedex[75].recoilordrain = 0;
    movedex[75].hitcount = 1;
    movedex[75].target = 1;

    movedex[76].name = "Iron Head";
    movedex[76].basepower = 80;
    movedex[76].acc = 100;
    movedex[76].type = "Steel";
    movedex[76].typeint = 17;
    movedex[76].effect = "Flinch";
    movedex[76].effectchance = 30;
    movedex[76].priority = 0;
    movedex[76].attackorstatus = 0;
    movedex[76].physicalorspecial = 0;
    movedex[76].recoilordrain = 0;
    movedex[76].hitcount = 1;
    movedex[76].target = 0;

    movedex[77].name = "Signal Beam";
    movedex[77].basepower = 75;
    movedex[77].acc = 100;
    movedex[77].type = "Bug";
    movedex[77].typeint = 12;
    movedex[77].effect = "Confuse";
    movedex[77].effectchance = 10;
    movedex[77].priority = 0;
    movedex[77].attackorstatus = 0;
    movedex[77].physicalorspecial = 1;
    movedex[77].recoilordrain = 0;
    movedex[77].hitcount = 1;
    movedex[77].target = 0;

    movedex[78].name = "Blaze Kick";
    movedex[78].basepower = 85;
    movedex[78].acc = 90;
    movedex[78].type = "Fire";
    movedex[78].typeint = 2;
    movedex[78].effect = "Burn";
    movedex[78].effectchance = 10;
    movedex[78].priority = 0;
    movedex[78].attackorstatus = 0;
    movedex[78].physicalorspecial = 0;
    movedex[78].recoilordrain = 0;
    movedex[78].hitcount = 1;
    movedex[78].target = 0;

    movedex[79].name = "Ice Punch";
    movedex[79].basepower = 75;
    movedex[79].acc = 100;
    movedex[79].type = "Ice";
    movedex[79].typeint = 6;
    movedex[79].effect = "Freeze";
    movedex[79].effectchance = 10;
    movedex[79].priority = 0;
    movedex[79].attackorstatus = 0;
    movedex[79].physicalorspecial = 0;
    movedex[79].recoilordrain = 0;
    movedex[79].hitcount = 1;
    movedex[79].target = 0;

    movedex[80].name = "Mach Punch";
    movedex[80].basepower = 40;
    movedex[80].acc = 100;
    movedex[80].type = "Fighting";
    movedex[80].typeint = 7;
    movedex[80].effect = "None";
    movedex[80].effectchance = 0;
    movedex[80].priority = 1;
    movedex[80].attackorstatus = 0;
    movedex[80].physicalorspecial = 0;
    movedex[80].recoilordrain = 0;
    movedex[80].hitcount = 1;
    movedex[80].target = 0;

    movedex[81].name = "Will-o-wisp";
    movedex[81].basepower = 0;
    movedex[81].acc = 75;
    movedex[81].type = "Fire";
    movedex[81].typeint = 2;
    movedex[81].effect = "Burn";
    movedex[81].effectchance = 100;
    movedex[81].priority = 0;
    movedex[81].attackorstatus = 1;
    movedex[81].physicalorspecial = 1;
    movedex[81].recoilordrain = 0;
    movedex[81].hitcount = 1;
    movedex[81].target = 0;

    movedex[82].name = "Softboiled";
    movedex[82].basepower = 0;
    movedex[82].acc = 100;
    movedex[82].type = "Normal";
    movedex[82].typeint = 1;
    movedex[82].effect = "Restore";
    movedex[82].effectchance = 100;
    movedex[82].priority = 0;
    movedex[82].attackorstatus = 1;
    movedex[82].physicalorspecial = 1;
    movedex[82].recoilordrain = 0;
    movedex[82].hitcount = 1;
    movedex[82].target = 1;

    movedex[83].name = "Stun Spore";
    movedex[83].basepower = 0;
    movedex[83].acc = 75;
    movedex[83].type = "Grass";
    movedex[83].typeint = 5;
    movedex[83].effect = "Paralyze";
    movedex[83].effectchance = 100;
    movedex[83].priority = 0;
    movedex[83].attackorstatus = 1;
    movedex[83].physicalorspecial = 1;
    movedex[83].recoilordrain = 0;
    movedex[83].hitcount = 1;
    movedex[83].target = 0;

    movedex[84].name = "Dragon Dance";
    movedex[84].basepower = 0;
    movedex[84].acc = 100;
    movedex[84].type = "Dragon";
    movedex[84].typeint = 15;
    movedex[84].effect = "+atk+speed";
    movedex[84].effectchance = 100;
    movedex[84].priority = 0;
    movedex[84].attackorstatus = 1;
    movedex[84].physicalorspecial = 1;
    movedex[84].recoilordrain = 0;
    movedex[84].hitcount = 1;
    movedex[84].target = 1;

    movedex[85].name = "Waterfall";
    movedex[85].basepower = 80;
    movedex[85].acc = 100;
    movedex[85].type = "Water";
    movedex[85].typeint = 3;
    movedex[85].effect = "Flinch";
    movedex[85].effectchance = 20;
    movedex[85].priority = 0;
    movedex[85].attackorstatus = 0;
    movedex[85].physicalorspecial = 0;
    movedex[85].recoilordrain = 0;
    movedex[85].hitcount = 1;
    movedex[85].target = 0;

    movedex[86].name = "Cross Chop";
    movedex[86].basepower = 100;
    movedex[86].acc = 80;
    movedex[86].type = "Fighting";
    movedex[86].typeint = 7;
    movedex[86].effect = "None";
    movedex[86].effectchance = 0;
    movedex[86].priority = 0;
    movedex[86].attackorstatus = 0;
    movedex[86].physicalorspecial = 0;
    movedex[86].recoilordrain = 0;
    movedex[86].hitcount = 1;
    movedex[86].target = 0;

    movedex[87].name = "Flare Blitz";
    movedex[87].basepower = 120;
    movedex[87].acc = 100;
    movedex[87].type = "Fire";
    movedex[87].typeint = 2;
    movedex[87].effect = "Burn";
    movedex[87].effectchance = 10;
    movedex[87].priority = 0;
    movedex[87].attackorstatus = 0;
    movedex[87].physicalorspecial = 0;
    movedex[87].recoilordrain = -2;
    movedex[87].hitcount = 1;
    movedex[87].target = 0;

    movedex[88].name = "Dragon Claw";
    movedex[88].basepower = 80;
    movedex[88].acc = 100;
    movedex[88].type = "Dragon";
    movedex[88].typeint = 15;
    movedex[88].effect = "None";
    movedex[88].effectchance = 0;
    movedex[88].priority = 0;
    movedex[88].attackorstatus = 0;
    movedex[88].physicalorspecial = 0;
    movedex[88].recoilordrain = 0;
    movedex[88].hitcount = 1;
    movedex[88].target = 0;

    movedex[89].name = "Defense Curl";
    movedex[89].basepower = 0;
    movedex[89].acc = 100;
    movedex[89].type = "Normal";
    movedex[89].typeint = 1;
    movedex[89].effect = "+def";
    movedex[89].effectchance = 100;
    movedex[89].priority = 0;
    movedex[89].attackorstatus = 1;
    movedex[89].physicalorspecial = 1;
    movedex[89].recoilordrain = 0;
    movedex[89].hitcount = 1;
    movedex[89].target = 1;

    movedex[90].name = "Heat Wave";
    movedex[90].basepower = 100;
    movedex[90].acc = 90;
    movedex[90].type = "Fire";
    movedex[90].typeint = 2;
    movedex[90].effect = "Burn";
    movedex[90].effectchance = 10;
    movedex[90].priority = 0;
    movedex[90].attackorstatus = 0;
    movedex[90].physicalorspecial = 1;
    movedex[90].recoilordrain = 0;
    movedex[90].hitcount = 1;
    movedex[90].target = 0;
























    //initialize pokedex-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    for (int i = 0; i < 151; i++)
    {
        pokedex[i].name = "FillerFiller12401940"; //this is jsut so the program doesnt segfault when checking thru pokedex[i].name
    }

    pokedex[2].name = "Venusaur";
    pokedex[2].baseatk = 82;
    pokedex[2].basedef = 83;
    pokedex[2].basespatk = 100;
    pokedex[2].basespdef = 100;
    pokedex[2].basespeed = 80;
    pokedex[2].basehp = 80;
    pokedex[2].growthmod = 3;
    pokedex[2].expyield = 1;
    pokedex[2].evolveat = 101;
    pokedex[2].evolveto = 2;
    pokedex[2].type1 = "Grass";
    pokedex[2].type2 = "Poison";
    pokedex[2].typeint1 = 5;
    pokedex[2].typeint2 = 8;
    pokedex[2].move1 = movedex[28];
    pokedex[2].move2 = movedex[29];
    pokedex[2].move3 = movedex[30];
    pokedex[2].move4 = movedex[31];


    pokedex[5].name = "Charizard";
    pokedex[5].baseatk = 84;
    pokedex[5].basedef = 78;
    pokedex[5].basespatk = 109;
    pokedex[5].basespdef = 85;
    pokedex[5].basespeed = 100;
    pokedex[5].basehp = 78;
    pokedex[5].growthmod = 3;
    pokedex[5].expyield = 1;
    pokedex[5].evolveat = 101;
    pokedex[5].evolveto = 5;
    pokedex[5].type1 = "Fire";
    pokedex[5].type2 = "Flying";
    pokedex[5].typeint1 = 2;
    pokedex[5].typeint2 = 10;
    pokedex[5].move1 = movedex[26];
    pokedex[5].move2 = movedex[32];
    pokedex[5].move3 = movedex[30];
    pokedex[5].move4 = movedex[7]; //33


    pokedex[8].name = "Blastoise";
    pokedex[8].baseatk = 83;
    pokedex[8].basedef = 100;
    pokedex[8].basespatk = 85;
    pokedex[8].basespdef = 105;
    pokedex[8].basespeed = 78;
    pokedex[8].basehp = 79;
    pokedex[8].growthmod = 3;
    pokedex[8].expyield = 1;
    pokedex[8].evolveat = 101;
    pokedex[8].evolveto = 8;
    pokedex[8].type1 = "Water";
    pokedex[8].type2 = "None";
    pokedex[8].typeint1 = 3;
    pokedex[8].typeint2 = 0;
    pokedex[8].move1 = movedex[27];
    pokedex[8].move2 = movedex[34];
    pokedex[8].move3 = movedex[30];
    pokedex[8].move4 = movedex[25];


    pokedex[11].name = "Butterfree";
    pokedex[11].basehp = 60;
    pokedex[11].baseatk = 45;
    pokedex[11].basedef = 50;
    pokedex[11].basespatk = 80;
    pokedex[11].basespdef = 80;
    pokedex[11].basespeed = 70;
    pokedex[11].growthmod = 3;
    pokedex[11].expyield = 1;
    pokedex[11].evolveat = 101;
    pokedex[11].evolveto = 8;
    pokedex[11].type1 = "Bug";
    pokedex[11].type2 = "Flying";
    pokedex[11].typeint1 = 12;
    pokedex[11].typeint2 = 10;
    pokedex[11].move1 =  movedex[35];
    pokedex[11].move2 = movedex[36];
    pokedex[11].move3 = movedex[37];
    pokedex[11].move4 = movedex[38];

    pokedex[14].name = "Beedrill";
    pokedex[14].basehp = 65;
    pokedex[14].baseatk = 80;
    pokedex[14].basedef = 40;
    pokedex[14].basespatk = 45;
    pokedex[14].basespdef = 80;
    pokedex[14].basespeed = 75;
    pokedex[14].growthmod = 11;
    pokedex[14].expyield = 11;
    pokedex[14].evolveat = 11;
    pokedex[14].evolveto = 11;
    pokedex[14].type1 = "Bug";
    pokedex[14].type2 = "Poison";
    pokedex[14].typeint1 = 12;
    pokedex[14].typeint2 = 8;
    pokedex[14].move1 =  movedex[39];
    pokedex[14].move2 = movedex[40];
    pokedex[14].move3 = movedex[31];
    pokedex[14].move4 = movedex[28];

    pokedex[17].name = "Pidgeot";
    pokedex[17].basehp = 83;
    pokedex[17].baseatk = 80;
    pokedex[17].basedef = 75;
    pokedex[17].basespatk = 70;
    pokedex[17].basespdef = 70;
    pokedex[17].basespeed = 91;
    pokedex[17].growthmod = 11;
    pokedex[17].expyield = 11;
    pokedex[17].evolveat = 11;
    pokedex[17].evolveto = 11;
    pokedex[17].type1 = "Normal";
    pokedex[17].type2 = "Flying";
    pokedex[17].typeint1 = 1;
    pokedex[17].typeint2 = 10;
    pokedex[17].move1 =  movedex[25];
    pokedex[17].move2 = movedex[32];
    pokedex[17].move3 = movedex[41];
    pokedex[17].move4 = movedex[42];

    pokedex[19].name = "Raticate";
    pokedex[19].basehp = 55;
    pokedex[19].baseatk = 81;
    pokedex[19].basedef = 60;
    pokedex[19].basespatk = 50;
    pokedex[19].basespdef = 70;
    pokedex[19].basespeed = 97;
    pokedex[19].growthmod = 11;
    pokedex[19].expyield = 11;
    pokedex[19].evolveat = 11;
    pokedex[19].evolveto = 11;
    pokedex[19].type1 = "Normal";
    pokedex[19].type2 = "None";
    pokedex[19].typeint1 = 1;
    pokedex[19].typeint2 = 0;
    pokedex[19].move1 = movedex[25];
    pokedex[19].move2 = movedex[43];
    pokedex[19].move3 = movedex[34];
    pokedex[19].move4 = movedex[44];

    pokedex[21].name = "Fearow";
    pokedex[21].basehp = 65;
    pokedex[21].baseatk = 90;
    pokedex[21].basedef = 65;
    pokedex[21].basespatk = 61;
    pokedex[21].basespdef = 61;
    pokedex[21].basespeed = 100;
    pokedex[21].growthmod = 11;
    pokedex[21].expyield = 11;
    pokedex[21].evolveat = 11;
    pokedex[21].evolveto = 11;
    pokedex[21].type1 = "Normal";
    pokedex[21].type2 = "Flying";
    pokedex[21].typeint1 = 1;
    pokedex[21].typeint2 = 10;
    pokedex[21].move1 = movedex[45];
    pokedex[21].move2 = movedex[25];
    pokedex[21].move3 = movedex[42];
    pokedex[21].move4 = movedex[46];

    pokedex[23].name = "Arbok";
    pokedex[23].basehp = 60;
    pokedex[23].baseatk = 85;
    pokedex[23].basedef = 69;
    pokedex[23].basespatk = 65;
    pokedex[23].basespdef = 79;
    pokedex[23].basespeed = 80;
    pokedex[23].growthmod = 11;
    pokedex[23].expyield = 11;
    pokedex[23].evolveat = 11;
    pokedex[23].evolveto = 11;
    pokedex[23].type1 = "Poison";
    pokedex[23].type2 = "None";
    pokedex[23].typeint1 = 8;
    pokedex[23].typeint2 = 0;
    pokedex[23].move1 = movedex[29];
    pokedex[23].move2 = movedex[30];
    pokedex[23].move3 = movedex[47];
    pokedex[23].move4 = movedex[16];

    pokedex[24].name = "Pikachu";
    pokedex[24].basehp = 35;
    pokedex[24].baseatk = 55;
    pokedex[24].basedef = 30;
    pokedex[24].basespatk = 50;
    pokedex[24].basespdef = 40;
    pokedex[24].basespeed = 90;
    pokedex[24].growthmod = 11;
    pokedex[24].expyield = 11;
    pokedex[24].evolveat = 11;
    pokedex[24].evolveto = 11;
    pokedex[24].type1 = "Electric";
    pokedex[24].type2 = "None";
    pokedex[24].typeint1 = 4;
    pokedex[24].typeint2 = 0;
    pokedex[24].move1 = movedex[43];
    pokedex[24].move2 = movedex[44];
    pokedex[24].move3 = movedex[48];
    pokedex[24].move4 = movedex[49];

    pokedex[25].name = "Raichu";
    pokedex[25].basehp = 60;
    pokedex[25].baseatk = 90;
    pokedex[25].basedef = 55;
    pokedex[25].basespatk = 90;
    pokedex[25].basespdef = 80;
    pokedex[25].basespeed = 100;
    pokedex[25].growthmod = 11;
    pokedex[25].expyield = 11;
    pokedex[25].evolveat = 11;
    pokedex[25].evolveto = 11;
    pokedex[25].type1 = "Electric";
    pokedex[25].type2 = "None";
    pokedex[25].typeint1 = 4;
    pokedex[25].typeint2 = 0;
    pokedex[25].move1 = movedex[43];
    pokedex[25].move2 = movedex[44];
    pokedex[25].move3 = movedex[48];
    pokedex[25].move4 = movedex[49];

    pokedex[27].name = "Sandslash";
    pokedex[27].basehp = 75;
    pokedex[27].baseatk = 100;
    pokedex[27].basedef = 110;
    pokedex[27].basespatk = 45;
    pokedex[27].basespdef = 55;
    pokedex[27].basespeed = 65;
    pokedex[27].growthmod = 11;
    pokedex[27].expyield = 11;
    pokedex[27].evolveat = 11;
    pokedex[27].evolveto = 11;
    pokedex[27].type1 = "Ground";
    pokedex[27].type2 = "None";
    pokedex[27].typeint1 = 9;
    pokedex[27].typeint2 = 0;
    pokedex[27].move1 = movedex[30];
    pokedex[27].move2 = movedex[39];
    pokedex[27].move3 = movedex[50];
    pokedex[27].move4 = movedex[51];

    pokedex[30].name = "Nidoqueen";
    pokedex[30].basehp = 90;
    pokedex[30].baseatk = 82;
    pokedex[30].basedef = 87;
    pokedex[30].basespatk = 75;
    pokedex[30].basespdef = 85;
    pokedex[30].basespeed = 76;
    pokedex[30].growthmod = 11;
    pokedex[30].expyield = 11;
    pokedex[30].evolveat = 11;
    pokedex[30].evolveto = 11;
    pokedex[30].type1 = "Ground";
    pokedex[30].type2 = "Poison";
    pokedex[30].typeint1 = 9;
    pokedex[30].typeint2 = 8;
    pokedex[30].move1 = movedex[30];
    pokedex[30].move2 = movedex[39];
    pokedex[30].move3 = movedex[34];
    pokedex[30].move4 = movedex[43];

    pokedex[33].name = "Nidoking";
    pokedex[33].basehp = 81;
    pokedex[33].baseatk = 92;
    pokedex[33].basedef = 77;
    pokedex[33].basespatk = 85;
    pokedex[33].basespdef = 75;
    pokedex[33].basespeed = 85;
    pokedex[33].growthmod = 11;
    pokedex[33].expyield = 11;
    pokedex[33].evolveat = 11;
    pokedex[33].evolveto = 11;
    pokedex[33].type1 = "Ground";
    pokedex[33].type2 = "Poison";
    pokedex[33].typeint1 = 9;
    pokedex[33].typeint2 = 8;
    pokedex[33].move1 = movedex[30];
    pokedex[33].move2 = movedex[29];
    pokedex[33].move3 = movedex[34];
    pokedex[33].move4 = movedex[43];

    pokedex[35].name = "Clefable";
    pokedex[35].basehp = 95;
    pokedex[35].baseatk = 70;
    pokedex[35].basedef = 73;
    pokedex[35].basespatk = 85;
    pokedex[35].basespdef = 90;
    pokedex[35].basespeed = 60;
    pokedex[35].growthmod = 11;
    pokedex[35].expyield = 11;
    pokedex[35].evolveat = 11;
    pokedex[35].evolveto = 11;
    pokedex[35].type1 = "Normal";
    pokedex[35].type2 = "None";
    pokedex[35].typeint1 = 1;
    pokedex[35].typeint2 = 0;
    pokedex[35].move1 = movedex[43];
    pokedex[35].move2 = movedex[34];
    pokedex[35].move3 = movedex[36];
    pokedex[35].move4 = movedex[16];

    pokedex[37].name = "Ninetails";
    pokedex[37].basehp = 73;
    pokedex[37].baseatk = 76;
    pokedex[37].basedef = 75;
    pokedex[37].basespatk = 81;
    pokedex[37].basespdef = 100;
    pokedex[37].basespeed = 100;
    pokedex[37].growthmod = 11;
    pokedex[37].expyield = 11;
    pokedex[37].evolveat = 11;
    pokedex[37].evolveto = 11;
    pokedex[37].type1 = "Fire";
    pokedex[37].type2 = "None";
    pokedex[37].typeint1 = 2;
    pokedex[37].typeint2 = 0;
    pokedex[37].move1 = movedex[26];
    pokedex[37].move2 = movedex[52];
    pokedex[37].move3 = movedex[49];
    pokedex[37].move4 = movedex[53];

    pokedex[39].name = "Wigglytuff";
    pokedex[39].basehp = 140;
    pokedex[39].baseatk = 70;
    pokedex[39].basedef = 45;
    pokedex[39].basespatk = 75;
    pokedex[39].basespdef = 50;
    pokedex[39].basespeed = 45;
    pokedex[39].growthmod = 11;
    pokedex[39].expyield = 11;
    pokedex[39].evolveat = 11;
    pokedex[39].evolveto = 11;
    pokedex[39].type1 = "Normal";
    pokedex[39].type2 = "None";
    pokedex[39].typeint1 = 1;
    pokedex[39].typeint2 = 0;
    pokedex[39].move1 = movedex[16];
    pokedex[39].move2 = movedex[33];
    pokedex[39].move3 = movedex[15];
    pokedex[39].move4 = movedex[48];

    pokedex[41].name = "Golbat";
    pokedex[41].basehp = 75;
    pokedex[41].baseatk = 80;
    pokedex[41].basedef = 70;
    pokedex[41].basespatk = 65;
    pokedex[41].basespdef = 75;
    pokedex[41].basespeed = 90;
    pokedex[41].growthmod = 11;
    pokedex[41].expyield = 11;
    pokedex[41].evolveat = 11;
    pokedex[41].evolveto = 11;
    pokedex[41].type1 = "Poison";
    pokedex[41].type2 = "Flying";
    pokedex[41].typeint1 = 8;
    pokedex[41].typeint2 = 10;
    pokedex[41].move1 = movedex[32];
    pokedex[41].move2 = movedex[29];
    pokedex[41].move3 = movedex[31];
    pokedex[41].move4 = movedex[42];

    pokedex[44].name = "Vileplume";
    pokedex[44].basehp = 75;
    pokedex[44].baseatk = 80;
    pokedex[44].basedef = 85;
    pokedex[44].basespatk = 100;
    pokedex[44].basespdef = 90;
    pokedex[44].basespeed = 50;
    pokedex[44].growthmod = 11;
    pokedex[44].expyield = 11;
    pokedex[44].evolveat = 11;
    pokedex[44].evolveto = 11;
    pokedex[44].type1 = "Grass";
    pokedex[44].type2 = "Poison";
    pokedex[44].typeint1 = 5;
    pokedex[44].typeint2 = 8;
    pokedex[44].move1 = movedex[28];
    pokedex[44].move2 = movedex[29];
    pokedex[44].move3 = movedex[31];
    pokedex[44].move4 = movedex[32];

    pokedex[46].name = "Parasect";
    pokedex[46].basehp = 60;
    pokedex[46].baseatk = 95;
    pokedex[46].basedef = 80;
    pokedex[46].basespatk = 60;
    pokedex[46].basespdef = 80;
    pokedex[46].basespeed = 30;
    pokedex[46].growthmod = 11;
    pokedex[46].expyield = 11;
    pokedex[46].evolveat = 11;
    pokedex[46].evolveto = 11;
    pokedex[46].type1 = "Bug";
    pokedex[46].type2 = "Grass";
    pokedex[46].typeint1 = 12;
    pokedex[46].typeint2 = 5;
    pokedex[46].move1 = movedex[40];
    pokedex[46].move2 = movedex[52];
    pokedex[46].move3 = movedex[31];
    pokedex[46].move4 = movedex[54];

    pokedex[48].name = "Venomoth";
    pokedex[48].basehp = 70;
    pokedex[48].baseatk = 65;
    pokedex[48].basedef = 60;
    pokedex[48].basespatk = 90;
    pokedex[48].basespdef = 75;
    pokedex[48].basespeed = 90;
    pokedex[48].growthmod = 11;
    pokedex[48].expyield = 11;
    pokedex[48].evolveat = 11;
    pokedex[48].evolveto = 11;
    pokedex[48].type1 = "Bug";
    pokedex[48].type2 = "Poison";
    pokedex[48].typeint1 = 12;
    pokedex[48].typeint2 = 8;
    pokedex[48].move1 = movedex[36];
    pokedex[48].move2 = movedex[29];
    pokedex[48].move3 = movedex[35];
    pokedex[48].move4 = movedex[31];

    pokedex[50].name = "Dugtrio";
    pokedex[50].basehp = 35;
    pokedex[50].baseatk = 80;
    pokedex[50].basedef = 50;
    pokedex[50].basespatk = 50;
    pokedex[50].basespdef = 70;
    pokedex[50].basespeed = 120;
    pokedex[50].growthmod = 11;
    pokedex[50].expyield = 11;
    pokedex[50].evolveat = 11;
    pokedex[50].evolveto = 11;
    pokedex[50].type1 = "Ground";
    pokedex[50].type2 = "None";
    pokedex[50].typeint1 = 9;
    pokedex[50].typeint2 = 0;
    pokedex[50].move1 = movedex[30];
    pokedex[50].move2 = movedex[55];
    pokedex[50].move3 = movedex[25];
    pokedex[50].move4 = movedex[56];

    pokedex[52].name = "Persian";
    pokedex[52].basehp = 65;
    pokedex[52].baseatk = 70;
    pokedex[52].basedef = 60;
    pokedex[52].basespatk = 65;
    pokedex[52].basespdef = 65;
    pokedex[52].basespeed = 115;
    pokedex[52].growthmod = 11;
    pokedex[52].expyield = 11;
    pokedex[52].evolveat = 11;
    pokedex[52].evolveto = 11;
    pokedex[52].type1 = "Normal";
    pokedex[52].type2 = "None";
    pokedex[52].typeint1 = 1;
    pokedex[52].typeint2 = 0;
    pokedex[52].move1 = movedex[16];
    pokedex[52].move2 = movedex[25];
    pokedex[52].move3 = movedex[57];
    pokedex[52].move4 = movedex[43];

    pokedex[54].name = "Golduck";
    pokedex[54].basehp = 80;
    pokedex[54].baseatk = 82;
    pokedex[54].basedef = 78;
    pokedex[54].basespatk = 95;
    pokedex[54].basespdef = 80;
    pokedex[54].basespeed = 85;
    pokedex[54].growthmod = 11;
    pokedex[54].expyield = 11;
    pokedex[54].evolveat = 11;
    pokedex[54].evolveto = 11;
    pokedex[54].type1 = "Water";
    pokedex[54].type2 = "None";
    pokedex[54].typeint1 = 3;
    pokedex[54].typeint2 = 0;
    pokedex[54].move1 = movedex[27];
    pokedex[54].move2 = movedex[34];
    pokedex[54].move3 = movedex[36];
    pokedex[54].move4 = movedex[58];

    pokedex[56].name = "Primeape";
    pokedex[56].basehp = 65;
    pokedex[56].baseatk = 105;
    pokedex[56].basedef = 60;
    pokedex[56].basespatk = 60;
    pokedex[56].basespdef = 70;
    pokedex[56].basespeed = 95;
    pokedex[56].growthmod = 11;
    pokedex[56].expyield = 11;
    pokedex[56].evolveat = 11;
    pokedex[56].evolveto = 11;
    pokedex[56].type1 = "Fighting";
    pokedex[56].type2 = "None";
    pokedex[56].typeint1 = 7;
    pokedex[56].typeint2 = 0;
    pokedex[56].move1 = movedex[59];
    pokedex[56].move2 = movedex[51];
    pokedex[56].move3 = movedex[16];
    pokedex[56].move4 = movedex[60];

    pokedex[58].name = "Arcanine";
    pokedex[58].basehp = 90;
    pokedex[58].baseatk = 110;
    pokedex[58].basedef = 80;
    pokedex[58].basespatk = 100;
    pokedex[58].basespdef = 80;
    pokedex[58].basespeed = 95;
    pokedex[58].growthmod = 11;
    pokedex[58].expyield = 11;
    pokedex[58].evolveat = 11;
    pokedex[58].evolveto = 11;
    pokedex[58].type1 = "Fire";
    pokedex[58].type2 = "None";
    pokedex[58].typeint1 = 2;
    pokedex[58].typeint2 = 0;
    pokedex[58].move1 = movedex[61];
    pokedex[58].move2 = movedex[26];
    pokedex[58].move3 = movedex[16];
    pokedex[58].move4 = movedex[47];

    pokedex[61].name = "Poliwrath";
    pokedex[61].basehp = 90;
    pokedex[61].baseatk = 85;
    pokedex[61].basedef = 95;
    pokedex[61].basespatk = 70;
    pokedex[61].basespdef = 90;
    pokedex[61].basespeed = 70;
    pokedex[61].growthmod = 11;
    pokedex[61].expyield = 11;
    pokedex[61].evolveat = 11;
    pokedex[61].evolveto = 11;
    pokedex[61].type1 = "Water";
    pokedex[61].type2 = "Fighting";
    pokedex[61].typeint1 = 3;
    pokedex[61].typeint2 = 7;
    pokedex[61].move1 = movedex[59];
    pokedex[61].move2 = movedex[27];
    pokedex[61].move3 = movedex[51];
    pokedex[61].move4 = movedex[16];

    pokedex[64].name = "Alakazam";
    pokedex[64].basehp = 55;
    pokedex[64].baseatk = 50;
    pokedex[64].basedef = 45;
    pokedex[64].basespatk = 135;
    pokedex[64].basespdef = 85;
    pokedex[64].basespeed = 120;
    pokedex[64].growthmod = 11;
    pokedex[64].expyield = 11;
    pokedex[64].evolveat = 11;
    pokedex[64].evolveto = 11;
    pokedex[64].type1 = "Psychic";
    pokedex[64].type2 = "None";
    pokedex[64].typeint1 = 11;
    pokedex[64].typeint2 = 0;
    pokedex[64].move1 = movedex[58];
    pokedex[64].move2 = movedex[36];
    pokedex[64].move3 = movedex[62];
    pokedex[64].move4 = movedex[63];

    pokedex[67].name = "Machamp";
    pokedex[67].basehp = 90;
    pokedex[67].baseatk = 130;
    pokedex[67].basedef = 80;
    pokedex[67].basespatk = 65;
    pokedex[67].basespdef = 85;
    pokedex[67].basespeed = 55;
    pokedex[67].growthmod = 11;
    pokedex[67].expyield = 11;
    pokedex[67].evolveat = 11;
    pokedex[67].evolveto = 11;
    pokedex[67].type1 = "Fighting";
    pokedex[67].type2 = "None";
    pokedex[67].typeint1 = 7;
    pokedex[67].typeint2 = 0;
    pokedex[67].move1 = movedex[59];
    pokedex[67].move2 = movedex[51];
    pokedex[67].move3 = movedex[16];
    pokedex[67].move4 = movedex[30];

    pokedex[70].name = "Victreebel";
    pokedex[70].basehp = 80;
    pokedex[70].baseatk = 105;
    pokedex[70].basedef = 65;
    pokedex[70].basespatk = 100;
    pokedex[70].basespdef = 60;
    pokedex[70].basespeed = 70;
    pokedex[70].growthmod = 11;
    pokedex[70].expyield = 11;
    pokedex[70].evolveat = 11;
    pokedex[70].evolveto = 11;
    pokedex[70].type1 = "Grass";
    pokedex[70].type2 = "Poison";
    pokedex[70].typeint1 = 5;
    pokedex[70].typeint2 = 8;
    pokedex[70].move1 = movedex[29];
    pokedex[70].move2 = movedex[52];
    pokedex[70].move3 = movedex[31];
    pokedex[70].move4 = movedex[64];

    pokedex[72].name = "Tentacruel";
    pokedex[72].basehp = 80;
    pokedex[72].baseatk = 70;
    pokedex[72].basedef = 65;
    pokedex[72].basespatk = 80;
    pokedex[72].basespdef = 120;
    pokedex[72].basespeed = 100;
    pokedex[72].growthmod = 11;
    pokedex[72].expyield = 11;
    pokedex[72].evolveat = 11;
    pokedex[72].evolveto = 11;
    pokedex[72].type1 = "Water";
    pokedex[72].type2 = "Poison";
    pokedex[72].typeint1 = 3;
    pokedex[72].typeint2 = 8;
    pokedex[72].move1 = movedex[27];
    pokedex[72].move2 = movedex[34];
    pokedex[72].move3 = movedex[29];
    pokedex[72].move4 = movedex[19];

    pokedex[75].name = "Golem";
    pokedex[75].basehp = 80;
    pokedex[75].baseatk = 110;
    pokedex[75].basedef = 130;
    pokedex[75].basespatk = 55;
    pokedex[75].basespdef = 65;
    pokedex[75].basespeed = 45;
    pokedex[75].growthmod = 11;
    pokedex[75].expyield = 11;
    pokedex[75].evolveat = 11;
    pokedex[75].evolveto = 11;
    pokedex[75].type1 = "Rock";
    pokedex[75].type2 = "Ground";
    pokedex[75].typeint1 = 13;
    pokedex[75].typeint2 = 9;
    pokedex[75].move1 = movedex[30];
    pokedex[75].move2 = movedex[55];
    pokedex[75].move3 = movedex[60];
    pokedex[75].move4 = movedex[65];

    pokedex[77].name = "Rapidash";
    pokedex[77].basehp = 65;
    pokedex[77].baseatk = 100;
    pokedex[77].basedef = 70;
    pokedex[77].basespatk = 80;
    pokedex[77].basespdef = 80;
    pokedex[77].basespeed = 105;
    pokedex[77].growthmod = 11;
    pokedex[77].expyield = 11;
    pokedex[77].evolveat = 11;
    pokedex[77].evolveto = 11;
    pokedex[77].type1 = "Fire";
    pokedex[77].type2 = "None";
    pokedex[77].typeint1 = 2;
    pokedex[77].typeint2 = 0;
    pokedex[77].move1 = movedex[33];
    pokedex[77].move2 = movedex[66];
    pokedex[77].move3 = movedex[25];
    pokedex[77].move4 = movedex[67];

    pokedex[79].name = "Slowbro";
    pokedex[79].basehp = 95;
    pokedex[79].baseatk = 75;
    pokedex[79].basedef = 110;
    pokedex[79].basespatk = 100;
    pokedex[79].basespdef = 80;
    pokedex[79].basespeed = 30;
    pokedex[79].growthmod = 11;
    pokedex[79].expyield = 11;
    pokedex[79].evolveat = 11;
    pokedex[79].evolveto = 11;
    pokedex[79].type1 = "Water";
    pokedex[79].type2 = "Psychic";
    pokedex[79].typeint1 = 3;
    pokedex[79].typeint2 = 11;
    pokedex[79].move1 = movedex[58];
    pokedex[79].move2 = movedex[27];
    pokedex[79].move3 = movedex[36];
    pokedex[79].move4 = movedex[33];

    pokedex[81].name = "Magneton";
    pokedex[81].basehp = 50;
    pokedex[81].baseatk = 60;
    pokedex[81].basedef = 95;
    pokedex[81].basespatk = 120;
    pokedex[81].basespdef = 70;
    pokedex[81].basespeed = 70;
    pokedex[81].growthmod = 11;
    pokedex[81].expyield = 11;
    pokedex[81].evolveat = 11;
    pokedex[81].evolveto = 11;
    pokedex[81].type1 = "Electric";
    pokedex[81].type2 = "Steel";
    pokedex[81].typeint1 = 4;
    pokedex[81].typeint2 = 17;
    pokedex[81].move1 = movedex[43];
    pokedex[81].move2 = movedex[68];
    pokedex[81].move3 = movedex[44];
    pokedex[81].move4 = movedex[69];

    pokedex[82].name = "Farfetch'd";
    pokedex[82].basehp = 52;
    pokedex[82].baseatk = 65;
    pokedex[82].basedef = 55;
    pokedex[82].basespatk = 58;
    pokedex[82].basespdef = 62;
    pokedex[82].basespeed = 60;
    pokedex[82].growthmod = 11;
    pokedex[82].expyield = 11;
    pokedex[82].evolveat = 11;
    pokedex[82].evolveto = 11;
    pokedex[82].type1 = "Normal";
    pokedex[82].type2 = "Flying";
    pokedex[82].typeint1 = 1;
    pokedex[82].typeint2 = 10;
    pokedex[82].move1 = movedex[50];
    pokedex[82].move2 = movedex[42];
    pokedex[82].move3 = movedex[16];
    pokedex[82].move4 = movedex[70];

    pokedex[84].name = "Dodrio";
    pokedex[84].basehp = 60;
    pokedex[84].baseatk = 110;
    pokedex[84].basedef = 70;
    pokedex[84].basespatk = 60;
    pokedex[84].basespdef = 60;
    pokedex[84].basespeed = 100;
    pokedex[84].growthmod = 11;
    pokedex[84].expyield = 11;
    pokedex[84].evolveat = 11;
    pokedex[84].evolveto = 11;
    pokedex[84].type1 = "Normal";
    pokedex[84].type2 = "flying";
    pokedex[84].typeint1 = 1;
    pokedex[84].typeint2 = 10;
    pokedex[84].move1 = movedex[45];
    pokedex[84].move2 = movedex[25];
    pokedex[84].move3 = movedex[42];
    pokedex[84].move4 = movedex[71];

    pokedex[86].name = "Dewgong";
    pokedex[86].basehp = 90;
    pokedex[86].baseatk = 70;
    pokedex[86].basedef = 80;
    pokedex[86].basespatk = 70;
    pokedex[86].basespdef = 95;
    pokedex[86].basespeed = 70;
    pokedex[86].growthmod = 11;
    pokedex[86].expyield = 11;
    pokedex[86].evolveat = 11;
    pokedex[86].evolveto = 11;
    pokedex[86].type1 = "Ice";
    pokedex[86].type2 = "Water";
    pokedex[86].typeint1 = 6;
    pokedex[86].typeint2 = 3;
    pokedex[86].move1 = movedex[27];
    pokedex[86].move2 = movedex[34];
    pokedex[86].move3 = movedex[72];
    pokedex[86].move4 = movedex[73];

    pokedex[88].name = "Muk";
    pokedex[88].basehp = 105;
    pokedex[88].baseatk = 105;
    pokedex[88].basedef = 75;
    pokedex[88].basespatk = 65;
    pokedex[88].basespdef = 100;
    pokedex[88].basespeed = 50;
    pokedex[88].growthmod = 11;
    pokedex[88].expyield = 11;
    pokedex[88].evolveat = 11;
    pokedex[88].evolveto = 11;
    pokedex[88].type1 = "Poison";
    pokedex[88].type2 = "None";
    pokedex[88].typeint1 = 8;
    pokedex[88].typeint2 = 0;
    pokedex[88].move1 = movedex[29];
    pokedex[88].move2 = movedex[33];
    pokedex[88].move3 = movedex[31];
    pokedex[88].move4 = movedex[51];

    pokedex[90].name = "Cloyster";
    pokedex[90].basehp = 50;
    pokedex[90].baseatk = 95;
    pokedex[90].basedef = 180;
    pokedex[90].basespatk = 85;
    pokedex[90].basespdef = 45;
    pokedex[90].basespeed = 70;
    pokedex[90].growthmod = 11;
    pokedex[90].expyield = 11;
    pokedex[90].evolveat = 11;
    pokedex[90].evolveto = 11;
    pokedex[90].type1 = "Water";
    pokedex[90].type2 = "Ice";
    pokedex[90].typeint1 = 3;
    pokedex[90].typeint2 = 6;
    pokedex[90].move1 = movedex[34];
    pokedex[90].move2 = movedex[27];
    pokedex[90].move3 = movedex[73];
    pokedex[90].move4 = movedex[15];

    pokedex[93].name = "Gengar";
    pokedex[93].basehp = 60;
    pokedex[93].baseatk = 65;
    pokedex[93].basedef = 60;
    pokedex[93].basespatk = 130;
    pokedex[93].basespdef = 75;
    pokedex[93].basespeed = 110;
    pokedex[93].growthmod = 11;
    pokedex[93].expyield = 11;
    pokedex[93].evolveat = 11;
    pokedex[93].evolveto = 11;
    pokedex[93].type1 = "Ghost";
    pokedex[93].type2 = "Poison";
    pokedex[93].typeint1 = 14;
    pokedex[93].typeint2 = 8;
    pokedex[93].move1 = movedex[62];
    pokedex[93].move2 = movedex[29];
    pokedex[93].move3 = movedex[43];
    pokedex[93].move4 = movedex[36];

    pokedex[94].name = "Onix";
    pokedex[94].basehp = 35;
    pokedex[94].baseatk = 45;
    pokedex[94].basedef = 160;
    pokedex[94].basespatk = 30;
    pokedex[94].basespdef = 45;
    pokedex[94].basespeed = 70;
    pokedex[94].growthmod = 11;
    pokedex[94].expyield = 11;
    pokedex[94].evolveat = 11;
    pokedex[94].evolveto = 11;
    pokedex[94].type1 = "Rock";
    pokedex[94].type2 = "Ground";
    pokedex[94].typeint1 = 13;
    pokedex[94].typeint2 = 9;
    pokedex[94].move1 = movedex[30];
    pokedex[94].move2 = movedex[74];
    pokedex[94].move3 = movedex[75];
    pokedex[94].move4 = movedex[76];

    pokedex[96].name = "Hypno";
    pokedex[96].basehp = 85;
    pokedex[96].baseatk = 73;
    pokedex[96].basedef = 70;
    pokedex[96].basespatk = 73;
    pokedex[96].basespdef = 115;
    pokedex[96].basespeed = 67;
    pokedex[96].growthmod = 11;
    pokedex[96].expyield = 11;
    pokedex[96].evolveat = 11;
    pokedex[96].evolveto = 11;
    pokedex[96].type1 = "Psychic";
    pokedex[96].type2 = "None";
    pokedex[96].typeint1 = 11;
    pokedex[96].typeint2 = 0;
    pokedex[96].move1 = movedex[36];
    pokedex[96].move2 = movedex[58];
    pokedex[96].move3 = movedex[62];
    pokedex[96].move4 = movedex[44];

    pokedex[98].name = "Kingler";
    pokedex[98].basehp = 55;
    pokedex[98].baseatk = 130;
    pokedex[98].basedef = 115;
    pokedex[98].basespatk = 50;
    pokedex[98].basespdef = 50;
    pokedex[98].basespeed = 75;
    pokedex[98].growthmod = 11;
    pokedex[98].expyield = 11;
    pokedex[98].evolveat = 11;
    pokedex[98].evolveto = 11;
    pokedex[98].type1 = "Water";
    pokedex[98].type2 = "None";
    pokedex[98].typeint1 = 3;
    pokedex[98].typeint2 = 0;
    pokedex[98].move1 = movedex[21];
    pokedex[98].move2 = movedex[7];
    pokedex[98].move3 = movedex[40];
    pokedex[98].move4 = movedex[50];

    pokedex[100].name = "Electrode";
    pokedex[100].basehp = 60;
    pokedex[100].baseatk = 50;
    pokedex[100].basedef = 70;
    pokedex[100].basespatk = 80;
    pokedex[100].basespdef = 80;
    pokedex[100].basespeed = 140;
    pokedex[100].growthmod = 11;
    pokedex[100].expyield = 11;
    pokedex[100].evolveat = 11;
    pokedex[100].evolveto = 11;
    pokedex[100].type1 = "Electric";
    pokedex[100].type2 = "None";
    pokedex[100].typeint1 = 4;
    pokedex[100].typeint2 = 0;
    pokedex[100].move1 = movedex[44];
    pokedex[100].move2 = movedex[43];
    pokedex[100].move3 = movedex[48];
    pokedex[100].move4 = movedex[77];

    pokedex[102].name = "Exeggutor";
    pokedex[102].basehp = 95;
    pokedex[102].baseatk = 95;
    pokedex[102].basedef = 85;
    pokedex[102].basespatk = 125;
    pokedex[102].basespdef = 65;
    pokedex[102].basespeed = 55;
    pokedex[102].growthmod = 11;
    pokedex[102].expyield = 11;
    pokedex[102].evolveat = 11;
    pokedex[102].evolveto = 11;
    pokedex[102].type1 = "Grass";
    pokedex[102].type2 = "Psychic";
    pokedex[102].typeint1 = 5;
    pokedex[102].typeint2 = 11;
    pokedex[102].move1 = movedex[36];
    pokedex[102].move2 = movedex[52];
    pokedex[102].move3 = movedex[37];
    pokedex[102].move4 = movedex[29];

    pokedex[104].name = "Marowak";
    pokedex[104].basehp = 60;
    pokedex[104].baseatk = 80;
    pokedex[104].basedef = 110;
    pokedex[104].basespatk = 50;
    pokedex[104].basespdef = 80;
    pokedex[104].basespeed = 45;
    pokedex[104].growthmod = 11;
    pokedex[104].expyield = 11;
    pokedex[104].evolveat = 11;
    pokedex[104].evolveto = 11;
    pokedex[104].type1 = "Ground";
    pokedex[104].type2 = "None";
    pokedex[104].typeint1 = 9;
    pokedex[104].typeint2 = 0;
    pokedex[104].move1 = movedex[25];
    pokedex[104].move2 = movedex[30];
    pokedex[104].move3 = movedex[50];
    pokedex[104].move4 = movedex[65];

    pokedex[105].name = "Hitmonlee";
    pokedex[105].basehp = 50;
    pokedex[105].baseatk = 120;
    pokedex[105].basedef = 53;
    pokedex[105].basespatk = 35;
    pokedex[105].basespdef = 110;
    pokedex[105].basespeed = 87;
    pokedex[105].growthmod = 11;
    pokedex[105].expyield = 11;
    pokedex[105].evolveat = 11;
    pokedex[105].evolveto = 11;
    pokedex[105].type1 = "Fighting";
    pokedex[105].type2 = "None";
    pokedex[105].typeint1 = 7;
    pokedex[105].typeint2 = 0;
    pokedex[105].move1 = movedex[59];
    pokedex[105].move2 = movedex[30];
    pokedex[105].move3 = movedex[78];
    pokedex[105].move4 = movedex[51];

    pokedex[106].name = "Hitmonchan";
    pokedex[106].basehp = 50;
    pokedex[106].baseatk = 105;
    pokedex[106].basedef = 79;
    pokedex[106].basespatk = 35;
    pokedex[106].basespdef = 110;
    pokedex[106].basespeed = 76;
    pokedex[106].growthmod = 11;
    pokedex[106].expyield = 11;
    pokedex[106].evolveat = 11;
    pokedex[106].evolveto = 11;
    pokedex[106].type1 = "Fighting";
    pokedex[106].type2 = "None";
    pokedex[106].typeint1 = 7;
    pokedex[106].typeint2 = 0;
    pokedex[106].move1 = movedex[59];
    pokedex[106].move2 = movedex[65];
    pokedex[106].move3 = movedex[79];
    pokedex[106].move4 = movedex[80];

    pokedex[107].name = "Lickitung";
    pokedex[107].basehp = 90;
    pokedex[107].baseatk = 55;
    pokedex[107].basedef = 75;
    pokedex[107].basespatk = 60;
    pokedex[107].basespdef = 75;
    pokedex[107].basespeed = 30;
    pokedex[107].growthmod = 11;
    pokedex[107].expyield = 11;
    pokedex[107].evolveat = 11;
    pokedex[107].evolveto = 11;
    pokedex[107].type1 = "Normal";
    pokedex[107].type2 = "None";
    pokedex[107].typeint1 = 1;
    pokedex[107].typeint2 = 0;
    pokedex[107].move1 = movedex[25];
    pokedex[107].move2 = movedex[30];
    pokedex[107].move3 = movedex[16];
    pokedex[107].move4 = movedex[33];

    pokedex[109].name = "Weezing";
    pokedex[109].basehp = 65;
    pokedex[109].baseatk = 90;
    pokedex[109].basedef = 120;
    pokedex[109].basespatk = 85;
    pokedex[109].basespdef = 70;
    pokedex[109].basespeed = 60;
    pokedex[109].growthmod = 11;
    pokedex[109].expyield = 11;
    pokedex[109].evolveat = 11;
    pokedex[109].evolveto = 11;
    pokedex[109].type1 = "Poison";
    pokedex[109].type2 = "None";
    pokedex[109].typeint1 = 8;
    pokedex[109].typeint2 = 0;
    pokedex[109].move1 = movedex[29];
    pokedex[109].move2 = movedex[33];
    pokedex[109].move3 = movedex[31];
    pokedex[109].move4 = movedex[81];

    pokedex[111].name = "Rhydon";
    pokedex[111].basehp = 105;
    pokedex[111].baseatk = 130;
    pokedex[111].basedef = 120;
    pokedex[111].basespatk = 45;
    pokedex[111].basespdef = 45;
    pokedex[111].basespeed = 40;
    pokedex[111].growthmod = 11;
    pokedex[111].expyield = 11;
    pokedex[111].evolveat = 11;
    pokedex[111].evolveto = 11;
    pokedex[111].type1 = "Rock";
    pokedex[111].type2 = "Ground";
    pokedex[111].typeint1 = 13;
    pokedex[111].typeint2 = 9;
    pokedex[111].move1 = movedex[30];
    pokedex[111].move2 = movedex[74];
    pokedex[111].move3 = movedex[75];
    pokedex[111].move4 = movedex[79];

    pokedex[112].name = "Chansey";
    pokedex[112].basehp = 250;
    pokedex[112].baseatk = 5;
    pokedex[112].basedef = 5;
    pokedex[112].basespatk = 35;
    pokedex[112].basespdef = 105;
    pokedex[112].basespeed = 50;
    pokedex[112].growthmod = 11;
    pokedex[112].expyield = 11;
    pokedex[112].evolveat = 11;
    pokedex[112].evolveto = 11;
    pokedex[112].type1 = "Normal";
    pokedex[112].type2 = "None";
    pokedex[112].typeint1 = 1;
    pokedex[112].typeint2 = 0;
    pokedex[112].move1 = movedex[82];
    pokedex[112].move2 = movedex[33];
    pokedex[112].move3 = movedex[31];
    pokedex[112].move4 = movedex[43];

    pokedex[113].name = "Tangela";
    pokedex[113].basehp = 65;
    pokedex[113].baseatk = 55;
    pokedex[113].basedef = 115;
    pokedex[113].basespatk = 100;
    pokedex[113].basespdef = 40;
    pokedex[113].basespeed = 60;
    pokedex[113].growthmod = 11;
    pokedex[113].expyield = 11;
    pokedex[113].evolveat = 11;
    pokedex[113].evolveto = 11;
    pokedex[113].type1 = "Grass";
    pokedex[113].type2 = "None";
    pokedex[113].typeint1 = 5;
    pokedex[113].typeint2 = 0;
    pokedex[113].move1 = movedex[28];
    pokedex[113].move2 = movedex[29];
    pokedex[113].move3 = movedex[37];
    pokedex[113].move4 = movedex[83];

    pokedex[114].name = "Kangaskhan";
    pokedex[114].basehp = 105;
    pokedex[114].baseatk = 95;
    pokedex[114].basedef = 80;
    pokedex[114].basespatk = 40;
    pokedex[114].basespdef = 80;
    pokedex[114].basespeed = 90;
    pokedex[114].growthmod = 11;
    pokedex[114].expyield = 11;
    pokedex[114].evolveat = 11;
    pokedex[114].evolveto = 11;
    pokedex[114].type1 = "Normal";
    pokedex[114].type2 = "None";
    pokedex[114].typeint1 = 1;
    pokedex[114].typeint2 = 0;
    pokedex[114].move1 = movedex[25];
    pokedex[114].move2 = movedex[30];
    pokedex[114].move3 = movedex[60];
    pokedex[114].move4 = movedex[65];

    pokedex[116].name = "Seadra";
    pokedex[116].basehp = 55;
    pokedex[116].baseatk = 65;
    pokedex[116].basedef = 95;
    pokedex[116].basespatk = 95;
    pokedex[116].basespdef = 45;
    pokedex[116].basespeed = 85;
    pokedex[116].growthmod = 11;
    pokedex[116].expyield = 11;
    pokedex[116].evolveat = 11;
    pokedex[116].evolveto = 11;
    pokedex[116].type1 = "Water";
    pokedex[116].type2 = "None";
    pokedex[116].typeint1 = 3;
    pokedex[116].typeint2 = 0;
    pokedex[116].move1 = movedex[84];
    pokedex[116].move2 = movedex[85];
    pokedex[116].move3 = movedex[25];
    pokedex[116].move4 = movedex[27];

    pokedex[118].name = "Seaking";
    pokedex[118].basehp = 80;
    pokedex[118].baseatk = 92;
    pokedex[118].basedef = 65;
    pokedex[118].basespatk = 65;
    pokedex[118].basespdef = 80;
    pokedex[118].basespeed = 68;
    pokedex[118].growthmod = 11;
    pokedex[118].expyield = 11;
    pokedex[118].evolveat = 11;
    pokedex[118].evolveto = 11;
    pokedex[118].type1 = "Water";
    pokedex[118].type2 = "None";
    pokedex[118].typeint1 = 3;
    pokedex[118].typeint2 = 0;
    pokedex[118].move1 = movedex[85];
    pokedex[118].move2 = movedex[25];
    pokedex[118].move3 = movedex[46];
    pokedex[118].move4 = movedex[66];

    pokedex[120].name = "Starmie";
    pokedex[120].basehp = 60;
    pokedex[120].baseatk = 75;
    pokedex[120].basedef = 85;
    pokedex[120].basespatk = 100;
    pokedex[120].basespdef = 85;
    pokedex[120].basespeed = 115;
    pokedex[120].growthmod = 11;
    pokedex[120].expyield = 11;
    pokedex[120].evolveat = 11;
    pokedex[120].evolveto = 11;
    pokedex[120].type1 = "Water";
    pokedex[120].type2 = "Psychic";
    pokedex[120].typeint1 = 3;
    pokedex[120].typeint2 = 11;
    pokedex[120].move1 = movedex[36];
    pokedex[120].move2 = movedex[27];
    pokedex[120].move3 = movedex[43];
    pokedex[120].move4 = movedex[34];

    pokedex[121].name = "Mr. Mime";
    pokedex[121].basehp = 40;
    pokedex[121].baseatk = 45;
    pokedex[121].basedef = 65;
    pokedex[121].basespatk = 100;
    pokedex[121].basespdef = 120;
    pokedex[121].basespeed = 90;
    pokedex[121].growthmod = 11;
    pokedex[121].expyield = 11;
    pokedex[121].evolveat = 11;
    pokedex[121].evolveto = 11;
    pokedex[121].type1 = "Psychic";
    pokedex[121].type2 = "None";
    pokedex[121].typeint1 = 11;
    pokedex[121].typeint2 = 0;
    pokedex[121].move1 = movedex[36];
    pokedex[121].move2 = movedex[11];
    pokedex[121].move3 = movedex[58];
    pokedex[121].move4 = movedex[43];

    pokedex[122].name = "Scyther";
    pokedex[122].basehp = 70;
    pokedex[122].baseatk = 110;
    pokedex[122].basedef = 80;
    pokedex[122].basespatk = 55;
    pokedex[122].basespdef = 80;
    pokedex[122].basespeed = 105;
    pokedex[122].growthmod = 11;
    pokedex[122].expyield = 11;
    pokedex[122].evolveat = 11;
    pokedex[122].evolveto = 11;
    pokedex[122].type1 = "Bug";
    pokedex[122].type2 = "Flying";
    pokedex[122].typeint1 = 12;
    pokedex[122].typeint2 = 10;
    pokedex[122].move1 = movedex[50];
    pokedex[122].move2 = movedex[46];
    pokedex[122].move3 = movedex[40];
    pokedex[122].move4 = movedex[25];

    pokedex[123].name = "Jynx";
    pokedex[123].basehp = 65;
    pokedex[123].baseatk = 50;
    pokedex[123].basedef = 35;
    pokedex[123].basespatk = 115;
    pokedex[123].basespdef = 95;
    pokedex[123].basespeed = 95;
    pokedex[123].growthmod = 11;
    pokedex[123].expyield = 11;
    pokedex[123].evolveat = 11;
    pokedex[123].evolveto = 11;
    pokedex[123].type1 = "Psychic";
    pokedex[123].type2 = "Ice";
    pokedex[123].typeint1 = 11;
    pokedex[123].typeint2 = 6;
    pokedex[123].move1 = movedex[36];
    pokedex[123].move2 = movedex[34];
    pokedex[123].move3 = movedex[49];
    pokedex[123].move4 = movedex[62];

    pokedex[124].name = "Electabuzz";
    pokedex[124].basehp = 65;
    pokedex[124].baseatk = 83;
    pokedex[124].basedef = 57;
    pokedex[124].basespatk = 95;
    pokedex[124].basespdef = 85;
    pokedex[124].basespeed = 105;
    pokedex[124].growthmod = 11;
    pokedex[124].expyield = 11;
    pokedex[124].evolveat = 11;
    pokedex[124].evolveto = 11;
    pokedex[124].type1 = "Electric";
    pokedex[124].type2 = "None";
    pokedex[124].typeint1 = 4;
    pokedex[124].typeint2 = 0;
    pokedex[124].move1 = movedex[65];
    pokedex[124].move2 = movedex[43];
    pokedex[124].move3 = movedex[16];
    pokedex[124].move4 = movedex[86];

    pokedex[125].name = "Magmar";
    pokedex[125].basehp = 65;
    pokedex[125].baseatk = 95;
    pokedex[125].basedef = 57;
    pokedex[125].basespatk = 100;
    pokedex[125].basespdef = 85;
    pokedex[125].basespeed = 93;
    pokedex[125].growthmod = 11;
    pokedex[125].expyield = 11;
    pokedex[125].evolveat = 11;
    pokedex[125].evolveto = 11;
    pokedex[125].type1 = "Fire";
    pokedex[125].type2 = "None";
    pokedex[125].typeint1 = 2;
    pokedex[125].typeint2 = 0;
    pokedex[125].move1 = movedex[33];
    pokedex[125].move2 = movedex[16];
    pokedex[125].move3 = movedex[65];
    pokedex[125].move4 = movedex[87];

    pokedex[126].name = "Pinsir";
    pokedex[126].basehp = 65;
    pokedex[126].baseatk = 125;
    pokedex[126].basedef = 100;
    pokedex[126].basespatk = 55;
    pokedex[126].basespdef = 70;
    pokedex[126].basespeed = 85;
    pokedex[126].growthmod = 11;
    pokedex[126].expyield = 11;
    pokedex[126].evolveat = 11;
    pokedex[126].evolveto = 11;
    pokedex[126].type1 = "Bug";
    pokedex[126].type2 = "None";
    pokedex[126].typeint1 = 12;
    pokedex[126].typeint2 = 0;
    pokedex[126].move1 = movedex[40];
    pokedex[126].move2 = movedex[50];
    pokedex[126].move3 = movedex[30];
    pokedex[126].move4 = movedex[25];

    pokedex[127].name = "Tauros";
    pokedex[127].basehp = 75;
    pokedex[127].baseatk = 100;
    pokedex[127].basedef = 95;
    pokedex[127].basespatk = 40;
    pokedex[127].basespdef = 70;
    pokedex[127].basespeed = 110;
    pokedex[127].growthmod = 11;
    pokedex[127].expyield = 11;
    pokedex[127].evolveat = 11;
    pokedex[127].evolveto = 11;
    pokedex[127].type1 = "Normal";
    pokedex[127].type2 = "None";
    pokedex[127].typeint1 = 1;
    pokedex[127].typeint2 = 0;
    pokedex[127].move1 = movedex[25];
    pokedex[127].move2 = movedex[30];
    pokedex[127].move3 = movedex[34];
    pokedex[127].move4 = movedex[74];

    pokedex[129].name = "Gyarados";
    pokedex[129].basehp = 95;
    pokedex[129].baseatk = 125;
    pokedex[129].basedef = 79;
    pokedex[129].basespatk = 60;
    pokedex[129].basespdef = 100;
    pokedex[129].basespeed = 81;
    pokedex[129].growthmod = 11;
    pokedex[129].expyield = 11;
    pokedex[129].evolveat = 11;
    pokedex[129].evolveto = 11;
    pokedex[129].type1 = "Water";
    pokedex[129].type2 = "Flying";
    pokedex[129].typeint1 = 3;
    pokedex[129].typeint2 = 10;
    pokedex[129].move1 = movedex[27];
    pokedex[129].move2 = movedex[34];
    pokedex[129].move3 = movedex[47];
    pokedex[129].move4 = movedex[30];

    pokedex[130].name = "Lapras";
    pokedex[130].basehp = 130;
    pokedex[130].baseatk = 85;
    pokedex[130].basedef = 80;
    pokedex[130].basespatk = 85;
    pokedex[130].basespdef = 95;
    pokedex[130].basespeed = 60;
    pokedex[130].growthmod = 11;
    pokedex[130].expyield = 11;
    pokedex[130].evolveat = 11;
    pokedex[130].evolveto = 11;
    pokedex[130].type1 = "Water";
    pokedex[130].type2 = "Ice";
    pokedex[130].typeint1 = 3;
    pokedex[130].typeint2 = 6;
    pokedex[130].move1 = movedex[27];
    pokedex[130].move2 = movedex[34];
    pokedex[130].move3 = movedex[36];
    pokedex[130].move4 = movedex[43];

    pokedex[133].name = "Vaporeon";
    pokedex[133].basehp = 130;
    pokedex[133].baseatk = 65;
    pokedex[133].basedef = 60;
    pokedex[133].basespatk = 110;
    pokedex[133].basespdef = 95;
    pokedex[133].basespeed = 65;
    pokedex[133].growthmod = 11;
    pokedex[133].expyield = 11;
    pokedex[133].evolveat = 11;
    pokedex[133].evolveto = 11;
    pokedex[133].type1 = "Water";
    pokedex[133].type2 = "None";
    pokedex[133].typeint1 = 3;
    pokedex[133].typeint2 = 0;
    pokedex[133].move1 = movedex[27];
    pokedex[133].move2 = movedex[34];
    pokedex[133].move3 = movedex[6];
    pokedex[133].move4 = movedex[72];

    pokedex[134].name = "Jolteon";
    pokedex[134].basehp = 65;
    pokedex[134].baseatk = 65;
    pokedex[134].basedef = 60;
    pokedex[134].basespatk = 110;
    pokedex[134].basespdef = 95;
    pokedex[134].basespeed = 130;
    pokedex[134].growthmod = 11;
    pokedex[134].expyield = 11;
    pokedex[134].evolveat = 11;
    pokedex[134].evolveto = 11;
    pokedex[134].type1 = "Electric";
    pokedex[134].type2 = "None";
    pokedex[134].typeint1 = 4;
    pokedex[134].typeint2 = 0;
    pokedex[134].move1 = movedex[43];
    pokedex[134].move2 = movedex[44];
    pokedex[134].move3 = movedex[48];
    pokedex[134].move4 = movedex[62];

    pokedex[135].name = "Flareon";
    pokedex[135].basehp = 65;
    pokedex[135].baseatk = 130;
    pokedex[135].basedef = 60;
    pokedex[135].basespatk = 95;
    pokedex[135].basespdef = 110;
    pokedex[135].basespeed = 65;
    pokedex[135].growthmod = 11;
    pokedex[135].expyield = 11;
    pokedex[135].evolveat = 11;
    pokedex[135].evolveto = 11;
    pokedex[135].type1 = "Fire";
    pokedex[135].type2 = "None";
    pokedex[135].typeint1 = 2;
    pokedex[135].typeint2 = 0;
    pokedex[135].move1 = movedex[87];
    pokedex[135].move2 = movedex[25];
    pokedex[135].move3 = movedex[81];
    pokedex[135].move4 = movedex[33];

    pokedex[136].name = "Porygon";
    pokedex[136].basehp = 65;
    pokedex[136].baseatk = 60;
    pokedex[136].basedef = 70;
    pokedex[136].basespatk = 85;
    pokedex[136].basespdef = 75;
    pokedex[136].basespeed = 40;
    pokedex[136].growthmod = 11;
    pokedex[136].expyield = 11;
    pokedex[136].evolveat = 11;
    pokedex[136].evolveto = 11;
    pokedex[136].type1 = "Normal";
    pokedex[136].type2 = "None";
    pokedex[136].typeint1 = 1;
    pokedex[136].typeint2 = 0;
    pokedex[136].move1 = movedex[15];
    pokedex[136].move2 = movedex[48];
    pokedex[136].move3 = movedex[25];
    pokedex[136].move4 = movedex[36];

    pokedex[138].name = "Omastar";
    pokedex[138].basehp = 70;
    pokedex[138].baseatk = 60;
    pokedex[138].basedef = 125;
    pokedex[138].basespatk = 115;
    pokedex[138].basespdef = 70;
    pokedex[138].basespeed = 55;
    pokedex[138].growthmod = 11;
    pokedex[138].expyield = 11;
    pokedex[138].evolveat = 11;
    pokedex[138].evolveto = 11;
    pokedex[138].type1 = "Water";
    pokedex[138].type2 = "Rock";
    pokedex[138].typeint1 = 3;
    pokedex[138].typeint2 = 13;
    pokedex[138].move1 = movedex[27];
    pokedex[138].move2 = movedex[74];
    pokedex[138].move3 = movedex[15];
    pokedex[138].move4 = movedex[75];

    pokedex[140].name = "Kabutops";
    pokedex[140].basehp = 60;
    pokedex[140].baseatk = 115;
    pokedex[140].basedef = 105;
    pokedex[140].basespatk = 65;
    pokedex[140].basespdef = 70;
    pokedex[140].basespeed = 80;
    pokedex[140].growthmod = 11;
    pokedex[140].expyield = 11;
    pokedex[140].evolveat = 11;
    pokedex[140].evolveto = 11;
    pokedex[140].type1 = "Water";
    pokedex[140].type2 = "Rock";
    pokedex[140].typeint1 = 3;
    pokedex[140].typeint2 = 13;
    pokedex[140].move1 = movedex[85];
    pokedex[140].move2 = movedex[74];
    pokedex[140].move3 = movedex[50];
    pokedex[140].move4 = movedex[75];

    pokedex[141].name = "Aerodactyl";
    pokedex[141].basehp = 80;
    pokedex[141].baseatk = 105;
    pokedex[141].basedef = 65;
    pokedex[141].basespatk = 60;
    pokedex[141].basespdef = 75;
    pokedex[141].basespeed = 130;
    pokedex[141].growthmod = 11;
    pokedex[141].expyield = 11;
    pokedex[141].evolveat = 11;
    pokedex[141].evolveto = 11;
    pokedex[141].type1 = "Rock";
    pokedex[141].type2 = "Flying";
    pokedex[141].typeint1 = 13;
    pokedex[141].typeint2 = 10;
    pokedex[141].move1 = movedex[74];
    pokedex[141].move2 = movedex[30];
    pokedex[141].move3 = movedex[88];
    pokedex[141].move4 = movedex[47];

    pokedex[142].name = "Snorlax";
    pokedex[142].basehp = 160;
    pokedex[142].baseatk = 110;
    pokedex[142].basedef = 65;
    pokedex[142].basespatk = 65;
    pokedex[142].basespdef = 110;
    pokedex[142].basespeed = 30;
    pokedex[142].growthmod = 11;
    pokedex[142].expyield = 11;
    pokedex[142].evolveat = 11;
    pokedex[142].evolveto = 11;
    pokedex[142].type1 = "Normal";
    pokedex[142].type2 = "None";
    pokedex[142].typeint1 = 1;
    pokedex[142].typeint2 = 0;
    pokedex[142].move1 = movedex[16];
    pokedex[142].move2 = movedex[89];
    pokedex[142].move3 = movedex[72];
    pokedex[142].move4 = movedex[47];

    pokedex[143].name = "Articuno";
    pokedex[143].basehp = 90;
    pokedex[143].baseatk = 85;
    pokedex[143].basedef = 100;
    pokedex[143].basespatk = 95;
    pokedex[143].basespdef = 125;
    pokedex[143].basespeed = 85;
    pokedex[143].growthmod = 11;
    pokedex[143].expyield = 11;
    pokedex[143].evolveat = 11;
    pokedex[143].evolveto = 11;
    pokedex[143].type1 = "Ice";
    pokedex[143].type2 = "Flying";
    pokedex[143].typeint1 = 6;
    pokedex[143].typeint2 = 10;
    pokedex[143].move1 = movedex[15];
    pokedex[143].move2 = movedex[73];
    pokedex[143].move3 = movedex[42];
    pokedex[143].move4 = movedex[31];

    pokedex[144].name = "Zapdos";
    pokedex[144].basehp = 90;
    pokedex[144].baseatk = 90;
    pokedex[144].basedef = 85;
    pokedex[144].basespatk = 125;
    pokedex[144].basespdef = 90;
    pokedex[144].basespeed = 100;
    pokedex[144].growthmod = 11;
    pokedex[144].expyield = 11;
    pokedex[144].evolveat = 11;
    pokedex[144].evolveto = 11;
    pokedex[144].type1 = "Electric";
    pokedex[144].type2 = "Flying";
    pokedex[144].typeint1 = 4;
    pokedex[144].typeint2 = 10;
    pokedex[144].move1 = movedex[45];
    pokedex[144].move2 = movedex[43];
    pokedex[144].move3 = movedex[42];
    pokedex[144].move4 = movedex[90];

    pokedex[145].name = "Moltres";
    pokedex[145].basehp = 90;
    pokedex[145].baseatk = 100;
    pokedex[145].basedef = 90;
    pokedex[145].basespatk = 125;
    pokedex[145].basespdef = 85;
    pokedex[145].basespeed = 90;
    pokedex[145].growthmod = 11;
    pokedex[145].expyield = 11;
    pokedex[145].evolveat = 11;
    pokedex[145].evolveto = 11;
    pokedex[145].type1 = "Fire";
    pokedex[145].type2 = "Flying";
    pokedex[145].typeint1 = 2;
    pokedex[145].typeint2 = 10;
    pokedex[145].move1 = movedex[33];
    pokedex[145].move2 = movedex[42];
    pokedex[145].move3 = movedex[32];
    pokedex[145].move4 = movedex[81];

    pokedex[148].name = "Dragonite";
    pokedex[148].basehp = 91;
    pokedex[148].baseatk = 134;
    pokedex[148].basedef = 95;
    pokedex[148].basespatk = 100;
    pokedex[148].basespdef = 100;
    pokedex[148].basespeed = 80;
    pokedex[148].growthmod = 11;
    pokedex[148].expyield = 11;
    pokedex[148].evolveat = 11;
    pokedex[148].evolveto = 11;
    pokedex[148].type1 = "Dragon";
    pokedex[148].type2 = "Flying";
    pokedex[148].typeint1 = 15;
    pokedex[148].typeint2 = 10;
    pokedex[148].move1 = movedex[84];
    pokedex[148].move2 = movedex[88];
    pokedex[148].move3 = movedex[30];
    pokedex[148].move4 = movedex[60];

    pokedex[149].name = "Mewtwo";
    pokedex[149].basehp = 106;
    pokedex[149].baseatk = 110;
    pokedex[149].basedef = 90;
    pokedex[149].basespatk = 154;
    pokedex[149].basespdef = 90;
    pokedex[149].basespeed = 130;
    pokedex[149].growthmod = 11;
    pokedex[149].expyield = 11;
    pokedex[149].evolveat = 11;
    pokedex[149].evolveto = 11;
    pokedex[149].type1 = "Psychic";
    pokedex[149].type2 = "None";
    pokedex[149].typeint1 = 11;
    pokedex[149].typeint2 = 0;
    pokedex[149].move1 = movedex[58];
    pokedex[149].move2 = movedex[36];
    pokedex[149].move3 = movedex[43];
    pokedex[149].move4 = movedex[34];

    pokedex[150].name = "Mew";
    pokedex[150].basehp = 100;
    pokedex[150].baseatk = 100;
    pokedex[150].basedef = 100;
    pokedex[150].basespatk = 100;
    pokedex[150].basespdef = 100;
    pokedex[150].basespeed = 100;
    pokedex[150].growthmod = 11;
    pokedex[150].expyield = 11;
    pokedex[150].evolveat = 11;
    pokedex[150].evolveto = 11;
    pokedex[150].type1 = "Psychic";
    pokedex[150].type2 = "None";
    pokedex[150].typeint1 = 11;
    pokedex[150].typeint2 = 0;
    pokedex[150].move1 = movedex[49];
    pokedex[150].move2 = movedex[63];
    pokedex[150].move3 = movedex[36];
    pokedex[150].move4 = movedex[33];












    //assign type strings to integers, then initialize typechart doublearray int typeeffectiveness[17][17]---------------------------------------------------------------------------------------------

    //no type 0
    //normal 1
    //fire 2
    //water 3
    //electric 4
    //grass 5
    //ice 6
    //fighting 7
    //poison 8
    //ground 9
    //flying 10
    //psychic 11
    //bug 12
    //rock 13
    //ghost 14
    //dragon 15
    //dark 16
    //steel 17
    //typechart[attackingtype][defendingtype] is how this works
    //format for typechart will be integer from 0 thru 4. 0 is no effect, 1 not very effective, 2 normal, 4 super effective

//    int typechart[18][18]; //seventeen types plus null type at 0

    typechart[0][0] = 2;
    typechart[0][1] = 2;
    typechart[0][2] = 2;
    typechart[0][3] = 2;
    typechart[0][4] = 2;
    typechart[0][4] = 2;
    typechart[0][5] = 2;
    typechart[0][6] = 2;
    typechart[0][7] = 2;
    typechart[0][8] = 2;
    typechart[0][9] = 2;
    typechart[0][10] = 2;
    typechart[0][11] = 2;
    typechart[0][12] = 2;
    typechart[0][13] = 2;
    typechart[0][14] = 2;
    typechart[0][15] = 2;
    typechart[0][16] = 2;
    typechart[0][17] = 2;
    typechart[1][0] = 2;
    typechart[2][0] = 2;
    typechart[3][0] = 2;
    typechart[4][0] = 2;
    typechart[5][0] = 2;
    typechart[6][0] = 2;
    typechart[7][0] = 2;
    typechart[8][0] = 2;
    typechart[9][0] = 2;
    typechart[10][0] = 2;
    typechart[11][0] = 2;
    typechart[12][0] = 2;
    typechart[13][0] = 2;
    typechart[14][0] = 2;
    typechart[15][0] = 2;
    typechart[16][0] = 2;
    typechart[17][0] = 2;

    typechart[1][1] = 2;
    typechart[1][2] = 2;
    typechart[1][3] = 2;
    typechart[1][4] = 2;
    typechart[1][5] = 2;
    typechart[1][6] = 2;
    typechart[1][7] = 2;
    typechart[1][8] = 2;
    typechart[1][9] = 2;
    typechart[1][10] = 2;
    typechart[1][11] = 2;
    typechart[1][12] = 2;
    typechart[1][13] = 1;
    typechart[1][14] = 0;
    typechart[1][15] = 2;
    typechart[1][16] = 2;
    typechart[1][17] = 1;

    typechart[2][1] = 2;
    typechart[2][2] = 1;
    typechart[2][3] = 1;
    typechart[2][4] = 2;
    typechart[2][5] = 4;
    typechart[2][6] = 4;
    typechart[2][7] = 2;
    typechart[2][8] = 2;
    typechart[2][9] = 2;
    typechart[2][10] = 2;
    typechart[2][11] = 2;
    typechart[2][12] = 4;
    typechart[2][13] = 1;
    typechart[2][14] = 2;
    typechart[2][15] = 1;
    typechart[2][16] = 2;
    typechart[2][17] = 4;

    typechart[3][1] = 2;
    typechart[3][2] = 4;
    typechart[3][3] = 1;
    typechart[3][4] = 2;
    typechart[3][5] = 1;
    typechart[3][6] = 2;
    typechart[3][7] = 2;
    typechart[3][8] = 2;
    typechart[3][9] = 4;
    typechart[3][10] = 2;
    typechart[3][11] = 2;
    typechart[3][12] = 2;
    typechart[3][13] = 4;
    typechart[3][14] = 2;
    typechart[3][15] = 1;
    typechart[3][16] = 2;
    typechart[3][17] = 2;

    typechart[4][1] = 2;
    typechart[4][2] = 2;
    typechart[4][3] = 4;
    typechart[4][4] = 1;
    typechart[4][5] = 1;
    typechart[4][6] = 2;
    typechart[4][7] = 2;
    typechart[4][8] = 2;
    typechart[4][9] = 0;
    typechart[4][10] = 4;
    typechart[4][11] = 2;
    typechart[4][12] = 2;
    typechart[4][13] = 2;
    typechart[4][14] = 2;
    typechart[4][15] = 1;
    typechart[4][16] = 2;
    typechart[4][17] = 2;

    typechart[5][1] = 2;
    typechart[5][2] = 1;
    typechart[5][3] = 4;
    typechart[5][4] = 2;
    typechart[5][5] = 1;
    typechart[5][6] = 2;
    typechart[5][7] = 2;
    typechart[5][8] = 1;
    typechart[5][9] = 4;
    typechart[5][10] = 1;
    typechart[5][11] = 2;
    typechart[5][12] = 1;
    typechart[5][13] = 4;
    typechart[5][14] = 2;
    typechart[5][15] = 1;
    typechart[5][16] = 2;
    typechart[5][17] = 1;

    typechart[6][1] = 2;
    typechart[6][2] = 1;
    typechart[6][3] = 1;
    typechart[6][4] = 2;
    typechart[6][5] = 4;
    typechart[6][6] = 1;
    typechart[6][7] = 2;
    typechart[6][8] = 2;
    typechart[6][9] = 4;
    typechart[6][10] = 4;
    typechart[6][11] = 2;
    typechart[6][12] = 2;
    typechart[6][13] = 2;
    typechart[6][14] = 2;
    typechart[6][15] = 4;
    typechart[6][16] = 2;
    typechart[6][17] = 1;

    typechart[7][1] = 4;
    typechart[7][2] = 2;
    typechart[7][3] = 2;
    typechart[7][4] = 2;
    typechart[7][5] = 2;
    typechart[7][6] = 4;
    typechart[7][7] = 2;
    typechart[7][8] = 1;
    typechart[7][9] = 2;
    typechart[7][10] = 1;
    typechart[7][11] = 1;
    typechart[7][12] = 1;
    typechart[7][13] = 4;
    typechart[7][14] = 0;
    typechart[7][15] = 2;
    typechart[7][16] = 4;
    typechart[7][17] = 4;

    typechart[8][1] = 2;
    typechart[8][2] = 2;
    typechart[8][3] = 2;
    typechart[8][4] = 2;
    typechart[8][5] = 4;
    typechart[8][6] = 2;
    typechart[8][7] = 2;
    typechart[8][8] = 1;
    typechart[8][9] = 1;
    typechart[8][10] = 2;
    typechart[8][11] = 2;
    typechart[8][12] = 2;
    typechart[8][13] = 1;
    typechart[8][14] = 1;
    typechart[8][15] = 2;
    typechart[8][16] = 2;
    typechart[8][17] = 0;

    typechart[9][1] = 2;
    typechart[9][2] = 4;
    typechart[9][3] = 2;
    typechart[9][4] = 4;
    typechart[9][5] = 1;
    typechart[9][6] = 2;
    typechart[9][7] = 2;
    typechart[9][8] = 4;
    typechart[9][9] = 2;
    typechart[9][10] = 0;
    typechart[9][11] = 2;
    typechart[9][12] = 1;
    typechart[9][13] = 4;
    typechart[9][14] = 2;
    typechart[9][15] = 2;
    typechart[9][16] = 2;
    typechart[9][17] = 4;

    typechart[10][1] = 2;
    typechart[10][2] = 2;
    typechart[10][3] = 2;
    typechart[10][4] = 1;
    typechart[10][5] = 4;
    typechart[10][6] = 2;
    typechart[10][7] = 4;
    typechart[10][8] = 2;
    typechart[10][9] = 2;
    typechart[10][10] = 2;
    typechart[10][11] = 2;
    typechart[10][12] = 4;
    typechart[10][13] = 1;
    typechart[10][14] = 2;
    typechart[10][15] = 2;
    typechart[10][16] = 2;
    typechart[10][17] = 1;

    typechart[11][1] = 2;
    typechart[11][2] = 2;
    typechart[11][3] = 2;
    typechart[11][4] = 2;
    typechart[11][5] = 2;
    typechart[11][6] = 2;
    typechart[11][7] = 4;
    typechart[11][8] = 4;
    typechart[11][9] = 2;
    typechart[11][10] = 2;
    typechart[11][11] = 1;
    typechart[11][12] = 2;
    typechart[11][13] = 2;
    typechart[11][14] = 2;
    typechart[11][15] = 2;
    typechart[11][16] = 0;
    typechart[11][17] = 1;

    typechart[12][1] = 2;
    typechart[12][2] = 1;
    typechart[12][3] = 2;
    typechart[12][4] = 2;
    typechart[12][5] = 4;
    typechart[12][6] = 2;
    typechart[12][7] = 1;
    typechart[12][8] = 1;
    typechart[12][9] = 2;
    typechart[12][10] = 1;
    typechart[12][11] = 4;
    typechart[12][12] = 2;
    typechart[12][13] = 2;
    typechart[12][14] = 1;
    typechart[12][15] = 2;
    typechart[12][16] = 4;
    typechart[12][17] = 1;

    typechart[13][1] = 2;
    typechart[13][2] = 4;
    typechart[13][3] = 2;
    typechart[13][4] = 2;
    typechart[13][5] = 2;
    typechart[13][6] = 4;
    typechart[13][7] = 1;
    typechart[13][8] = 2;
    typechart[13][9] = 1;
    typechart[13][10] = 4;
    typechart[13][11] = 2;
    typechart[13][12] = 4;
    typechart[13][13] = 2;
    typechart[13][14] = 2;
    typechart[13][15] = 2;
    typechart[13][16] = 2;
    typechart[13][17] = 1;

    typechart[14][1] = 0;
    typechart[14][2] = 2;
    typechart[14][3] = 2;
    typechart[14][4] = 2;
    typechart[14][5] = 2;
    typechart[14][6] = 2;
    typechart[14][7] = 2;
    typechart[14][8] = 2;
    typechart[14][9] = 2;
    typechart[14][10] = 2;
    typechart[14][11] = 4;
    typechart[14][12] = 2;
    typechart[14][13] = 2;
    typechart[14][14] = 4;
    typechart[14][15] = 2;
    typechart[14][16] = 1;
    typechart[14][17] = 1;

    typechart[15][1] = 2;
    typechart[15][2] = 2;
    typechart[15][3] = 2;
    typechart[15][4] = 2;
    typechart[15][5] = 2;
    typechart[15][6] = 2;
    typechart[15][7] = 2;
    typechart[15][8] = 2;
    typechart[15][9] = 2;
    typechart[15][10] = 2;
    typechart[15][11] = 2;
    typechart[15][12] = 2;
    typechart[15][13] = 2;
    typechart[15][14] = 2;
    typechart[15][15] = 4;
    typechart[15][16] = 2;
    typechart[15][17] = 1;

    typechart[16][1] = 2;
    typechart[16][2] = 2;
    typechart[16][3] = 2;
    typechart[16][4] = 2;
    typechart[16][5] = 2;
    typechart[16][6] = 2;
    typechart[16][7] = 1;
    typechart[16][8] = 2;
    typechart[16][9] = 2;
    typechart[16][10] = 2;
    typechart[16][11] = 4;
    typechart[16][12] = 2;
    typechart[16][13] = 2;
    typechart[16][14] = 4;
    typechart[16][15] = 2;
    typechart[16][16] = 1;
    typechart[16][17] = 1;

    typechart[17][1] = 2;
    typechart[17][2] = 1;
    typechart[17][3] = 1;
    typechart[17][4] = 1;
    typechart[17][5] = 2;
    typechart[17][6] = 4;
    typechart[17][7] = 2;
    typechart[17][8] = 2;
    typechart[17][9] = 2;
    typechart[17][10] = 2;
    typechart[17][11] = 2;
    typechart[17][12] = 2;
    typechart[17][13] = 4;
    typechart[17][14] = 2;
    typechart[17][15] = 2;
    typechart[17][16] = 2;
    typechart[17][17] = 1;





    //user has choice of pokemon-----------------------------------------------------------------------------------------------------------------------------------------------------------------------

    //allow user to select 3 dex numbers of mons they'd like to battle with--------------------------------------------------------------------------

    printf("Please pick 3 Pokemon to use by entering their names.\n");
    userselection1();
    userselection2();
    userselection3();

    ///string pickurmon1 = get_string("1st mon");
    ///for i=0 i<151; i++
    ///    if (pickurmon == pokedex[i].name)



    int urmons[3];

    urmons[0] = *urmon1ptr; //minus 1 cuz index starts at 0. bulba is 0, if they enter 1, they get bulba
    urmons[1] = *urmon2ptr;
    urmons[2] = *urmon3ptr;

    for (int i = 0; i < 3; i++) //party initialization!!!!!
    {
        party[i].dexno = urmons[i]; //inherits minus 1, dexno should be correct
        party[i].name = pokedex[party[i].dexno].name;
        //nickname
        party[i].xp = 100;
        party[i].growthmod = 4;
        party[i].lvl = 50; //all lvl 50 here, use formula based off xp for a responsive lvl
        //party[i].maxhp = pokedex[party[i].dexno].basehp * party[i].lvl / 10; //hp
        party[i].maxhp = ( (31 + 2 * pokedex[party[i].dexno].basehp) * party[i].lvl / 100) + 10 + party[i].lvl; //formulae for stats in formulas.txt
        party[i].currenthp = party[i].maxhp; //all stats assume 31 ivs, no evs, no nature.
        //party[i].atk = pokedex[party[i].dexno].baseatk * party[i].lvl; //atk
        party[i].atk = (((31 + 2 * pokedex[party[i].dexno].baseatk ) * party[i].lvl / 100 ) + 5 ); //atk
        party[i].def = (((31 + 2 * pokedex[party[i].dexno].basedef ) * party[i].lvl / 100 ) + 5 ); //def
        party[i].spatk = (((31 + 2 * pokedex[party[i].dexno].basespatk ) * party[i].lvl / 100 ) + 5 ); //spatk
        party[i].spdef = (((31 + 2 * pokedex[party[i].dexno].basespdef ) * party[i].lvl / 100 ) + 5 ); //spdef
        party[i].speed = (((31 + 2 * pokedex[party[i].dexno].basespeed ) * party[i].lvl / 100 ) + 5 ); //speed
        party[i].typeint1 = pokedex[party[i].dexno].typeint1;
        party[i].typeint2 = pokedex[party[i].dexno].typeint2;
        party[i].status = "Healthy";
        party[i].statusint = 0;
        party[i].statusintpointer = &party[i].statusint;

        party[i].move1 = pokedex[party[i].dexno].move1;
        party[i].move2 = pokedex[party[i].dexno].move2;
        party[i].move3 = pokedex[party[i].dexno].move3;
        party[i].move4 = pokedex[party[i].dexno].move4;
    }

    //allow user to pick 3 opposing monsters---------------------------------------------------------------------------------------------------------

    printf("\nPlease pick three Pokemon to fight against by entering their names.\n");
    foeselection1();
    foeselection2();
    foeselection3();

    int theirmons[3];






    theirmons[0] = *theirmon1ptr;
    theirmons[1] = *theirmon2ptr;
    theirmons[2] = *theirmon3ptr;

    for (int i = 0; i < 3; i++) //opposing party initialization!!!
    {
        foeparty[i].dexno = theirmons[i];
        foeparty[i].name = pokedex[foeparty[i].dexno].name;
        //nickname
        foeparty[i].xp = 100;
        foeparty[i].growthmod = 4;
        foeparty[i].lvl = 50; //use better leveling function
        //foeparty[i].maxhp = pokedex[foeparty[i].dexno].basehp * foeparty[i].lvl / 10; //we need better stat calcs overall. hp
        foeparty[i].maxhp = ( (31 + 2 * pokedex[foeparty[i].dexno].basehp) * foeparty[i].lvl / 100) + 10 + foeparty[i].lvl;
        foeparty[i].currenthp = foeparty[i].maxhp;
        foeparty[i].atk = (((31 + 2 * pokedex[foeparty[i].dexno].baseatk ) * foeparty[i].lvl / 100 ) + 5 ); //atk
        foeparty[i].def = (((31 + 2 * pokedex[foeparty[i].dexno].basedef ) * foeparty[i].lvl / 100 ) + 5 ); //def
        foeparty[i].spatk = (((31 + 2 * pokedex[foeparty[i].dexno].basespatk ) * foeparty[i].lvl / 100 ) + 5 ); //spatk
        foeparty[i].spdef = (((31 + 2 * pokedex[foeparty[i].dexno].basespdef ) * foeparty[i].lvl / 100 ) + 5 ); //spdef
        foeparty[i].speed = (((31 + 2 * pokedex[foeparty[i].dexno].basespeed ) * foeparty[i].lvl / 100 ) + 5 ); //speed
        foeparty[i].typeint1 = pokedex[foeparty[i].dexno].typeint1;
        foeparty[i].typeint2 = pokedex[foeparty[i].dexno].typeint2;
        foeparty[i].status = "Healthy";
        foeparty[i].statusint = 0;
        foeparty[i].statusintpointer = &foeparty[i].statusint;

        foeparty[i].move1 = pokedex[foeparty[i].dexno].move1;
        foeparty[i].move2 = pokedex[foeparty[i].dexno].move2;
        foeparty[i].move3 = pokedex[foeparty[i].dexno].move3;
        foeparty[i].move4 = pokedex[foeparty[i].dexno].move4;
    }

    //now set combatants[2] as the first monsters in the player's and foe's parties-----------------------------------------------------------------------------------------------------------------------

    //first our guy----------------------------------------------------------------------------------------------------------------------------------

    combatants[0].individualdata = party[0];
    combatants[0].dexnumber = party[0].dexno; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
    combatants[0].nickname = party[0].nickname;
    combatants[0].atkmod = 0; // combatants[0].hp = combatants[0].individualdata.currenthp
    combatants[0].defmod = 0;  //combatants[0].type1 = pokedex[ dexnumber ].type1
    combatants[0].spatkmod = 0;
    combatants[0].spdefmod = 0;
    combatants[0].speedmod = 0;
    combatants[0].evasionmod = 0;
    combatants[0].accuracymod = 0;
    combatants[0].protectcounter = 0;
    combatants[0].status = "Healthy";
    combatants[0].statusint = 0;
    combatants[0].confused = 0;
    combatants[0].disabled = 0;
    combatants[0].toxiccounter = 0;
    combatants[0].leechseeded = 0;
    combatants[0].flinched = 0;
    combatants[0].substitute = 0;
    combatants[0].substitutehp = 0;
    combatants[0].hyperskyskull = "None"; //if this is hyperbeam, cant move. if this is sky attack or skull bash, cant move. same with dig and fly
    combatants[0].invulnerable = 0; //protect, fly, dig
    combatants[0].caughtinwrap = 0; //if theyre being wrapped or fire spun. subtract 1/16 hp and print "user is caught in foe's attack!"
    combatants[0].cursed = 0;
    combatants[0].biding = 0; //0 if not biding, 1 if biding
    combatants[0].bidedamagetracker = 0; // if pokemon is biding, track damage it recieves
    combatants[0].countering = 0;
    combatants[0].counterdamagetracker = 0;
    combatants[0].thrashcounter = 0; //set to 2 or 3 when poke uses thrash or petal dance. decrease each turn. when it hits 0, confuse the user
    combatants[0].whichpartymemberami = 0;


    *combatspeedpointer = party[combatants[0].whichpartymemberami].speed;




    moveset[0] = combatants[0].individualdata.move1;
    moveset[1] = combatants[0].individualdata.move2;
    moveset[2] = combatants[0].individualdata.move3;
    moveset[3] = combatants[0].individualdata.move4;


    //next their guy---------------------------------------------------------------------------------------------------------------------------------

    combatants[1].individualdata = foeparty[0];
    combatants[1].dexnumber = foeparty[0].dexno; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
    combatants[1].nickname = foeparty[0].nickname;
    combatants[1].atkmod = 0; // combatants[0].hp = combatants[0].individualdata.currenthp
    combatants[1].defmod = 0;  //combatants[0].type1 = pokedex[ dexnumber ].type1
    combatants[1].spatkmod = 0;
    combatants[1].spdefmod = 0;
    combatants[1].speedmod = 0;
    combatants[1].evasionmod = 0;
    combatants[1].accuracymod = 0;
    combatants[1].protectcounter = 0;
    combatants[1].status = "Healthy";
    combatants[1].statusint = 0;
    combatants[1].confused = 0;
    combatants[1].disabled = 0;
    combatants[1].toxiccounter = 0;
    combatants[1].leechseeded = 0;
    combatants[1].flinched = 0;
    combatants[1].substitute = 0;
    combatants[1].substitutehp = 0;
    combatants[1].hyperskyskull = "None"; //if this is hyperbeam, cant move. if this is sky attack or skull bash, cant move. same with dig and fly
    combatants[1].invulnerable = 0; //protect, fly, dig
    combatants[1].caughtinwrap = 0; //if theyre being wrapped or fire spun. subtract 1/16 hp and print "user is caught in foe's attack!"
    combatants[1].cursed = 0;
    combatants[1].biding = 0; //0 if not biding, 1 if biding one turn 2 if bidin 2
    combatants[1].bidedamagetracker = 0; // if pokemon is biding, track damage it recieves
    combatants[1].countering = 0;
    combatants[1].counterdamagetracker = 0;
    combatants[1].thrashcounter = 0; //set to 2 or 3 when poke uses thrash or petal dance. decrease each turn. when it hits 0, confuse the user
    combatants[1].whichpartymemberami = 0;


    *foecombatspeedpointer = foeparty[combatants[1].whichpartymemberami].speed;

    foemoveset[0] = combatants[1].individualdata.move1;
    foemoveset[1] = combatants[1].individualdata.move2;
    foemoveset[2] = combatants[1].individualdata.move3;
    foemoveset[3] = combatants[1].individualdata.move4;






//ok below here is the main battle loop----------------------------------------------------------------------------------------------------------------------------------------------------------------
//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------







    printf("\n\nBATTLE START! \n%s, LV %i vs %s, LV %i \n", pokedex[ party[0].dexno ].name, party[0].lvl, pokedex[ foeparty[0].dexno ].name, foeparty[0].lvl);

     while (*isoverpointer == 0) //while the "is the battle over?" integer hasn't been flipped to 1...
    {


        string option = get_string("\nEnter:\nA to attack\nE to escape\nS to switch\nP for party stats\n");   //this is where string option comes from
         //we aren't swtching. query will change this is we choose to switch


        if (*loopcountpointer == 0)
        {
            *userswitchpointer = 0;
        }

        *loopcountpointer = *loopcountpointer + 1;

        query(option, combatants[0]); //query() calls fight(), which calls attack() and deplate(). attack returns int d, damage, deplete manipulates the hp stat of defending pokemon according to damage and prevents from dropping below zero, fight calls deplete and attack to deal damage, and if hp is zero fight ends the battle
        //printf("DEBUGDEBUGDEBUG\n");

        if (*runpointer == 1) //if u ran away, end encounter
            return 0;
        if (*isoverpointer == 1) //if all pkmn has hit 0 hp, end encounter
            //get exp
            //level recalc. stat recalc. end battle
            return 0;

        foequery(combatants[1]); //asks opposition what they want (they always attack for now)

        *foefaint = 0; //reset these
        *userfaint = 0;


        if (*runpointer == 1) //same as above but this time check durign the opponents turn
            return 0;
        if (*isoverpointer == 1)
            //get exp
            //level recalc. stat recalc. end battle
            return 0;

        //now SPEED AND PRIORITY CHECK---------------------------------------------------------------------------------------------------------------
        if (bothmoves[0].movechoice.priority > bothmoves[1].movechoice.priority) //IF USER PRIORITY GREATR--
        {
            //printf("priority user@@@@@@@\n");
            if (*userswitchpointer == 0) //if we havent switched:
            {
                int kill = fight(bothmoves[0].movechoice, bothmoves[0].STAB, bothmoves[0].attacker, bothmoves[0].defender);
            }
                //now check foe hp. if 0, switch foe mon, return 0
            if (*foefaint == 1)
            {
                //do end of turn stuff
                //load combatants[1]
                int fillll = 0;
            }
            else //if foe hp didn't hit zero, they get to fight
            {
                int srvv = foefight(bothmoves[1].movechoice, bothmoves[1].STAB, bothmoves[1].attacker, bothmoves[1].defender);
            }
        }
        else if (bothmoves[0].movechoice.priority < bothmoves[1].movechoice.priority) //IF FOE PRIORITY GREATER--
        {
            //printf("priority ai@@@@@@@@\n");
            int kill = foefight(bothmoves[1].movechoice, bothmoves[1].STAB, bothmoves[1].attacker, bothmoves[1].defender);
                //now check foe hp. if 0, switch foe mon, return 0
            if (*userfaint == 1)
            {
                //do end of turn stuff
                //load combatants[0]
                int fillll = 0;
            }
            else //if foe hp didn't hit zero, they get to fight
            {
                if (*userswitchpointer == 0)
                {
                    int srvv = fight(bothmoves[0].movechoice, bothmoves[0].STAB, bothmoves[0].attacker, bothmoves[0].defender);
                }
                int fiiilllll = 0;
            }
        }
        else //IF PRIORITY IS TIED:::--------------------------------------------------------------
        {   //IF USER FASTER:----------------------------------------------------------------------


            if ( (*combatspeedpointer * speedchange(bothmoves[0].attacker.speedmod, bothmoves[0].attacker.statusint) ) > (*foecombatspeedpointer * speedchange(bothmoves[0].defender.speedmod, bothmoves[0].defender.statusint) ))
            {
                //printf("speed user@@@@@@@\n");
                if (*userswitchpointer == 0)
                {
                    int kill = fight(bothmoves[0].movechoice, bothmoves[0].STAB, bothmoves[0].attacker, bothmoves[0].defender);
                }
                //problem! if kill, other mon is sent in and kill resets to 1, so other mon moves
                if (*foefaint == 1)
                {
                    //do end of turn stuff
                    //load combatants[1]
                    int fillll = 0;
                }
                else //if foe hp didn't hit zero, they get to fight
                {
                    int srvv = foefight(bothmoves[1].movechoice, bothmoves[1].STAB, bothmoves[1].attacker, bothmoves[1].defender);
                }
            }
            //ELSE IF FOE IS FASTER


            else if ( (*combatspeedpointer * speedchange(bothmoves[0].attacker.speedmod, bothmoves[0].attacker.statusint) ) < (*foecombatspeedpointer * speedchange(bothmoves[0].defender.speedmod, bothmoves[0].defender.statusint) ))
            {
                int kill = foefight(bothmoves[1].movechoice, bothmoves[1].STAB, bothmoves[1].attacker, bothmoves[1].defender);
                    //now check foe hp. if 0, switch foe mon, return 0
                if (*userfaint == 1)
                {
                    //do end of turn stuff
                    //load combatants[0]
                    int fillll = 0;
                }
                else //if foe hp didn't hit zero, they get to fight
                {
                    if (*userswitchpointer == 0)
                    {
                        int srvv = fight(bothmoves[0].movechoice, bothmoves[0].STAB, bothmoves[0].attacker, bothmoves[0].defender);
                    }
                    int fiiilllll = 0;
                }
            }




            ////
            else //ELSE  IF SPEED TIE----------------------------------
            {
                //printf("speed ai@@@@@@@@@");
                int movefirst = 1 + rand() % 100;
                if (movefirst > 50) //if heads, they move first
                {
                    int kill = foefight(bothmoves[1].movechoice, bothmoves[1].STAB, bothmoves[1].attacker, bothmoves[1].defender);
                    //now check foe hp. if 0, switch foe mon, return 0
                    if (*userfaint == 1)
                    {
                        //do end of turn stuff
                        //load combatants[0]
                        int fillll = 0;
                    }
                    else //if foe hp didn't hit zero, they get to fight
                    {
                        if (*userswitchpointer == 0)
                        {
                            int srvv = fight(bothmoves[0].movechoice, bothmoves[0].STAB, bothmoves[0].attacker, bothmoves[0].defender);
                        }
                    }
                }
                else //else if tails u move first
                {
                    if (*userswitchpointer == 0) //if u didnt use this turn to  switch u get to fight
                    {
                        int kill = fight(bothmoves[0].movechoice, bothmoves[0].STAB, bothmoves[0].attacker, bothmoves[0].defender);
                    }

                    if (*foefaint == 1)
                    {
                        //do end of turn stuff
                        //load combatants[1]
                        int fillll = 0;
                    }
                    else //if foe hp didn't hit zero, they get to fight
                    {
                        int srvv = foefight(bothmoves[1].movechoice, bothmoves[1].STAB, bothmoves[1].attacker, bothmoves[1].defender);
                    }
                }
            }
        }



        *userswitchpointer = 0;

        // end of turn stuff-------------------------------------------------------------------------------------------------------------------------

        int healthymons = 0; //check if the foe's pokemon are still alive--------------------------
        for (int i = 0; i < 3; i ++)
        {
            if (foeparty[i].currenthp != 0)
            {
                healthymons++;
            }
        }
        if (healthymons == 0) //if they aint, it's all over kid
        {
            *isoverpointer = 1;
            printf("\nYou won the battle!\n");
            return 0;
        }


        healthymons = 0; //check if your pokemon are still alive-----------------------------------
        for (int i = 0; i < 3; i ++)
        {
            if (party[i].currenthp != 0)
            {
                healthymons++;
            }
        }
        if (healthymons == 0) //if they aint, it's all over kid
        {
            *isoverpointer = 1;
            printf("You lost the battle!\n");
            return 0;
        }


        //now specific stuff, first BURN DAMAGE----------------------------------------------------
        if (party[ combatants[0].whichpartymemberami ].statusint == 1) //if you are burned
        {
            printf("\n%s is hurt by it's burn!\n", party[ combatants[0].whichpartymemberami ].name);
            int burndamage = (party[ combatants[0].whichpartymemberami ].maxhp / 8);
            int finhp = foedeplete(&party[ combatants[0].whichpartymemberami ].currenthp, burndamage, party[ combatants[0].whichpartymemberami ]);
        }
        if (foeparty[ combatants[1].whichpartymemberami ].statusint == 1) //if foe is burned. this check doesnt work????
        {
            printf("\n%s is hurt by it's burn!\n", foeparty[ combatants[1].whichpartymemberami ].name);
            int burndamage = (foeparty[ combatants[1].whichpartymemberami ].maxhp / 8);
            int finhp = deplete(&foeparty[ combatants[1].whichpartymemberami ].currenthp, burndamage, foeparty[ combatants[1].whichpartymemberami ]);
        }
        //-----------------------------------------------------------------------------------------

        if (party[ combatants[0].whichpartymemberami ].statusint == 4) //if ur poisoned or toxicd
        {
            printf("\n%s is hurt by poison!\n", party[ combatants[0].whichpartymemberami ].name);
            int psndamage = ((1 + combatants[0].toxiccounter) * party[ combatants[0].whichpartymemberami ].maxhp / 16);
            int finhp = foedeplete(&party[ combatants[0].whichpartymemberami ].currenthp, psndamage, party[ combatants[0].whichpartymemberami ]);
        }
        if (foeparty[ combatants[1].whichpartymemberami ].statusint == 4) //if theyre poisoned or toxicd
        {
            printf("\n%s is hurt by poison!\n", foeparty[ combatants[1].whichpartymemberami ].name);
            int psndamage = ((1 + combatants[1].toxiccounter) * foeparty[ combatants[1].whichpartymemberami ].maxhp / 16);
            int finhp = deplete(&foeparty[ combatants[1].whichpartymemberami ].currenthp, psndamage, foeparty[ combatants[1].whichpartymemberami ]);
        }
        if (combatants[1].toxiccounter > 0) //increase toxic counter if toxicd
        {
            combatants[1].toxiccounter += 1;
        }
        if (combatants[0].toxiccounter > 0)
        {
            combatants[0].toxiccounter += 1;
        }

        //----------------------------

        if (combatants[0].flinched == 1) //resolves flinch
        {
            combatants[0].flinched = 0;
        }
        if (combatants[1].flinched == 1)
        {
            combatants[1].flinched = 0;
        }

        //////////////////
        //if either finhp has reached zero cuz of burns or whatever, wee neeeed to send out a new mon.
        //if (foeparty[ combatants[1].whichpartymemberami ].currenthp <= 0)


        //if (party[combatants[0].whichpartymemberami].currenthp <= 0) //THIS ONE IS BUGGY. evenwhen hp not zero it goes into this block. maybe the info isnt getting updated in between battle cycles? maybe combatants[0] isnt updated properly?
        //int please = combatants[0].individualdata.currenthp;

        //if (combatants[0].individualdata.currenthp <= 0)


        if (party[combatants[0].whichpartymemberami].statusint == 1 || party[combatants[0].whichpartymemberami].statusint == 4 )
        {
            if (*combathppointer <= 0) //this death check is for deaths due to toxic and burning
            {
                *userfaint = 1;
                printf("\nYour Pokemon fainted!\n");
                party[ combatants[0].whichpartymemberami ].status = "Fainted";
                party[ combatants[0].whichpartymemberami ].statusint = 6;


                healthymons = 0; //counts number of non-fainted monsters
                for (int i = 0; i < 3; i ++)
                {
                    if (party[i].currenthp != 0)
                    {
                        healthymons++;
                    }
                }
                if (healthymons == 0) //if they're all fainted, it's all over kid
                {
                    *isoverpointer = 1;
                    printf("You've been defeated!\n");
                    return 2;
                }
                else     //if theyre not all fainted, pick from one that's not
                {
                    *isoverpointer = 0; //battle isnt over
                    printf("\nWhich party member woud you like to switch to?\n");

                    for (int i = 0; i < 3; i++) //list party members that aren't on the field
                    {
                        if (combatants[0].whichpartymemberami != i && party[i].currenthp != 0) //if not mon thats out and not mon that has 0hp
                        {
                            printf("Press %i to switch to %s, lv%i\n", i, party[i].name, party[i].lvl);
                        }
                        else
                        {
                            int filllerrr = 32;
                        }
                    }

                    int whichmem = combatants[0].whichpartymemberami; //just a contraction
                    int choice; //declaration for a do while
                    do //ask player which party member to switch to
                    {
                        choice = get_int("Type choice and press enter:\n");
                    }while(choice == combatants[0].whichpartymemberami || choice < 0 || choice > 3);

                    //ok here i need to keep status. currenthp uses pointers to party/foeparty so should already be updated
                    party[whichmem].status = combatants[0].status;
                    party[whichmem].statusint = combatants[0].statusint;


                   //alright now i need to load party[choice] into combatants[0]
                    combatants[0].individualdata = party[choice];
                    combatants[0].dexnumber = party[choice].dexno; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
                    combatants[0].nickname = party[choice].nickname;
                    combatants[0].atkmod = 0;
                    combatants[0].defmod = 0;
                    combatants[0].spatkmod = 0;
                    combatants[0].spdefmod = 0;
                    combatants[0].speedmod = 0;
                    combatants[0].evasionmod = 0;
                    combatants[0].accuracymod = 0;
                    combatants[0].protectcounter = 0;
                    combatants[0].status = party[choice].status;
                    combatants[0].statusint = party[choice].statusint;
                    combatants[0].confused = 0;
                    combatants[0].disabled = 0;
                    combatants[0].toxiccounter = 0;
                    combatants[0].leechseeded = 0;
                    combatants[0].flinched = 0;
                    combatants[0].substitute = 0;
                    combatants[0].substitutehp = 0;
                    combatants[0].hyperskyskull = "None";
                    combatants[0].invulnerable = 0;
                    combatants[0].caughtinwrap = 0;
                    combatants[0].cursed = 0;
                    combatants[0].biding = 0;
                    combatants[0].bidedamagetracker = 0;
                    combatants[0].countering = 0;
                    combatants[0].counterdamagetracker = 0;
                    combatants[0].thrashcounter = 0;
                    combatants[0].whichpartymemberami = choice;



                    party[combatants[0].whichpartymemberami].currenthp = party[choice].currenthp;

                    *combathppointer = party[choice].currenthp;
                    *combatspeedpointer = party[choice].speed;

                    //no userswitchpointer here, this is end of turn switching cuz of toxic or burn
                    *userfaint = 0;

                    printf("\nCome back, %s. Go, %s!\n", party[whichmem].name, party[choice].name);
                }
            }
        }

        if (foeparty[combatants[1].whichpartymemberami].statusint == 1 || foeparty[combatants[1].whichpartymemberami].statusint == 4)
        {
            if (*foecombathppointer <= 0)
            {
                printf("\nThe opposing Pokemon fainted!\n");
                foeparty[ combatants[1].whichpartymemberami ].status = "Fainted";
                foeparty[ combatants[1].whichpartymemberami ].statusint = 6;
                *foefaint = 1;

                //check if all opposing mons are fainted
                int airandomswitch = -1;
                healthymons = 0;
                for (int i = 0; i < 3; i ++)
                {
                    if (foeparty[i].statusint != 6)
                    {
                        healthymons++;
                    }
                }
                if (healthymons == 0) //if they are, it's all over kid
                {
                    *isoverpointer = 1;
                    printf("\nYou defeated all enemy Pokemon!");
                    return 0;
                }
                else //if not all fainted, ai sends out random not-fainted mon
                {
                    *isoverpointer = 0;
                    do
                    {
                        airandomswitch = rand() % 3;
                    }while(foeparty[ airandomswitch ].statusint == 6);
                }

                //fill out combatants with new mon
                combatants[1].individualdata = foeparty[airandomswitch];
                combatants[1].dexnumber = foeparty[airandomswitch].dexno; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
                combatants[1].nickname = foeparty[airandomswitch].nickname;
                combatants[1].atkmod = 0;
                combatants[1].defmod = 0;
                combatants[1].spatkmod = 0;
                combatants[1].spdefmod = 0;
                combatants[1].speedmod = 0;
                combatants[1].evasionmod = 0;
                combatants[1].accuracymod = 0;
                combatants[1].protectcounter = 0;
                combatants[1].status = foeparty[airandomswitch].status;
                combatants[1].statusint = foeparty[airandomswitch].statusint;
                combatants[1].confused = 0;
                combatants[1].disabled = 0;
                combatants[1].toxiccounter = 0;
                combatants[1].leechseeded = 0;
                combatants[1].flinched = 0;
                combatants[1].substitute = 0;
                combatants[1].substitutehp = 0;
                combatants[1].hyperskyskull = "None";
                combatants[1].invulnerable = 0;
                combatants[1].caughtinwrap = 0;
                combatants[1].cursed = 0;
                combatants[1].biding = 0;
                combatants[1].bidedamagetracker = 0;
                combatants[1].countering = 0;
                combatants[1].counterdamagetracker = 0;
                combatants[1].thrashcounter = 0;
                combatants[1].whichpartymemberami = airandomswitch;

                printf("Foe sent out %s!\n\n", foeparty[airandomswitch].name);
                *foecombathppointer = foeparty[airandomswitch].currenthp;
                *foecombatspeedpointer = foeparty[airandomswitch].speed;

            }
        }
        //////////////
        //do end of turn stuff
    }
}







//end of battle loop-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------







//down here function definitions-----------------------------------------------------------------------------------------------------------------------------------------------------------------------

int query(string option, struct combatmon ourfighter)
{
    if (strcmp(option, "A") == 0) //if u pressed a for fight
    {
        int whichmove = 1;
        int STAB = 2;
        printf("\nPress:\n");
        printf("1 for %s\n", ourfighter.individualdata.move1.name);
        printf("2 for %s\n", ourfighter.individualdata.move2.name);
        printf("3 for %s\n", ourfighter.individualdata.move3.name);
        printf("4 for %s\n", ourfighter.individualdata.move4.name);

        do{
            whichmove = get_int("Enter move selection and press return.\n");
        }while(whichmove > 4 || whichmove < 1);

        struct move ourset[4];

        ourset[0] = ourfighter.individualdata.move1;
        ourset[1] = ourfighter.individualdata.move2;
        ourset[2] = ourfighter.individualdata.move3;
        ourset[3] = ourfighter.individualdata.move4;

        struct move chosenmove = ourset[ whichmove - 1];

        if (ourfighter.individualdata.typeint1 == chosenmove.typeint)
            STAB = 3;
        else if (ourfighter.individualdata.typeint2 == chosenmove.typeint)
            STAB = 3;
        else
            STAB = 2;

//        printf("\n\n%s used %s!\n", combatants[0].individualdata.name, chosenmove.name);

        bothmoves[0].movechoice = chosenmove; //here is the moveselection struct
        bothmoves[0].STAB = STAB;
        bothmoves[0].attacker = combatants[0];
        bothmoves[0].defender = combatants[1];

//        fight(chosenmove, STAB, combatants[0], combatants[1]); //add as argument movestruct   //############# take fight out of query so u can do speed checks then decide who fights first
        return 1;
    }

    else if (strcmp(option, "a") == 0) //if u pressed a for fight
    {
        int whichmove = 1;
        int STAB = 2;
        printf("\nPress:\n");
        printf("1 for %s\n", ourfighter.individualdata.move1.name);
        printf("2 for %s\n", ourfighter.individualdata.move2.name);
        printf("3 for %s\n", ourfighter.individualdata.move3.name);
        printf("4 for %s\n", ourfighter.individualdata.move4.name);

        do{
            whichmove = get_int("Enter move selection and press return.\n");
        }while(whichmove > 4 || whichmove < 1);

        struct move ourset[4];

        ourset[0] = ourfighter.individualdata.move1;
        ourset[1] = ourfighter.individualdata.move2;
        ourset[2] = ourfighter.individualdata.move3;
        ourset[3] = ourfighter.individualdata.move4;

        struct move chosenmove = ourset[ whichmove - 1];

        if (ourfighter.individualdata.typeint1 == chosenmove.typeint)
            STAB = 3;
        else if (ourfighter.individualdata.typeint2 == chosenmove.typeint)
            STAB = 3;
        else
            STAB = 2;

//        printf("\n\n%s used %s!\n", combatants[0].individualdata.name, chosenmove.name);

        bothmoves[0].movechoice = chosenmove; //here is the moveselection struct
        bothmoves[0].STAB = STAB;
        bothmoves[0].attacker = combatants[0];
        bothmoves[0].defender = combatants[1];

//        fight(chosenmove, STAB, combatants[0], combatants[1]); //add as argument movestruct   //############# take fight out of query so u can do speed checks then decide who fights first
        return 1;
    }

    else if (strcmp(option, "E") == 0) //else if u pressed e for escape
    {
        printf("\nYou got away safely!\n");
        *runpointer = 1;
        return 2;
    }

    else if (strcmp(option, "e") == 0) //else if u pressed e for escape
    {
        printf("\nYou got away safely!\n");
        *runpointer = 1;
        return 2;
    }

    else if (strcmp(option, "S") == 0) //else if u pressed s to switch
    {
        printf("\nWhich party member woud you like to switch to?\n");

            for (int i = 0; i < 3; i++) //list party members that aren't on the field
            {
                if (combatants[0].whichpartymemberami != i && party[i].currenthp != 0) //if not mon thats out and not mon that has 0hp
                {
                    printf("Press %i to switch to %s, lv%i\n", i, party[i].name, party[i].lvl);
                }
                else
                {
                    int filllerrr = 32;
                }
            }

            int whichmem = combatants[0].whichpartymemberami; //just a contraction
            int choice; //declaration for a do while
            do //ask player which party member to switch to
            {
                choice = get_int("Type choice and press enter:\n");
            }while(choice == combatants[0].whichpartymemberami || choice < 0 || choice > 3);

            //ok here i need to keep status. currenthp uses pointers to party/foeparty so should already be updated
            party[whichmem].status = combatants[0].status;
            party[whichmem].statusint = combatants[0].statusint;


           //alright now i need to load party[choice] into combatants[0]
            combatants[0].individualdata = party[choice];
            combatants[0].dexnumber = party[choice].dexno; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
            combatants[0].nickname = party[choice].nickname;
            combatants[0].atkmod = 0;
            combatants[0].defmod = 0;
            combatants[0].spatkmod = 0;
            combatants[0].spdefmod = 0;
            combatants[0].speedmod = 0;
            combatants[0].evasionmod = 0;
            combatants[0].accuracymod = 0;
            combatants[0].protectcounter = 0;
            combatants[0].status = party[choice].status;
            combatants[0].statusint = party[choice].statusint;
            combatants[0].confused = 0;
            combatants[0].disabled = 0;
            combatants[0].toxiccounter = 0;
            combatants[0].leechseeded = 0;
            combatants[0].flinched = 0;
            combatants[0].substitute = 0;
            combatants[0].substitutehp = 0;
            combatants[0].hyperskyskull = "None";
            combatants[0].invulnerable = 0;
            combatants[0].caughtinwrap = 0;
            combatants[0].cursed = 0;
            combatants[0].biding = 0;
            combatants[0].bidedamagetracker = 0;
            combatants[0].countering = 0;
            combatants[0].counterdamagetracker = 0;
            combatants[0].thrashcounter = 0;
            combatants[0].whichpartymemberami = choice;

            //printf("choice is %i\n", choice);
            //printf("combatants[0].whichpartymemberami = %i\n", combatants[0].whichpartymemberami);

            party[combatants[0].whichpartymemberami].currenthp = party[choice].currenthp;
            *combathppointer = party[choice].currenthp;
            *combatspeedpointer = party[choice].speed;

            //printf("current hp of party[choice] is %i\n", party[choice].currenthp);
            //printf("party[combatants[0].whichpartymemberami].currenthp = %i\n", party[combatants[0].whichpartymemberami].currenthp);

            *userswitchpointer = 1;
            *userfaint = 0;


            printf("\nCome back, %s. Go, %s!\n", party[whichmem].name, party[choice].name);
        return 2;
    }

    else if (strcmp(option, "s") == 0) //else if u pressed s to switch
    {
        printf("\nWhich party member would you like to switch to?\n");

            for (int i = 0; i < 3; i++) //list party members that aren't on the field
            {
                if (combatants[0].whichpartymemberami != i && party[i].currenthp != 0) //if not mon thats out and not mon that has 0hp
                {
                    printf("Press %i to switch to %s, lv%i\n", i, party[i].name, party[i].lvl);
                }
                else
                {
                    int filllerrr = 32;
                }
            }

            int whichmem = combatants[0].whichpartymemberami; //just a contraction
            int choice; //declaration for a do while
            do //ask player which party member to switch to
            {
                choice = get_int("Type choice and press enter:\n");
            }while(choice == combatants[0].whichpartymemberami || choice < 0 || choice > 3);

            //ok here i need to keep status. currenthp uses pointers to party/foeparty so should already be updated
            party[whichmem].status = combatants[0].status;
            party[whichmem].statusint = combatants[0].statusint;


           //alright now i need to load party[choice] into combatants[0]
            combatants[0].individualdata = party[choice];
            combatants[0].dexnumber = party[choice].dexno; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
            combatants[0].nickname = party[choice].nickname;
            combatants[0].atkmod = 0;
            combatants[0].defmod = 0;
            combatants[0].spatkmod = 0;
            combatants[0].spdefmod = 0;
            combatants[0].speedmod = 0;
            combatants[0].evasionmod = 0;
            combatants[0].accuracymod = 0;
            combatants[0].protectcounter = 0;
            combatants[0].status = party[choice].status;
            combatants[0].statusint = party[choice].statusint;
            combatants[0].confused = 0;
            combatants[0].disabled = 0;
            combatants[0].toxiccounter = 0;
            combatants[0].leechseeded = 0;
            combatants[0].flinched = 0;
            combatants[0].substitute = 0;
            combatants[0].substitutehp = 0;
            combatants[0].hyperskyskull = "None";
            combatants[0].invulnerable = 0;
            combatants[0].caughtinwrap = 0;
            combatants[0].cursed = 0;
            combatants[0].biding = 0;
            combatants[0].bidedamagetracker = 0;
            combatants[0].countering = 0;
            combatants[0].counterdamagetracker = 0;
            combatants[0].thrashcounter = 0;
            combatants[0].whichpartymemberami = choice;

            //printf("choice is %i\n", choice);
            //printf("combatants[0].whichpartymemberami = %i\n", combatants[0].whichpartymemberami);

            party[combatants[0].whichpartymemberami].currenthp = party[choice].currenthp;

            *combathppointer = party[choice].currenthp;
            *combatspeedpointer = party[choice].speed;

            //printf("current hp of party[choice] is %i\n", party[choice].currenthp);
            //printf("party[combatants[0].whichpartymemberami].currenthp = %i\n", party[combatants[0].whichpartymemberami].currenthp);

            *userswitchpointer = 1;
            *userfaint = 0;

            printf("\nCome back, %s. Go, %s!\n", party[whichmem].name, party[choice].name);
        return 2;
    }

        //    else if stcmp optiooon is P then print out their party stats, moves, status
    else if (strcmp(option, "P") == 0) //if u pressed P for party
    {
        printf("\n\n");
        printf("%s Lv 50, HP: %i/%i, Status: %s\n%s, %s, %s, %s\n", party[0].name, party[0].currenthp, party[0].maxhp, statinttostring(party[0].statusint), party[0].move1.name, party[0].move2.name, party[0].move3.name, party[0].move4.name);
        printf("ATTACK: %i, DEFENSE: %i, SPECIAL ATK: %i, SPECIAL DEF: %i, SPEED: %i\n\n", party[0].atk, party[0].def, party[0].spatk, party[0].spdef, party[0].speed);

        printf("%s Lv 50, HP: %i/%i, Status: %s\n%s, %s, %s, %s\n", party[1].name, party[1].currenthp, party[1].maxhp, statinttostring(party[1].statusint), party[1].move1.name, party[1].move2.name, party[1].move3.name, party[1].move4.name);
        printf("ATTACK: %i, DEFENSE: %i, SPECIAL ATK: %i, SPECIAL DEF: %i, SPEED: %i\n\n", party[1].atk, party[1].def, party[1].spatk, party[1].spdef, party[1].speed);

        printf("%s Lv 50, HP: %i/%i, Status: %s\n%s, %s, %s, %s\n", party[2].name, party[2].currenthp, party[2].maxhp, statinttostring(party[2].statusint), party[2].move1.name, party[2].move2.name, party[2].move3.name, party[2].move4.name);
        printf("ATTACK: %i, DEFENSE: %i, SPECIAL ATK: %i, SPECIAL DEF: %i, SPEED: %i\n\n", party[2].atk, party[2].def, party[2].spatk, party[2].spdef, party[2].speed);

        option = get_string("\nEnter:\nA to attack\nE to escape\nS to switch\nP for party stats\n\n");
        query(option, ourfighter); //looooook its recursion B) if you request party stats, you query again
    }

    else if (strcmp(option, "p") == 0) //if u pressed P for party
    {
        printf("\n\n");
        printf("%s Lv 50, HP: %i/%i, Status: %s\n%s, %s, %s, %s\n", party[0].name, party[0].currenthp, party[0].maxhp, statinttostring(party[0].statusint), party[0].move1.name, party[0].move2.name, party[0].move3.name, party[0].move4.name);
        printf("ATTACK: %i, DEFENSE: %i, SPECIAL ATK: %i, SPECIAL DEF: %i, SPEED: %i\n\n", party[0].atk, party[0].def, party[0].spatk, party[0].spdef, party[0].speed);

        printf("%s Lv 50, HP: %i/%i, Status: %s\n%s, %s, %s, %s\n", party[1].name, party[1].currenthp, party[1].maxhp, statinttostring(party[1].statusint), party[1].move1.name, party[1].move2.name, party[1].move3.name, party[1].move4.name);
        printf("ATTACK: %i, DEFENSE: %i, SPECIAL ATK: %i, SPECIAL DEF: %i, SPEED: %i\n\n", party[1].atk, party[1].def, party[1].spatk, party[1].spdef, party[1].speed);

        printf("%s Lv 50, HP: %i/%i, Status: %s\n%s, %s, %s, %s\n", party[2].name, party[2].currenthp, party[2].maxhp, statinttostring(party[2].statusint), party[2].move1.name, party[2].move2.name, party[2].move3.name, party[2].move4.name);
        printf("ATTACK: %i, DEFENSE: %i, SPECIAL ATK: %i, SPECIAL DEF: %i, SPEED: %i\n\n", party[2].atk, party[2].def, party[2].spatk, party[2].spdef, party[2].speed);

        option = get_string("\nEnter:\nA to attack\nE to escape\nS to switch\nP for party stats\n\n");
        query(option, ourfighter); //looooook its recursion B) if you request party stats, you query again
    }
    else //note, if you don't input A or E or S then you return 3, but pikachu still attacks you that turn. use this to test ur own poke fainting
    {
        printf("Input not recognized.\n");
        option = get_string("\nEnter:\nA to attack\nE to escape\nS to switch\nP for party stats\n");
        query(option, ourfighter);
    }
    return 69;
}


int foequery(struct combatmon theirfighter)
{
    int STAB = 2;
    int randy = rand() % 4;

    struct move theirset[4];

    theirset[0] = theirfighter.individualdata.move1;
    theirset[1] = theirfighter.individualdata.move2;
    theirset[2] = theirfighter.individualdata.move3;
    theirset[3] = theirfighter.individualdata.move4;

    struct move chosenmove = theirset[ randy ];

    if (theirfighter.individualdata.typeint1 == chosenmove.typeint)
        STAB = 3;
    else if (theirfighter.individualdata.typeint2 == chosenmove.typeint)
        STAB = 3;
    else
        STAB = 2;

//    printf("\n\n%s used %s!\n", theirfighter.individualdata.name, chosenmove.name);

    bothmoves[1].movechoice = chosenmove; //heres the moveselection struct
    bothmoves[1].STAB = STAB;
    bothmoves[1].attacker = combatants[1];
    bothmoves[1].defender = combatants[0];

//    foefight(chosenmove, STAB, combatants[1], combatants[0]);
    return 1;
}





int fight(struct move moveused, int STAB, struct combatmon attacker, struct combatmon defender) //ties attack and deplete functions together. u hurting foe
{
    printf("\n\nYour %s used %s!\n", combatants[0].individualdata.name, moveused.name);




    int dmg = attack(attacker, defender, moveused, STAB);
    int eighth = dmg / 8;

    //if attack is a status move,
    //  do statusmove(attacker, defender, moveused)
    //  return 1
    // remember to check who the target of the move is!

    /////////////////////////////////////////////////

    if (moveused.attackorstatus == 1) //if its a status move
    {
        if (strcmp(moveused.effect, "Rest") == 0) //if it's rest
        {
            party[ combatants[0].whichpartymemberami].statusint = 3;
            party[ combatants[0].whichpartymemberami].currenthp = party[ combatants[0].whichpartymemberami].maxhp;
            deplete(&party[ combatants[0].whichpartymemberami].currenthp, -1000, party[combatants[0].whichpartymemberami]);
            printf("%s went to sleep and became healthy!\n", party[ combatants[0].whichpartymemberami].name);
            return 15;
        }
        else if (strcmp(moveused.effect, "Restore") == 0) //if its a recovery move
        {
            int half = -1 * party[ combatants[0].whichpartymemberami].maxhp / 2;
            printf("HP was restored!\n");
            deplete(&party[ combatants[0].whichpartymemberami].currenthp, half, party[combatants[0].whichpartymemberami]);
            return 15;
        }
        else if (strcmp(moveused.effect, "Paralyze") == 0)
        {
            int didithit = 1 + rand() % 100;
            if (didithit < moveused.acc)
            {
                foeparty[ combatants[1].whichpartymemberami].statusint = 5;
                printf("The opposing Pokemon was paralyzed!\n");
                return 15;
            }
            else
            {
                printf("The attack missed!\n");
                return 16;
            }
        }
        else if (strcmp(moveused.effect, "Poison") == 0)
        {
            int didithit = 1 + rand() % 100;
            if (didithit < moveused.acc)
            {
                foeparty[ combatants[1].whichpartymemberami].statusint = 4;
                printf("The opposing Pokemon was poisoned!\n");
                return 15;
            }
            else
            {
                printf("The attack missed!\n");
                return 16;
            }
        }
        else if (strcmp(moveused.effect, "Toxic") == 0)
        {
            int didithit = 1 + rand() % 100;
            if (didithit < moveused.acc)
            {
                foeparty[ combatants[1].whichpartymemberami].statusint = 4;
                if (combatants[1].toxiccounter < 1)
                {
                    combatants[1].toxiccounter = 1;
                }
                printf("The opposing Pokemon was badly poisoned!\n");
                return 15;
            }
            else
            {
                printf("The attack missed!\n");
                return 16;
            }
        }
        else if (strcmp(moveused.effect, "Sleep") == 0)
        {
            int didithit = 1 + rand() % 100;
            if (didithit < moveused.acc)
            {
                foeparty[ combatants[1].whichpartymemberami].statusint = 3;
                printf("The opposing Pokemon fell asleep!\n");
                return 15;
            }
            else
            {
                printf("The attack missed!\n");
                return 16;
            }
        }
        else if (strcmp(moveused.effect, "Confuse") == 0)
        {
            int didithit = 1 + rand() % 100;
            if (didithit < moveused.acc)
            {
                combatants[1].confused = 1;
                printf("The opposing Pokemon was confused!\n");
                return 15;
            }
            else
            {
                printf("The attack missed!\n");
                return 16;
            }
        }
        else if (strcmp(moveused.effect, "+atk+speed") == 0)
        {
            combatants[0].atkmod += 1;
            combatants[0].speedmod += 1;
            printf("Attack and speed rose!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+atk+def") == 0)
        {
            combatants[0].atkmod += 1;
            combatants[0].defmod += 1;
            printf("Attack and defense rose!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+spatk+spdef") == 0)
        {
            combatants[0].spatkmod += 1;
            combatants[0].spdefmod += 1;
            printf("Special attack and special defense rose!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+2spdef") == 0)
        {
            combatants[0].spdefmod += 1;
            combatants[0].spdefmod += 1;
            printf("Special defense rose sharply!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+2spatk") == 0)
        {
            combatants[0].spatkmod += 1;
            combatants[0].spatkmod += 1;
            printf("Special attack rose sharply!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+2speed") == 0)
        {
            combatants[0].speedmod += 1;
            combatants[0].speedmod += 1;
            printf("Speed rose sharply!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+def") == 0)
        {
            combatants[0].defmod += 1;
            printf("Defense rose!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+2def") == 0)
        {
            combatants[0].defmod += 1;
            combatants[0].defmod += 1;
            printf("Defense rose sharply!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+2atk") == 0)
        {
            combatants[0].atkmod += 1;
            combatants[0].atkmod += 1;
            printf("Attack rose sharply!\n");
            return 17;
        }
        else
        {
            printf("The move failed!\n");
            return 18;
        }
    }




    /////////////////////////////////////////////////////

    //add conditional here that checks for confuse, flinch, sleep, freeze, paralyze, etc to see if you get to move



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (party[combatants[0].whichpartymemberami].statusint == 2) //if you're frozen, roll 30%chancce of unfreezing
    {
        int unfreeze = 1 + rand() % 10;
        if (unfreeze > 7) //if they unfreeze
        {
            printf("%s was unfrozen!\n", combatants[0].individualdata.name);
            attacker.individualdata.statusint = 0;
            party[combatants[0].whichpartymemberami].statusint = 0;
        }
        else //if they dont
        {
            printf("%s is frozen solid!\n", combatants[0].individualdata.name);
            return 11;
        }
    }

    if (combatants[0].confused == 1) //if confused
    {
        printf("%s is confused!\n", combatants[0].individualdata.name);
        int hiturself = 1 + rand() % 10;
        int unconfuse = 1 + rand() % 10;
        if (unconfuse >= 6) //if they snap out of confusion
        {
            attacker.confused = 0;
            printf("%s snapped out of confusion!\n", combatants[0].individualdata.name);
            hiturself = 1;
        }
        if (hiturself >= 7) //if hit self in confusion, return
        {
            printf("%s hurt itself in confusion!\n", combatants[0].individualdata.name);
            foedeplete(&party[ combatants[0].whichpartymemberami ].currenthp, eighth, attacker.individualdata);
            return 12;
        } //else, just continue function
    }

    if (combatants[0].flinched == 1) //if flinched
    {
        printf("... unfortunately, %s flinched and it's attack failed!\n", combatants[0].individualdata.name);
        attacker.flinched = 0;
        return 13;
    }

    if (party[combatants[0].whichpartymemberami].statusint == 3) //if you're asleep, roll 30% chancce of waking up
    {
        int unfreeze = 1 + rand() % 10;
        if (unfreeze > 7) //if they wake up
        {
            printf("%s woke up!\n", combatants[0].individualdata.name);
            attacker.individualdata.statusint = 0;
            party[combatants[0].whichpartymemberami].statusint = 0;
        }
        else //if they dont
        {
            printf("... but %s is fast asleep!\n", combatants[0].individualdata.name);
            return 11;
        }
    }

    if (party[combatants[0].whichpartymemberami].statusint == 5) //if you're paralyzed, roll 30% chancce of no move
    {
        //printf("%s is paralyzed!\n", combatants[0].individualdata.name);
        int unfreeze = 1 + rand() % 10; //if they paralyzed
        if (unfreeze > 7)
        {
            printf("%s can't move because of paralysis!\n", combatants[0].individualdata.name);
            return 14;
        }//if not, continue with turn as usual
    }




    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int recorsucc = (-1 * moveused.recoilordrain * dmg / 8);
    if (moveused.recoilordrain != 0) //if recoil like double edge or absorption like giga drain
    {
        int urhp = foedeplete(&party[ combatants[0].whichpartymemberami ].currenthp, recorsucc, attacker.individualdata);
        if (*combathppointer <= 0)
        {
            *userfaint = 1;
            printf("\nYour Pokemon fainted!\n");
            party[ combatants[0].whichpartymemberami ].status = "Fainted";
            party[ combatants[0].whichpartymemberami ].statusint = 6;


            int healthymons = 0; //counts number of non-fainted monsters
            for (int i = 0; i < 3; i ++)
            {
                if (party[i].currenthp != 0)
                {
                    healthymons++;
                }
            }
            if (healthymons == 0) //if they're all fainted, it's all over kid
            {
                *isoverpointer = 1;
                printf("You've been defeated!\n");
                return 2;
            }
            else     //if theyre not all fainted, pick from one that's not
            {
                *isoverpointer = 0; //battle isnt over
                printf("\nWhich party member woud you like to switch to?\n");

                for (int i = 0; i < 3; i++) //list party members that aren't on the field
                {
                    if (combatants[0].whichpartymemberami != i && party[i].currenthp != 0) //if not mon thats out and not mon that has 0hp
                    {
                        printf("Press %i to switch to %s, lv%i\n", i, party[i].name, party[i].lvl);
                    }
                    else
                    {
                        int filllerrr = 32;
                    }
                }

                int whichmem = combatants[0].whichpartymemberami; //just a contraction
                int choice; //declaration for a do while
                do //ask player which party member to switch to
                {
                    choice = get_int("Type choice and press enter:\n");
                }while(choice == combatants[0].whichpartymemberami || choice < 0 || choice > 3);

                //ok here i need to keep status. currenthp uses pointers to party/foeparty so should already be updated
                party[whichmem].status = combatants[0].status;
                party[whichmem].statusint = combatants[0].statusint;


               //alright now i need to load party[choice] into combatants[0]
                combatants[0].individualdata = party[choice];
                combatants[0].dexnumber = party[choice].dexno; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
                combatants[0].nickname = party[choice].nickname;
                combatants[0].atkmod = 0;
                combatants[0].defmod = 0;
                combatants[0].spatkmod = 0;
                combatants[0].spdefmod = 0;
                combatants[0].speedmod = 0;
                combatants[0].evasionmod = 0;
                combatants[0].accuracymod = 0;
                combatants[0].protectcounter = 0;
                combatants[0].status = party[choice].status;
                combatants[0].statusint = party[choice].statusint;
                combatants[0].confused = 0;
                combatants[0].disabled = 0;
                combatants[0].toxiccounter = 0;
                combatants[0].leechseeded = 0;
                combatants[0].flinched = 0;
                combatants[0].substitute = 0;
                combatants[0].substitutehp = 0;
                combatants[0].hyperskyskull = "None";
                combatants[0].invulnerable = 0;
                combatants[0].caughtinwrap = 0;
                combatants[0].cursed = 0;
                combatants[0].biding = 0;
                combatants[0].bidedamagetracker = 0;
                combatants[0].countering = 0;
                combatants[0].counterdamagetracker = 0;
                combatants[0].thrashcounter = 0;
                combatants[0].whichpartymemberami = choice;



                party[combatants[0].whichpartymemberami].currenthp = party[choice].currenthp;

                *combathppointer = party[choice].currenthp;
                *combatspeedpointer = party[choice].speed;

                *userswitchpointer = 1;
                *userfaint = 0;

                printf("\nCome back, %s. Go, %s!\n", party[whichmem].name, party[choice].name);
            }
        }
        //if ur hp hits 0
    }
    //if moveused.attackorstatus == 0;
    int finhp = deplete(&foeparty[ combatants[1].whichpartymemberami ].currenthp, dmg, defender.individualdata);
    //else
    //run status checks. strcmp if moveused.effect == "Burn" or if it == -spdef etc
    //
    //+2atk, +2def, +speed, +2speed, +2spatk, +2spdef, +spatk+spdef, +atk+def, +atk+speed, confuse, toxic, sleep, poison, paralyze, restore, rest,

    ////////////////////////////////////////////////////////////////////////
    //below are secondary effect checks
    if (moveused.effectchance != 0 && dmg > 0) //if theres an effect chance, and the attack dealt damage
    {
        if (strcmp(moveused.effect, "Burn") == 0) //if effect is burn
        {
            if (defender.individualdata.statusint == 0) //and theyre healthy
            {
                int rollchanceb = 1 + rand() % 100;
                if (rollchanceb < moveused.effectchance)
                {
                    defender.individualdata.statusint = 1; //burn them                 //THIS LINE DOESNT WORK
                    foeparty[ combatants[1].whichpartymemberami ].statusint = 1;       //THIS LINE DOES IDK WHY
                    printf("%s was burned!\n", defender.individualdata.name);
                }
            }
        }
        else if (strcmp(moveused.effect, "Poison") == 0)
        {
            if (defender.individualdata.statusint == 0)
            {
                int rollchancep = 1 + rand() % 100;
                if (rollchancep < moveused.effectchance)
                {
                    defender.individualdata.statusint = 4;
                    foeparty[ combatants[1].whichpartymemberami ].statusint = 4;
                    printf("%s was poisoned!\n", defender.individualdata.name);
                }
            }
        }
        else if (strcmp(moveused.effect, "Toxic") == 0)
        {
            if (defender.individualdata.statusint == 0)
            {
                int rollchancet = 1 + rand() % 100;
                if (rollchancet < moveused.effectchance)
                {
                    defender.individualdata.statusint = 4;
                    foeparty[ combatants[1].whichpartymemberami ].statusint = 4;
                    printf("%s was badly poisoned!\n", defender.individualdata.name);
                    combatants[1].toxiccounter = 1;
                    defender.toxiccounter = 1;
                }
            }
        }
        else if (strcmp(moveused.effect, "Freeze") == 0)
        {
            if (defender.individualdata.statusint == 0)
            {
                int rollchancef = 1 + rand() % 100;
                if (rollchancef < moveused.effectchance)
                {
                    defender.individualdata.statusint = 2;
                    foeparty[ combatants[1].whichpartymemberami ].statusint = 2;
                    printf("%s was frozen solid!\n", defender.individualdata.name);
                }
            }
        }
        else if (strcmp(moveused.effect, "Paralyze") == 0)
        {
            if (defender.individualdata.statusint == 0)
            {
                int rollchancepz = 1 + rand() % 100;
                if (rollchancepz < moveused.effectchance)
                {
                    defender.individualdata.statusint = 5;
                    foeparty[ combatants[1].whichpartymemberami ].statusint = 5;
                    printf("%s was paralyzed!\n", defender.individualdata.name);
                }
            }
        }
        else if (strcmp(moveused.effect, "Sleep") == 0)
        {
            if (defender.individualdata.statusint == 0)
            {
                int rollchances = 1 + rand() % 100;
                if (rollchances < moveused.effectchance)
                {
                    defender.individualdata.statusint = 3;
                    foeparty[ combatants[1].whichpartymemberami ].statusint = 3;
                    printf("%s fell asleep!\n", defender.individualdata.name);
                }
            }
        }
        else if (strcmp(moveused.effect, "-spdef") == 0)
        {
            int rollspdef = 1 + rand() % 100;
            if (rollspdef < moveused.effectchance)
            {
                defender.spdefmod = defender.spdefmod - 1;
                combatants[1].spdefmod = combatants[1].spdefmod - 1;
                printf("%s had it's special defense lowered!\n", defender.individualdata.name);
                if (defender.spdefmod < -6)
                {
                    defender.spdefmod = -6;
                }
            }
        }
        else if (strcmp(moveused.effect, "-def") == 0)
        {
            int rolldef = 1 + rand() % 100;
            if (rolldef < moveused.effectchance)
            {
                defender.defmod = defender.defmod - 1;
                combatants[1].defmod = combatants[1].defmod - 1;
                printf("%s had it's defense lowered!\n", defender.individualdata.name);
                if (defender.defmod < -6)
                {
                    defender.defmod = -6;
                }
            }
        }
        else if (strcmp(moveused.effect, "-atk") == 0)
        {
            int rollatk = 1 + rand() % 100;
            if (rollatk < moveused.effectchance)
            {
                defender.atkmod = defender.atkmod - 1;
                combatants[1].atkmod = combatants[1].atkmod - 1;
                printf("%s had it's attack lowered!\n", defender.individualdata.name);
                if (defender.atkmod < -6)
                {
                    defender.atkmod = -6;
                }
            }
        }
        else if (strcmp(moveused.effect, "Flinch") == 0)
        {
            int rollflinch = 1 + rand() % 100;
            if (rollflinch < moveused.effectchance)
            {
                defender.flinched = 1;
                combatants[1].flinched = 1;
            }
        }
        else if (strcmp(moveused.effect, "Confuse") == 0)
        {
            int rollcon = 1 + rand() % 100;
            if (rollcon < moveused.effectchance)
            {
                defender.confused = 1;
                combatants[1].confused = 1;
                printf("%s became confused!\n", defender.individualdata.name);
            }
        }
    }


    //we're still inside the fight() function lol




    ////////////////////////////////////////////////////////

    if (*foecombathppointer <= 0) //
    {
        printf("\nThe opposing Pokemon fainted!\n");
        foeparty[ combatants[1].whichpartymemberami ].status = "Fainted";
        foeparty[ combatants[1].whichpartymemberami ].statusint = 6;
        *foefaint = 1;

        //check if all opposing mons are fainted
        int airandomswitch = -1;
        int healthymons = 0;
        for (int i = 0; i < 3; i ++)
        {
            if (foeparty[i].statusint != 6)
            {
                healthymons++;
            }
        }
        if (healthymons == 0) //if they are, it's all over kid
        {
            *isoverpointer = 1;
            printf("\nYou defeated all enemy Pokemon!");
            return 0;
        }
        else //if not all fainted, ai sends out random not-fainted mon
        {
            *isoverpointer = 0;
            do
            {
                airandomswitch = rand() % 3;
            }while(foeparty[ airandomswitch ].statusint == 6);
        }

        //fill out combatants with new mon
        combatants[1].individualdata = foeparty[airandomswitch];
        combatants[1].dexnumber = foeparty[airandomswitch].dexno; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
        combatants[1].nickname = foeparty[airandomswitch].nickname;
        combatants[1].atkmod = 0;
        combatants[1].defmod = 0;
        combatants[1].spatkmod = 0;
        combatants[1].spdefmod = 0;
        combatants[1].speedmod = 0;
        combatants[1].evasionmod = 0;
        combatants[1].accuracymod = 0;
        combatants[1].protectcounter = 0;
        combatants[1].status = foeparty[airandomswitch].status;
        combatants[1].statusint = foeparty[airandomswitch].statusint;
        combatants[1].confused = 0;
        combatants[1].disabled = 0;
        combatants[1].toxiccounter = 0;
        combatants[1].leechseeded = 0;
        combatants[1].flinched = 0;
        combatants[1].substitute = 0;
        combatants[1].substitutehp = 0;
        combatants[1].hyperskyskull = "None";
        combatants[1].invulnerable = 0;
        combatants[1].caughtinwrap = 0;
        combatants[1].cursed = 0;
        combatants[1].biding = 0;
        combatants[1].bidedamagetracker = 0;
        combatants[1].countering = 0;
        combatants[1].counterdamagetracker = 0;
        combatants[1].thrashcounter = 0;
        combatants[1].whichpartymemberami = airandomswitch;

        printf("Foe sent out %s!\n\n", foeparty[airandomswitch].name);
        *foecombathppointer = foeparty[airandomswitch].currenthp;
        *foecombatspeedpointer = foeparty[airandomswitch].speed;

    }

    //do checks for non-major status effect of moves.
    //like -spdef, of confusion, or flinching

    //endoffight() bookmark

    return 1;
}

int foefight(struct move moveused, int STAB, struct combatmon attacker, struct combatmon defender) //ties attack and deplete functions together. foe hurting u
{   //add conditional here that checks for confuse, flinch, sleep, freeze, paralyze, etc to see if they get to move
    printf("\n\nFoe's %s used %s!\n", attacker.individualdata.name, moveused.name);
    int dmg = foeattack(attacker, defender, moveused, STAB);
    int eighth = dmg / 8;

    //if attack is a status move,
    //  do statusmove(attacker, defender, moveused)
    //  return 1
    ///////////////////////////////////////////////////////////////////////////////////////////////
    if (moveused.attackorstatus == 1) //if its a status move
    {
        if (strcmp(moveused.effect, "Rest") == 0) //if it's rest
        {
            foeparty[ combatants[1].whichpartymemberami].statusint = 3;
            foeparty[ combatants[1].whichpartymemberami].currenthp = foeparty[ combatants[1].whichpartymemberami].maxhp;
            deplete(&foeparty[ combatants[1].whichpartymemberami].currenthp, -1000, foeparty[combatants[1].whichpartymemberami]);
            printf("%s went to sleep and became healthy!\n", foeparty[ combatants[1].whichpartymemberami].name);
            return 15;
        }
        else if (strcmp(moveused.effect, "Restore") == 0) //if its a recovery move
        {
            int half = -1 * foeparty[ combatants[1].whichpartymemberami].maxhp / 2;
            printf("HP was restored!\n");
            deplete(&foeparty[ combatants[1].whichpartymemberami].currenthp, half, foeparty[combatants[1].whichpartymemberami]);
            return 15;
        }
        else if (strcmp(moveused.effect, "Paralyze") == 0)
        {
            int didithit = 1 + rand() % 100;
            if (didithit < moveused.acc)
            {
                party[ combatants[0].whichpartymemberami].statusint = 5;
                printf("Your Pokemon was paralyzed!\n");
                return 15;
            }
            else
            {
                printf("The attack missed!\n");
                return 16;
            }
        }
        else if (strcmp(moveused.effect, "Poison") == 0)
        {
            int didithit = 1 + rand() % 100;
            if (didithit < moveused.acc)
            {
                party[ combatants[0].whichpartymemberami].statusint = 4;
                printf("Your Pokemon was poisoned!\n");
                return 15;
            }
            else
            {
                printf("The attack missed!\n");
                return 16;
            }
        }
        else if (strcmp(moveused.effect, "Toxic") == 0)
        {
            int didithit = 1 + rand() % 100;
            if (didithit < moveused.acc)
            {
                party[ combatants[0].whichpartymemberami].statusint = 4;
                if (combatants[0].toxiccounter < 1)
                {
                    combatants[0].toxiccounter = 1;
                }
                printf("Your Pokemon was badly poisoned!\n");
                return 15;
            }
            else
            {
                printf("The attack missed!\n");
                return 16;
            }
        }
        else if (strcmp(moveused.effect, "Sleep") == 0)
        {
            int didithit = 1 + rand() % 100;
            if (didithit < moveused.acc)
            {
                party[ combatants[0].whichpartymemberami].statusint = 3;
                printf("Your Pokemon fell asleep!\n");
                return 15;
            }
            else
            {
                printf("The attack missed!\n");
                return 16;
            }
        }
        else if (strcmp(moveused.effect, "Confuse") == 0)
        {
            int didithit = 1 + rand() % 100;
            if (didithit < moveused.acc)
            {
                combatants[0].confused = 1;
                printf("Your Pokemon became confused!\n");
                return 15;
            }
            else
            {
                printf("The attack missed!\n");
                return 16;
            }
        }
        else if (strcmp(moveused.effect, "+atk+speed") == 0)
        {
            combatants[1].atkmod += 1;
            combatants[1].speedmod += 1;
            printf("Attack and speed rose!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+atk+def") == 0)
        {
            combatants[1].atkmod += 1;
            combatants[1].defmod += 1;
            printf("Attack and defense rose!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+spatk+spdef") == 0)
        {
            combatants[1].spatkmod += 1;
            combatants[1].spdefmod += 1;
            printf("Special attack and special defense rose!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+2spdef") == 0)
        {
            combatants[1].spdefmod += 1;
            combatants[1].spdefmod += 1;
            printf("Special defense rose sharply!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+2spatk") == 0)
        {
            combatants[1].spatkmod += 1;
            combatants[1].spatkmod += 1;
            printf("Special attack rose sharply!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+2speed") == 0)
        {
            combatants[1].speedmod += 1;
            combatants[1].speedmod += 1;
            printf("Speed rose sharply!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+def") == 0)
        {
            combatants[1].defmod += 1;
            printf("Defense rose!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+2def") == 0)
        {
            combatants[1].defmod += 1;
            combatants[1].defmod += 1;
            printf("Defense rose sharply!\n");
            return 17;
        }
        else if (strcmp(moveused.effect, "+2atk") == 0)
        {
            combatants[1].atkmod += 1;
            combatants[1].atkmod += 1;
            printf("Attack rose sharply!\n");
            return 17;
        }
        else
        {
            printf("The move failed!\n");
            return 18;
        }
    }






    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //check for sleep, freeze, confuse, paralyze, flinch before letting mon move
    if (foeparty[ combatants[1].whichpartymemberami ].statusint == 2) //if you're frozen, roll 30%chancce of unfreezing
    {
        int unfreeze = 1 + rand() % 10;
        if (unfreeze > 7) //if they unfreeze
        {
            printf("%s was unfrozen!\n", combatants[1].individualdata.name);
            attacker.individualdata.statusint = 0;
            foeparty[ combatants[1].whichpartymemberami ].statusint = 0;
        }
        else //if they dont
        {
            printf("%s is frozen solid!\n", combatants[1].individualdata.name);
            return 11;
        }
    }

    if (combatants[1].confused == 1) //if confused
    {
        printf("%s is confused!\n", combatants[1].individualdata.name);
        int hiturself = 1 + rand() % 10;
        int unconfuse = 1 + rand() % 10;
        if (unconfuse >= 6) //if they snap out of confusion
        {
            attacker.confused = 0;
            printf("%s snapped out of confusion!\n", combatants[1].individualdata.name);
            hiturself = 1;
        }
        if (hiturself >= 7) //if hit self in confusion, return
        {
            printf("%s hurt itself in confusion!\n", combatants[1].individualdata.name);
            deplete(&foeparty[ combatants[1].whichpartymemberami ].currenthp, eighth, attacker.individualdata);
            return 12;
        } //else, just continue function
    }

    if (combatants[1].flinched == 1) //if flinched
    {
        printf("... unfortunately, %s flinched and it's attack failed!\n", combatants[1].individualdata.name);
        attacker.flinched = 0;
        return 13;
    }

    if (foeparty[ combatants[1].whichpartymemberami ].statusint == 3) //if you're asleep, roll 30% chancce of waking up
    {
        int unfreeze = 1 + rand() % 10;
        if (unfreeze > 7) //if they wake up
        {
            printf("%s woke up!\n", combatants[1].individualdata.name);
            attacker.individualdata.statusint = 0;
            foeparty[ combatants[1].whichpartymemberami ].statusint = 0;
        }
        else //if they dont
        {
            printf("... but %s is fast asleep!\n", combatants[1].individualdata.name);
            return 11;
        }
    }

    if (foeparty[ combatants[1].whichpartymemberami ].statusint == 5) //if you're paralyzed, roll 30% chancce of no move
    {
        printf("%s is paralyzed!\n", combatants[1].individualdata.name);
        int unfreeze = 1 + rand() % 10; //if they paralyzed
        if (unfreeze > 7)
        {
            printf("%s can't move!\n", combatants[1].individualdata.name);
            return 14;
        }//if not, continue with turn as usual
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int recorsucc = (-1 * moveused.recoilordrain * dmg / 8); //calculates recoil or vampiring hp
    if (moveused.recoilordrain != 0) //if the move has recoil or drain:::
    {
        int urhp = deplete(&foeparty[ combatants[0].whichpartymemberami ].currenthp, recorsucc, attacker.individualdata); //applies recoil or succ
        if (*foecombathppointer <= 0)
        {
            printf("\nThe opposing Pokemon fainted!\n");
            foeparty[ combatants[1].whichpartymemberami ].status = "Fainted";
            foeparty[ combatants[1].whichpartymemberami ].statusint = 6;
            *foefaint = 1;

        //check if all opposing mons are fainted
            int airandomswitch = -1;
            int healthymons = 0;
            for (int i = 0; i < 3; i ++)
            {
                if (foeparty[i].statusint != 6)
                {
                    healthymons++;
                }
            }
            if (healthymons == 0) //if they are, it's all over kid
            {
                *isoverpointer = 1;
                printf("\nYou defeated all enemy Pokemon! You won the battle!");
                return 0;
            }
            else //if not all fainted, ai sends out random not-fainted mon
            {
                *isoverpointer = 0;
                do
                {
                    airandomswitch = rand() % 3;
                }while(foeparty[ airandomswitch ].statusint == 6);
            }

            //fill out combatants with new mon
            combatants[1].individualdata = foeparty[airandomswitch];
            combatants[1].dexnumber = foeparty[airandomswitch].dexno; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
            combatants[1].nickname = foeparty[airandomswitch].nickname;
            combatants[1].atkmod = 0;
            combatants[1].defmod = 0;
            combatants[1].spatkmod = 0;
            combatants[1].spdefmod = 0;
            combatants[1].speedmod = 0;
            combatants[1].evasionmod = 0;
            combatants[1].accuracymod = 0;
            combatants[1].protectcounter = 0;
            combatants[1].status = foeparty[airandomswitch].status;
            combatants[1].statusint = foeparty[airandomswitch].statusint;
            combatants[1].confused = 0;
            combatants[1].disabled = 0;
            combatants[1].toxiccounter = 0;
            combatants[1].leechseeded = 0;
            combatants[1].flinched = 0;
            combatants[1].substitute = 0;
            combatants[1].substitutehp = 0;
            combatants[1].hyperskyskull = "None";
            combatants[1].invulnerable = 0;
            combatants[1].caughtinwrap = 0;
            combatants[1].cursed = 0;
            combatants[1].biding = 0;
            combatants[1].bidedamagetracker = 0;
            combatants[1].countering = 0;
            combatants[1].counterdamagetracker = 0;
            combatants[1].thrashcounter = 0;
            combatants[1].whichpartymemberami = airandomswitch;

            printf("Foe sent out %s!\n", foeparty[airandomswitch].name);
            *foecombathppointer = foeparty[airandomswitch].currenthp;
            *foecombatspeedpointer = foeparty[airandomswitch].speed;
        }

        //if ur hp hits 0

    }
    int finhp = foedeplete(&party[ combatants[0].whichpartymemberami ].currenthp, dmg, defender.individualdata);



    //run status checks for secondary effects of moves

    ///////////////////////////////////////////////////////////////////////


    if (moveused.effectchance != 0 && dmg > 0) //if theres an effect chance,
    {
        if (strcmp(moveused.effect, "Burn") == 0) //if effect is burn
        {
            if (defender.individualdata.statusint == 0) //and theyre healthy
            {
                int rollchanceb = 1 + rand() % 100;
                if (rollchanceb < moveused.effectchance)
                {
                    defender.individualdata.statusint = 1; //burn them                 //THIS LINE DOESNT WORK
                    party[ combatants[0].whichpartymemberami ].statusint = 1;       //THIS LINE DOES IDK WHY
                    printf("%s was burned!\n", defender.individualdata.name);
                }
            }
        }
        else if (strcmp(moveused.effect, "Poison") == 0)
        {
            if (defender.individualdata.statusint == 0)
            {
                int rollchancep = 1 + rand() % 100;
                if (rollchancep < moveused.effectchance)
                {
                    defender.individualdata.statusint = 4;
                    party[ combatants[0].whichpartymemberami ].statusint = 4;
                    printf("%s was poisoned!\n", defender.individualdata.name);
                }
            }
        }
        else if (strcmp(moveused.effect, "Toxic") == 0)
        {
            if (defender.individualdata.statusint == 0)
            {
                int rollchancet = 1 + rand() % 100;
                if (rollchancet < moveused.effectchance)
                {
                    defender.individualdata.statusint = 4;
                    party[ combatants[0].whichpartymemberami ].statusint = 4;
                    printf("%s was badly poisoned!\n", defender.individualdata.name);
                    combatants[0].toxiccounter = 1;
                    defender.toxiccounter = 1;
                }
            }
        }
        else if (strcmp(moveused.effect, "Freeze") == 0)
        {
            if (defender.individualdata.statusint == 0)
            {
                int rollchancef = 1 + rand() % 100;
                if (rollchancef < moveused.effectchance)
                {
                    defender.individualdata.statusint = 2;
                    party[ combatants[0].whichpartymemberami ].statusint = 2;
                    printf("%s was frozen solid!\n", defender.individualdata.name);
                }
            }
        }
        else if (strcmp(moveused.effect, "Paralyze") == 0)
        {
            if (defender.individualdata.statusint == 0)
            {
                int rollchancepz = 1 + rand() % 100;
                if (rollchancepz < moveused.effectchance)
                {
                    defender.individualdata.statusint = 5;
                    party[ combatants[0].whichpartymemberami ].statusint = 5;
                    printf("%s was paralyzed!\n", defender.individualdata.name);
                }
            }
        }
        else if (strcmp(moveused.effect, "Sleep") == 0)
        {
            if (defender.individualdata.statusint == 0)
            {
                int rollchances = 1 + rand() % 100;
                if (rollchances < moveused.effectchance)
                {
                    defender.individualdata.statusint = 3;
                    party[ combatants[0].whichpartymemberami ].statusint = 3;
                    printf("%s fell asleep!\n", defender.individualdata.name);
                }
            }
        }
        else if (strcmp(moveused.effect, "-spdef") == 0)
        {
            int rollspdef = 1 + rand() % 100;
            if (rollspdef < moveused.effectchance)
            {
                defender.spdefmod = defender.spdefmod - 1;
                combatants[0].spdefmod = combatants[0].spdefmod - 1;
                printf("%s had it's special defense lowered!\n", defender.individualdata.name);
                if (defender.spdefmod < -6)
                {
                    defender.spdefmod = -6;
                }
            }
        }
        else if (strcmp(moveused.effect, "-def") == 0)
        {
            int rolldef = 1 + rand() % 100;
            if (rolldef < moveused.effectchance)
            {
                defender.defmod = defender.defmod - 1;
                combatants[0].defmod = combatants[0].defmod - 1;
                printf("%s had it's defense lowered!\n", defender.individualdata.name);
                if (defender.defmod < -6)
                {
                    defender.defmod = -6;
                }
            }
        }
        else if (strcmp(moveused.effect, "-atk") == 0)
        {
            int rollatk = 1 + rand() % 100;
            if (rollatk < moveused.effectchance)
            {
                defender.atkmod = defender.atkmod - 1;
                combatants[0].atkmod = combatants[0].atkmod - 1;
                printf("%s had it's attack lowered!\n", defender.individualdata.name);
                if (defender.atkmod < -6)
                {
                    defender.atkmod = -6;
                }
            }
        }
        else if (strcmp(moveused.effect, "Flinch") == 0)
        {
            int rollflinch = 1 + rand() % 100;
            if (rollflinch < moveused.effectchance)
            {
                defender.flinched = 1;
                combatants[0].flinched = 1;
            }
        }
        else if (strcmp(moveused.effect, "Confuse") == 0)
        {
            int rollcon = 1 + rand() % 100;
            if (rollcon < moveused.effectchance)
            {
                defender.confused = 1;
                combatants[0].confused = 1;
                printf("%s became confused!\n", defender.individualdata.name);
            }
        }
    }



    /////////////////////////////////////////////////////////////////////////
    //finhp = foedeplete(&party[ combatants[0].whichpartymemberami ].currenthp, dmg, defender.individualdata);
    if (*combathppointer <= 0) //note in above line i don't use &defender.ichp but instead use &oppparty[0].ichp. when i use the former, pika's hp never ectually changes. it will say its hp went down, but then the next fight phase shows that the hp remains unchanged. otherwise i'd just use the former and have one function, not func() and oppfunc()
    {
        *userfaint = 1;
        printf("\nYour Pokemon fainted!\n");
        party[ combatants[0].whichpartymemberami ].status = "Fainted";
        party[ combatants[0].whichpartymemberami ].statusint = 6;


        int healthymons = 0; //counts number of non-fainted monsters
        for (int i = 0; i < 3; i ++)
        {
            if (party[i].currenthp != 0)
            {
                healthymons++;
            }
        }
        if (healthymons == 0) //if they're all fainted, it's all over kid
        {
            *isoverpointer = 1;
            printf("You've been defeated!\n");
            return 2;
        }
        else     //if theyre not all fainted, pick from one that's not
        {
            *isoverpointer = 0; //battle isnt over
            printf("\nWhich party member woud you like to switch to?\n");

            for (int i = 0; i < 3; i++) //list party members that aren't on the field
            {
                if (combatants[0].whichpartymemberami != i && party[i].currenthp != 0) //if not mon thats out and not mon that has 0hp
                {
                    printf("Press %i to switch to %s, lv%i\n", i, party[i].name, party[i].lvl);
                }
                else
                {
                    int filllerrr = 32;
                }
            }

            int whichmem = combatants[0].whichpartymemberami; //just a contraction
            int choice; //declaration for a do while
            int *choicepointer = &choice;
            do //ask player which party member to switch to
            {
                choice = get_int("Type choice and press enter:\n");
                *choicepointer = choice;
            }while(choice == combatants[0].whichpartymemberami || choice < 0 || choice > 3);

            //ok here i need to keep status. currenthp uses pointers to party/foeparty so should already be updated
            party[whichmem].status = combatants[0].status;
            party[whichmem].statusint = combatants[0].statusint;


           //alright now i need to load party[choice] into combatants[0]
            combatants[0].individualdata = party[choice];
            combatants[0].dexnumber = party[choice].dexno; //inherit from pokemon struct. combatants[0].dexnumber = combatants[0].individualdata.dexno
            combatants[0].nickname = party[choice].nickname;
            combatants[0].atkmod = 0;
            combatants[0].defmod = 0;
            combatants[0].spatkmod = 0;
            combatants[0].spdefmod = 0;
            combatants[0].speedmod = 0;
            combatants[0].evasionmod = 0;
            combatants[0].accuracymod = 0;
            combatants[0].protectcounter = 0;
            combatants[0].status = party[choice].status;
            combatants[0].statusint = party[choice].statusint;
            combatants[0].confused = 0;
            combatants[0].disabled = 0;
            combatants[0].toxiccounter = 0;
            combatants[0].leechseeded = 0;
            combatants[0].flinched = 0;
            combatants[0].substitute = 0;
            combatants[0].substitutehp = 0;
            combatants[0].hyperskyskull = "None";
            combatants[0].invulnerable = 0;
            combatants[0].caughtinwrap = 0;
            combatants[0].cursed = 0;
            combatants[0].biding = 0;
            combatants[0].bidedamagetracker = 0;
            combatants[0].countering = 0;
            combatants[0].counterdamagetracker = 0;
            combatants[0].thrashcounter = 0;
            combatants[0].whichpartymemberami = choice;



            //printf("choice is %i\n", choice);
            //printf("combatants[0].whichpartymemberami = %i\n", combatants[0].whichpartymemberami);

            party[combatants[0].whichpartymemberami].currenthp = party[choice].currenthp;
            *combatspeedpointer = party[choice].speed;

            //printf("current hp of party[choice] is %i\n", party[choice].currenthp);
            //printf("party[combatants[0].whichpartymemberami].currenthp = %i\n", party[combatants[0].whichpartymemberami].currenthp);

            *userswitchpointer = 1;
            *userfaint = 0;


            printf("\nCome back, %s. Go, %s!\n\n", party[whichmem].name, party[choice].name);
        }
    }

    //implement secondary effects of moves
    //endoffoefight() bookmark

    return 1;
}


















//for attack, below, we need to make a conditional that takes a whole other route if the move is a status move.

int attack(struct combatmon attacker, struct combatmon defender, struct move movestruct, int STAB) //attack needs to take type effectiveness as an arguement
{
    //obv we need to take move type, user type 1 and type 2, opp types 1 and 2 as arguments for this function
    //also, if move is special use special stats, if move is physical, use physical stats
    float d = 0;
    if (movestruct.physicalorspecial == 0)
    { //physical damage and atk mods
        d = 0.7 * (3 * 7 / 20 * (movestruct.basepower / 40) * attackchange(combatants[0].atkmod, combatants[0].individualdata.statusint) * (((5 * combatants[0].individualdata.lvl + 2) * combatants[0].individualdata.atk / (combatants[1].individualdata.def * statchange(combatants[1].defmod)) / 10) + 2));
    }
    else
    {
        d = 0.7 * (3 * 7 / 20 * (movestruct.basepower / 40) * statchange(combatants[0].spatkmod) * (((5 * combatants[0].individualdata.lvl + 2) * combatants[0].individualdata.spatk / (combatants[1].individualdata.spdef * statchange(combatants[1].spdefmod)) / 10) + 2));
    }

    int d2 = (STAB * d) / 2;
    //d2 = d2 * type effective * type effective2 divided by 4. if neutral effective, both typechart entries = 2, so we mult by 4. this is why i divde by 4 at end
    d2 = d2 * typechart[movestruct.typeint][defender.individualdata.typeint1] * typechart[movestruct.typeint][defender.individualdata.typeint2] / 4;


    //super effective messages
    if ( typechart[movestruct.typeint][defender.individualdata.typeint1] * typechart[movestruct.typeint][defender.individualdata.typeint2] > 4)
    {
        printf("It's super effective!\n");
    }
    else if ( typechart[movestruct.typeint][defender.individualdata.typeint1] * typechart[movestruct.typeint][defender.individualdata.typeint2] < 4 && typechart[movestruct.typeint][defender.individualdata.typeint1] * typechart[movestruct.typeint][defender.individualdata.typeint2] != 0)
    {
        printf("It's not very effective.\n");
    }
    else if (typechart[movestruct.typeint][defender.individualdata.typeint1] * typechart[movestruct.typeint][defender.individualdata.typeint2] == 0)
    {
        printf("...but it had no effect!\n");
    }
    else
    {
        int fillerint = 69;
    }

    //accuracy check
    int acccheck = 1 + rand() % 100;
    if ( (movestruct.acc * statchange(attacker.accuracymod) < acccheck))
    {
        printf("The attack missed!\n");
        d2 = 0;
    }
    else
    {
        int rollforeffect = 1 + rand() % 100; ////////////////////here is where status effect from moves happen. need to do one for status string not status int
        if (movestruct.effectchance > rollforeffect)
        {
            //apply status to opponent
            if (movestruct.effectint > 0 && movestruct.effectint < 6 && *defender.individualdata.statusintpointer == 0) //if they're inflicting major status, and the defender doesn't already have a status condition
            {
                defender.statusint = movestruct.effectint; //apply to combatmon for in-battle stuff
                *defender.individualdata.statusintpointer = movestruct.effectint; //apply to individual struct so status stays on switch out
                printf("The attack inflicted %s!\n", movestruct.effect);
            }

        }
    }
    //else if acccheck < movestruct.acc, roll for status

    return d2;
}

int foeattack(struct combatmon attacker, struct combatmon defender, struct move movestruct, int STAB) //attack needs to take type effectiveness as an arguement
{
    //obv we need to take move type, user type 1 and type 2, opp types 1 and 2 as arguments for this function
    //also, if move is special use special stats, if move is physical, use physical stats
    float d = 0;
    if (movestruct.physicalorspecial == 0)
    { //physical damage and atk mods
        d = 0.7 * (3 * 7 / 20 * (movestruct.basepower / 40) * attackchange(combatants[1].atkmod, combatants[1].individualdata.statusint) * (((5 * combatants[1].individualdata.lvl + 2) * combatants[1].individualdata.atk / (combatants[0].individualdata.def * statchange(combatants[0].defmod)) / 10) + 2));
    }
    else
    {
        d = 0.7 * (3 * 7 / 20 * (movestruct.basepower / 40) * statchange(combatants[1].spatkmod) * (((5 * combatants[1].individualdata.lvl + 2) * combatants[1].individualdata.spatk / (combatants[0].individualdata.spdef * statchange(combatants[0].spdefmod)) / 10) + 2));
    }

    int d2 = (STAB * d) / 2;
    //d2 = d2 * type effective * type effective2 divided by 4. if neutral effective, both typechart entries = 2, so we mult by 4. this is why i divde by 4 at end
    d2 = d2 * typechart[movestruct.typeint][defender.individualdata.typeint1] * typechart[movestruct.typeint][defender.individualdata.typeint2] / 4;


    //super effective messages
    if ( typechart[movestruct.typeint][defender.individualdata.typeint1] * typechart[movestruct.typeint][defender.individualdata.typeint2] > 4)
    {
        printf("It's super effective!\n");
    }
    else if ( typechart[movestruct.typeint][defender.individualdata.typeint1] * typechart[movestruct.typeint][defender.individualdata.typeint2] < 4 && typechart[movestruct.typeint][defender.individualdata.typeint1] * typechart[movestruct.typeint][defender.individualdata.typeint2] != 0)
    {
        printf("It's not very effective.\n");
    }
    else if (typechart[movestruct.typeint][defender.individualdata.typeint1] * typechart[movestruct.typeint][defender.individualdata.typeint2] == 0)
    {
        printf("...but it had no effect!\n");
    }
    else
    {
        int fillerint = 69;
    }

    //accuracy check
    int acccheck = 1 + rand() % 100;
    if ( (movestruct.acc * statchange(attacker.accuracymod) < acccheck))
    {
        printf("The attack missed!\n");
        d2 = 0;
    }
    else
    {
        int rollforeffect = 1 + rand() % 100; ////////////////////here is where status effect from moves happen. need to do one for status string not status int
        if (movestruct.effectchance > rollforeffect)
        {
            //apply status to opponent
            if (movestruct.effectint > 0 && movestruct.effectint < 6 && *defender.individualdata.statusintpointer == 0) //if they're inflicting major status, and the defender doesn't already have a status condition
            {
                defender.statusint = movestruct.effectint; //apply to combatmon for in-battle stuff
                *defender.individualdata.statusintpointer = movestruct.effectint; //apply to individual struct so status stays on switch out
                printf("The attack inflicted %s!\n", movestruct.effect);
            }

        }
    }
    //else if acccheck < movestruct.acc, roll for status

    return d2;
}












int deplete(int *hppoint, int damage, struct pokemon victim) //this is for when you are damaging opponent. their hp pointer will get passed in by fight()
{
    if (damage < 0) //if healing
    {
        int intugor = damage * -1;
        printf("%i HP was restored!\n", intugor);
    }
    if (damage >= 0)
    {
        printf("It did %i damage!\n", damage);

    }


    printf("%s's HP went from %i ", victim.name, *hppoint);
    *hppoint = *hppoint - damage;

    if (*hppoint <= 0) //preventss hp from going below 0
    {
        *hppoint = 0;
        //status int pointer is 6 for faint?
    }
    if (*hppoint > victim.maxhp) //prevents hp from going over max
    {
        *hppoint = victim.maxhp;
    }
    printf("to %i!\n", *hppoint);
    *foecombathppointer = *hppoint;
    return *hppoint;
}

int foedeplete(int *hppoint, int damage, struct pokemon victim) //this is for when the foe is damaging u. ur hp pointer will get passed in by fight()
{
    if (damage < 0)
    {
        int intugor = damage * -1;
        printf("%i HP was restored!\n", intugor);
    }
    if (damage >= 0)
    {
        printf("It did %i damage!\n", damage);
    }



    printf("%s's HP went from %i ", victim.name, *hppoint);
    *hppoint = *hppoint - damage;

    if (*hppoint <= 0) //preventss hp from going below 0
    {
        *hppoint = 0;
        //status pointer is faint?
    }
    if (*hppoint > victim.maxhp) //prevents hp from going over max
    {
        *hppoint = victim.maxhp;
    }

    printf("to %i!\n", *hppoint);
    *combathppointer = *hppoint;
    return *hppoint;
}



float speedchange(int statmod, int statusint)
{
    float a = statmod;
    if (statusint == 5) //if paralyzed
    {
        a = a / 2;
    }
    if (statmod == 0)
    {
        if (statusint == 5)
        {
            return 0.5;
        }
        else
        {
            return 1;
        }
    }
    else if (statmod > 0)
    {
        return (1 + (a / 2));
    }
    else
    {
        return (3 / (3 - a));
    }
}

float attackchange(int statmod, int statusint)
{
    float mult = 1;
    float a = statmod;
    if (statusint == 1) //if burned
    {
        a = a / 2;
        mult = mult/2;
    }
    if (statmod == 0)
    {
        if (statusint == 1)
        {
            return 0.5;
        }
        else
        {
            return 1;
        }
    }
    else if (statmod > 0)
    {
        return (1 + (a / 2));
    }
    else //statmod < 0
    {
        return (3 / (3 - a));
    }
}

float statchange(int statmod)
{
    float a = statmod;
    if (statmod == 0) //if no stat mods
    {
        return 1; //return 1x multiplier
    }
    else if (statmod > 0) //if positive
    {
        return (1 + (a / 2));
    }
    else if (statmod < 0)
    {
        return (3 / (3 - a));
    }
    else
    {
        return 1;
    }
}

string statinttostring(int g)
{
    string answer;
    if (g == 1)
    {
        answer = "Burned";
    }
    else if (g == 2)
    {
        answer = "Frozen";
    }
    else if (g == 3)
    {
        answer = "Asleep";
    }
    else if (g == 4)
    {
        answer = "Poisoned";
    }
    else if (g == 5)
    {
        answer = "Paralyzed";
    }
    else
    {
        answer = "None";
    }
    return answer;
}

void userselection1(void)
{
    string usermon1 = get_string("Type the name of your first party member and hit Enter:\n");
    for (int i = 0; i < 151; i++)
    {
        if (strcmp(pokedex[i].name, usermon1) == 0)
        {
            *urmon1ptr = i;
            printf("\n%s has been added to your team!\n", pokedex[i].name);
            return;
        }
        else if (i >= 150)
        {
            printf("Pokemon not recognized. Please try again. \n--All fully-evolved Pokemon from the original 151 are available, minus Ditto, plus Pikachu.\n--Make sure only the first letter is capitalized!\n");
            userselection1();
        }
    }
}

void userselection2(void)
{
    string usermon2 = get_string("Type the name of your second party member and hit Enter:\n");
    for (int i = 0; i < 151; i++)
    {
        if (strcmp(pokedex[i].name, usermon2) == 0)
        {
            *urmon2ptr = i;
            printf("\n%s has been added to your team.\n", pokedex[i].name);
            return;
        }
        else if (i >= 150)
        {
            printf("Pokemon not recognized. Please try again. \n--All fully-evolved Pokemon from the original 151 are available, minus Ditto, plus Pikachu.\n--Make sure only the first letter is capitalized!\n");
            userselection2();
        }
    }
}

void userselection3(void)
{
    string usermon3 = get_string("Type the name of your third party member and hit Enter:\n");
    for (int i = 0; i < 151; i++)
    {
        if (strcmp(pokedex[i].name, usermon3) == 0)
        {
            *urmon3ptr = i;
            printf("\n%s has been added to your team.\n", pokedex[i].name);
            return;
        }
        else if (i >= 150)
        {
            printf("Pokemon not recognized. Please try again. \n--All fully-evolved Pokemon from the original 151 are available, minus Ditto, plus Pikachu.\n--Make sure only the first letter is capitalized!\n");
            userselection3();
        }
    }
}



void foeselection1(void)
{
    string foemon1 = get_string("Type the name of your foe's first party member and hit Enter:\n");
    for (int i = 0; i < 151; i++)
    {
        if (strcmp(pokedex[i].name, foemon1) == 0)
        {
            *theirmon1ptr = i;
            printf("\n%s has been added to foe's team.\n", pokedex[i].name);
            return;
        }
        else if (i >= 150)
        {
            printf("Pokemon not recognized. Please try again. \n--All fully-evolved Pokemon from the original 151 are available, minus Ditto, plus Pikachu.\n--Make sure only the first letter is capitalized!\n");
            foeselection1();
        }
    }
}

void foeselection2(void)
{
    string foemon2 = get_string("Type the name of your foe's second party member and hit Enter:\n");
    for (int i = 0; i < 151; i++)
    {
        if (strcmp(pokedex[i].name, foemon2) == 0)
        {
            *theirmon2ptr = i;
            printf("\n%s has been added to foe's team.\n", pokedex[i].name);
            return;
        }
        else if (i >= 150)
        {
            printf("Pokemon not recognized. Please try again. \n--All fully-evolved Pokemon from the original 151 are available, minus Ditto, plus Pikachu.\n--Make sure only the first letter is capitalized!\n");
            foeselection2();
        }
    }
}

void foeselection3(void)
{
    string foemon3 = get_string("Type the name of your foe's third party member and hit Enter:\n");
    for (int i = 0; i < 151; i++)
    {
        if (strcmp(pokedex[i].name, foemon3) == 0)
        {
            *theirmon3ptr = i;
            printf("\n%s has been added to foe's team.\n", pokedex[i].name);
            return;
        }
        else if (i >= 150)
        {
            printf("Pokemon not recognized. Please try again. \n--All fully-evolved Pokemon from the original 151 are available, minus Ditto, plus Pikachu.\n--Make sure only the first letter is capitalized!\n");
            foeselection3();
        }
    }
}