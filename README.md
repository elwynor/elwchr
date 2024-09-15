<pre>
   
    _______        _        ___               __  _         
   / ___/ /  ___ _(_)__    / _ \___ ___ _____/ /_(_)__  ___ 
  / /__/ _ \/ _ `/ / _ \  / , _/ -_) _ `/ __/ __/ / _ \/ _ \
  \___/_//_/\_,_/_/_//_/ /_/|_|\__/\_,_/\__/\__/_/\___/_//_/
                                          
                        version 1.1
           Copyright (c) 2024 Elwynor Technologies
</pre>

## WHAT IS CHAIN REACTION?

 Chain Reaction is a fun word game. You start with a word, then start guessing
 each word that follows.  All of the words are related to the word directly 
 above and directly below it.  For example:
   SPACE -> STAR -> SUN -> LIGHT
 
## GAMEPLAY
 
 The game is simple - when you enter the game, you are given a puzzle, and you
 must guess each word.  You only have a certain number of misses before the 
 game ends.  You gain points for each correct guess, and for solving puzzles.
 After three puzzles, you gain a bonus round.  If you guess all the words
 without making 6 mistakes, you gain 2000 bonus points.
 
 There is a top ten high score list available from the main menu.

## COMMANDS

 Simply type your word guesses in gameplay mode.  If you are wrong, you will
 be given a hint.
 
 At the main menu, you may (V)iew high scores, read (I)nstructions, (P)lay, or
 e(X)it.  
 
 Note that Sysops may import a puzzle file (seven words per line, separated by
 commas, with no spaces) by using the (L)oad option.
 
## INSTALLATION AND CONFIGURATION
 
 Simply unzip the archive to your BBS server directory,
 add the module to your menu, configure the MSG file to your liking, 
 and start the BBS! It's that easy! 

## MODULE HISTORY
 
 Chain Reaction was written by "Alfred" (Ralph Trynor) of Wizard Software 
 in August of 1994, for Major BBS/Worldgroup DOS.
 
 Ralph led Wizard Software, GraphicWares, and Major Programming Group
 (MPG) ISVs. GraphicWares and Wizard Software marketed their products 
 together as GraphicWares/Wizard Software (GWW), and had Jeff Lawton join
 Ralph in working on software for The Major BBS and Worldgroup.

 GWW and MPG were sold to WilderLands Software in the mid-to-late 1990s. 

 Elwynor Technologies acquired the module in 2004 and ported it to 
 Worldgroup 3.2 in February 2006. In September 2024, it was ported to
 Major BBS V10 and released as open source.
 
## LICENSE

 This project is licensed under the AGPL v3. Additional terms apply to 
 contributions and derivative projects. Please see the LICENSE file for 
 more details.

## CONTRIBUTING

 We welcome contributions from the community. By contributing, you agree to the
 terms outlined in the CONTRIBUTING file.

## CREATING A FORK

 If you create an entirely new project based on this work, it must be licensed 
 under the AGPL v3, assign all right, title, and interest, including all 
 copyrights, in and to your fork to Rick Hadsall and Elwynor Technologies, and 
 you must include the additional terms from the LICENSE file in your project's 
 LICENSE file.

## COMPILATION

 This is a Worldgroup 3.2 / Major BBS v10 module. It's compiled using Borland
 C/C++ 5.0 for Worldgroup 3.2. If you have a working Worldgroup 3.2 development
 kit, a simple "make -f ELWCHR" should do it! For Major BBS v10, import this
 project folder in the isv/ subtree of Visual Studio 2022, right click the
 project name and choose build! When ready to build for "release", ensure you
 are building for release.

## PACKING UP

 The DIST folder includes all of the items that should be packaged up in a 
 ELWCHR.ZIP. When unzipped in a Worldgroup 3.2 or Major BBS V10 installation 
 folder, it "installs" the module.
