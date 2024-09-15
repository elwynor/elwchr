/*****************************************************************************
 *                                                                           *
 *    Chain Reaction                                                   v1.1  *
 *                                                                           *
 *    MBBS6/WG1/WG2: Ralph Trynor August 28, 1994                            *
 *    WG3.2 port: Rick Hadsall February 22, 2006                             *
 *    V10/WG32 single kit: Rick Hadsall September 15, 2024                   *
 *                                                                           *
 * Copyright (C) 2024 Rick Hadsall.  All Rights Reserved.                    *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as published  *
 * by the Free Software Foundation, either version 3 of the License, or      *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.     *
 *                                                                           *
 * Additional Terms for Contributors:                                        *
 * 1. By contributing to this project, you agree to assign all right, title, *
 *    and interest, including all copyrights, in and to your contributions   *
 *    to Rick Hadsall and Elwynor Technologies.                              *
 * 2. You grant Rick Hadsall and Elwynor Technologies a non-exclusive,       *
 *    royalty-free, worldwide license to use, reproduce, prepare derivative  *
 *    works of, publicly display, publicly perform, sublicense, and          *
 *    distribute your contributions                                          *
 * 3. You represent that you have the legal right to make your contributions *
 *    and that the contributions do not infringe any third-party rights.     *
 * 4. Rick Hadsall and Elwynor Technologies are not obligated to incorporate *
 *    any contributions into the project.                                    *
 * 5. This project is licensed under the AGPL v3, and any derivative works   *
 *    must also be licensed under the AGPL v3.                               *
 * 6. If you create an entirely new project (a fork) based on this work, it  *
 *    must also be licensed under the AGPL v3, you assign all right, title,  *
 *    and interest, including all copyrights, in and to your contributions   *
 *    to Rick Hadsall and Elwynor Technologies, and you must include these   *
 *    additional terms in your project's LICENSE file(s).                    *
 *                                                                           *
 * By contributing to this project, you agree to these terms.                *
 *                                                                           *
 *****************************************************************************/


/* Include files */
#include "gcomm.h"
//include "brkthu.h"
#include "majorbbs.h"
#include "elwchr.h"

#define MAXPLY 255

/* Procedure declarations */
VOID EXPORT init__elwchr(VOID);
GBOOL chmain(VOID);
VOID abend(VOID);
VOID savehi(VOID);
VOID chterm(VOID);
VOID delusr(CHAR *usernm);
VOID arxy(INT x, INT y);

struct module chmd={         /*  Module interface block                    */
     "",                     /*  Name used to refer to this module         */
     NULL,                   /*  User Logon supplemental routine           */
     chmain,                 /*  Input routine if selected                 */
     dfsthn,                 /*  Status-Input routine if selected          */
     NULL,                   /*  "injoth" routine for this module          */
     NULL,                   /*  User logoff supplemental routine          */
     abend,                  /*  Hangup Lost carrier routine               */
     savehi,                 /*  Midnight Cleanup routine                  */
     delusr,                 /*  delete-account routine                    */
     chterm                  /*  finish-up (system shut down) routine      */
};

typedef struct {
  CHAR aword[20];
} CWORD;

typedef struct {
  CHAR user[30];
  CHAR userscore[8];
  CHAR currentsent[5];
 } SCORES;

typedef struct {
   CHAR puzlno[6];
   CHAR puzlstr[80];
 } PUZZLE;

typedef struct {
    PUZZLE puzl;
    SCORES dbrec;
    SHORT wmissed,newplayer,cline,cwscval;
    SHORT ccpos;
    USHORT csent;
    ULONG score,tdbest;
    CWORD pwords[10];
  } VDABLK;

static PUZZLE *pzlptr;
static VDABLK *zvda;
static SCORES *dbptr;
struct usracc *uaptr;
static INT modnum;
static INT missletter;

static FILE *kwtext;
static HMCVFILE msgblk;

static PUZZLE pzitemp;
static DFAFILE *plyf,*hiscf,*kwfile;
static UINT maxrec;
static CHAR tempstr[120];
static SCORES topten[11];

//  #define zvda ((struct vdarec *) vdaptr)
#define ucw zvda->cline
#define cwval zvda->cwscval

#define VERSION "1.1" // v1.1: BBSV10 port, BBSV10/WG32 single kit

VOID EXPORT init__elwchr(VOID)
{
  INT iostat,j;

  stzcpy(chmd.descrp,gmdnam("ELWCHR.MDF"),MNMSIZ);
  modnum=register_module(&chmd);
  
  plyf=dfaOpen("ELWCHRPL.DAT",sizeof(SCORES),NULL);
  hiscf=dfaOpen("ELWCHRHI.DAT",sizeof(SCORES),NULL);
  kwfile=dfaOpen("ELWCHRPZ.DAT",sizeof(PUZZLE),NULL);

  // assign vars from the msg file..........................
  msgblk=opnmsg("ELWCHR.MCV");
  missletter=numopt(LETTERS,1,250);

  dclvda(sizeof(VDABLK));
  
  /* init the high score stuff */
  dfaSetBlk(hiscf);
  iostat=dfaStepLO(topten);
  if (iostat!=1) {      /* new high score list */
    strcpy(topten[0].user,"Nobody Yet!");
    strcpy(topten[0].userscore,"0");
    for (j=0; j < 10; j++) { 
       dfaInsert(topten); 
    }
  }
  for (j=1; j<10; j++) { 
    dfaStepNX(&topten[j]); 
  }
  
  /* see if word dbase needs creating. This is a special case for me only. */
  dfaRstBlk();
  dfaSetBlk(kwfile);
  iostat=dfaStepLO(&pzitemp);
  
  if (iostat!=1) {  /* create initial record */
    strcpy(pzitemp.puzlno,"0");
    dfaInsert(&pzitemp);
    shocst("ELW Chain Reaction Import DB","Import Required (1)");
    maxrec=0;
  } else {
    maxrec=atoi(pzitemp.puzlno);
    if (!maxrec)
      shocst("ELW Chain Reaction Import DB","CR Import Required (2)");
  }

  dfaRstBlk();
  shocst(spr("ELW Chain Reaction v%s",VERSION),"(C) Copyright 2004-2024 Elwynor Technologies - www.elwynor.com");
}

VOID arxy(INT x,INT y)
{ 
  CHAR xstr[15];
  strcpy(xstr,spr("[%d;%dH",y,x));
  prf(xstr);
}

VOID savehi(VOID)
{ 
  INT j;
  
  dfaSetBlk(hiscf);
  dfaStepLO(&topten[10]);
  dfaUpdate(&topten[0]);
  for (j=1; j<10; j++) {
    dfaStepNX(&topten[10]);
    dfaUpdate(&topten[j]);
  }
  dfaRstBlk();
}

static VOID disphi(VOID)
{ 
  INT j;

  prfmsg(HILIST);
  for (j=0; j<10; j++) {
    prfmsg(HILINE,topten[j].user,topten[j].userscore);
  }
  outprf(usrnum);
}

static VOID uubest(VOID)
{
  zvda->score=zvda->score+cwval;
  zvda->tdbest=zvda->tdbest+cwval;
}

static VOID updhi(VOID)
{ 
  INT hi,hj;

  // remove him from hi score if present
  stzcpy(topten[10].user,"Nobody Yet!",30);
  stzcpy(topten[10].userscore,"0",8);
  for (hi=0; hi<10; hi++) if ( sameas(topten[hi].user,uaptr->userid) ) break;
  
  if (hi<10)                          /* kill his entry */
    while (hi<10) {
      topten[hi]=topten[hi+1];
      hi++;
    }
  
  // now find his spot
  hi=0;
  while (hi<10) {
    if (zvda->score>(ULONG)atol(topten[hi].userscore)) break;
    hi++;
  }
  if (hi<10) {
    hj=10;
    while (hj>hi) {
      topten[hj]=topten[hj-1];
      hj--;
    }
    stzcpy(topten[hi].user,uaptr->userid,30);
    stzcpy(topten[hi].userscore,l2as(zvda->score),8);
  }

  // save the scores    
  savehi();
}

static VOID cmdpos(VOID)
{
  arxy(5,15);
  prf("                    ");
  arxy(5,15);
  prf(" \b");
  outprf(usrnum);
}

static VOID drawscm(VOID)              /*  redraw his score/best today/letters missed fields */
{ 
  CHAR lclstr[10];

  prfmsg(COLOR2);
  strcpy(lclstr,l2as(zvda->score));
  arxy(55,7);
  prf(lclstr);
  strcpy(lclstr,l2as(zvda->tdbest));
  arxy(55,6);
  prf(lclstr);
  outprf(usrnum);
  itoa(zvda->wmissed,lclstr,10);
  arxy(55,9);
  prf(lclstr);
  outprf(usrnum);
}

static VOID savegame(VOID) 
{
  CHAR lstr[10];
  INT svf;

  dfaSetBlk(plyf);
  svf=dfaAcqEQ(dbptr,uaptr->userid,0);
  if (!svf) {
    stzcpy(dbptr->user,uaptr->userid,30);
  }
  stzcpy(dbptr->userscore,l2as(zvda->score),8);
  stzcpy(dbptr->currentsent,itoa(zvda->csent,lstr,10),5);
  if (svf==1) dfaUpdate(dbptr);
  else dfaInsert(dbptr);
  zvda->newplayer=0;
  dfaRstBlk();
}

static VOID initgame(VOID)                   /* setup a game for the guy */
{

 zvda->wmissed=0;
 zvda->tdbest=0;

 // see if a new player
 dfaSetBlk(plyf);
 if (dfaAcqEQ(dbptr,uaptr->userid,0)!=1) {
   zvda->newplayer=1;
   zvda->score=0;
   zvda->csent=2;
   stzcpy(dbptr->user,uaptr->userid,30);
 } else {
   zvda->newplayer=0;
   zvda->score=(ULONG)atol(dbptr->userscore);
   zvda->csent=(USHORT)atoi(dbptr->currentsent);
 }
 dfaRstBlk();
}

static VOID rsetup(VOID) 
{
  INT j,k,l,sf;
  CHAR wstr[20] = { 0 };
  
  /* setup for a round */
  if (zvda->csent > maxrec) {
    //prf("You've seen %d of the %d puzzles\n",zvda->csent,maxrec);
    prfmsg(NOMORE);
    outprf(usrnum);
    usrptr->substt=1;
    return;
  }
  sf=0;
  dfaSetBlk(kwfile);
  j=0;
  while(!sf) {
    itoa(zvda->csent,pzlptr->puzlno,10);
    sf=dfaAcqEQ(pzlptr,pzlptr->puzlno,0);
    if (!sf) {
      prf("Error reading puzzle %u. Notify SysOp\n",zvda->csent);
      shocst("ELW Chain Reaction: Puzzle Read Error",spr("Error reading puzzle #%u",zvda->csent));
      outprf(usrnum);
      zvda->csent++;
      j++;
      if (zvda->csent > maxrec) {
        prfmsg(NOMORE);
        outprf(usrnum);
        usrptr->substt=1;
        dfaRstBlk();
        return;
      }
      if (j==10) {
        prf("Database is damaged. Seek Terminated. Notify SysOp\n");
        shocst("ELW Chain Reaction: DB Damaged","Puzzle database is damaged");
        outprf(usrnum);
        usrptr->substt=1;
        dfaRstBlk();
        return;
      }
    }
  }
  dfaRstBlk();
  j=0;
  k=0;
  while (j<7) {
    l=0;
    while ((pzlptr->puzlstr[k]!=',') && (pzlptr->puzlstr[k]!='\n')) {
      if (l<19) {
        wstr[l]=pzlptr->puzlstr[k];
        l++;
      }
      k++;
      if (k==80) break;
    }
    wstr[l]='\0';
    stzcpy(zvda->pwords[j].aword,wstr,20);
    j++;
    k++;
    if (k>79 && j<7) {
      prf("Puzzle Error at %u. Notify SysOp\n",zvda->csent);
      shocst("ELW Chain Reaction: Puzzle Error",spr("Error within puzzle #%u",zvda->csent));
      outprf(usrnum);
      zvda->csent++;
      usrptr->substt=1;
      return;
    }
  }
  prfmsg(CLRSCR);
  prfmsg(PLSCR,missletter);                      /* draw main screen */
  outprf(usrnum);
  drawscm();                          /* update score fields */
  prfmsg(COLOR2);
  arxy(55,5);
  prf("%s",topten[0].userscore);
  arxy(10,5);
  prf(zvda->pwords[0].aword);
  arxy(10,11);
  prf(zvda->pwords[6].aword);
  k=6;
  prfmsg(COLOR1);
  outprf(usrnum);
  for (j=1; j<6; j++) {
    arxy(10,k);
    l=strlen(zvda->pwords[j].aword);
    while (l>0) {
      prf("_");
      l--;
    }
    outprf(usrnum);
    k++;
  }
  usrptr->substt=2;
  ucw=1;                       /* start with word 2 */
  cwval=(SHORT)strlen(zvda->pwords[1].aword)*10-10;
  zvda->ccpos=1;
  arxy(9,6);
  prfmsg(GTSYM);
  arxy(10,6);
  prf("%c",zvda->pwords[1].aword[0]);
  cmdpos();
}

static VOID newpline(VOID) 
{
  if (ucw==6) {                 // new puzzle
    zvda->csent++;
    cmdpos();
    prfmsg(NEWPZL);
    outprf(usrnum);
    rsetup();
  } else {
    arxy(9,ucw+5);
    prfmsg(GTSYM);
    arxy(10,ucw+5);  // next line
    prf("%c",zvda->pwords[ucw].aword[0]);
    zvda->ccpos=1;
    cwval=(SHORT)strlen(zvda->pwords[ucw].aword)*10-10;
    cmdpos();
  }
}

GBOOL chmain(VOID)                       /* main input handler */
{ 
  INT stpos,iostat;
  GBOOL rcode;
  CHAR mstr1[20];

  rcode=1;
  zvda=(VDABLK *)vdaptr;
  pzlptr=&zvda->puzl;
  dbptr=&zvda->dbrec;
  uaptr=uacoff(usrnum);
  setmbk(msgblk);                    /* prep for prf */
  switch (usrptr->substt) {
    case 0: prfmsg(TITLE);           /* print title screen */
            //prf("There are %d puzzles",maxrec);
            outprf(usrnum);
            usrptr->flags|=NOINJO;
            usrptr->substt=1;
            prfmsg(CHOPTS);          /* what to do */
            break;                   /* wait for response */
    case 1: if (sameas(margv[0],"p")) {
              usrptr->substt=2;
              initgame();
              rsetup();
              break;
            }
            if (sameas(margv[0],"v")) {
              disphi();
              prfmsg(CHOPTS);
              break;
            }
            if (sameas(margv[0],"i")) {
              prfmsg(INSTR);
              prfmsg(CHOPTS);
              break;
            }
            if (sameas(margv[0],"x")) {
              prfmsg(TGABKS);
              usrptr->flags^=NOINJO;
              rcode=0;
              break;
            }
            if (sameas(margv[0],"l") && usrptr->flags&MASTER) {
              usrptr->substt=44;
              prfmsg(ASKIMPF);
              break;
            }
            usrptr->substt=1;
            prfmsg(CHOPTS);          /* what to do */
            break;
    case 2: if (margc==0) {
              cmdpos();
              prfmsg(ABORT);
              usrptr->substt=43;
              outprf(usrnum);
              if (zvda->score>(ULONG)atol(topten[9].userscore)) updhi();
              cwval=0;
              uubest();
              savegame();
              break;
            }
            /* see if what he says matches with what i say */
            rstrin();
            if (sameas(input,zvda->pwords[ucw].aword)) {
              uubest();
              arxy(10,ucw+5);
              prf(zvda->pwords[ucw].aword);
              arxy(9,ucw+5);
              prfmsg(GOTW);
              ucw++;
              drawscm();
              newpline();
              break;
            }
            //   no match on word. give him another letter
            zvda->wmissed++;
            if (zvda->wmissed==missletter) {
              cmdpos();
              prfmsg(MTOOM);
              drawscm();
              usrptr->substt=1;
              if (zvda->score>(ULONG)atol(topten[9].userscore)) updhi();
              savegame();
              usrptr->substt=0;
              break;
            }
            arxy(10,ucw+5);
            zvda->ccpos++;
            stzcpy(mstr1,zvda->pwords[ucw].aword,zvda->ccpos+1);
            prf(mstr1);
            outprf(usrnum);
            cwval=cwval-10;
            if (cwval<0) cwval=0;
            if ( zvda->ccpos==(SHORT)strlen(zvda->pwords[ucw].aword) ) {
              arxy(9,ucw+5);
              prfmsg(NOTGOT);
              ucw++;
              newpline();
            }
            drawscm();
            cmdpos();
            break;
    case 43: usrptr->substt=1;
             prfmsg(CHOPTS);
             break;
    case 44: if (margc==0) {
               usrptr->substt=1;
               prfmsg(CHOPTS);           
               break;
             } else {
               strcpy(tempstr,margv[0]);
               if ((kwtext=fopen(tempstr,"r"))==NULL) {
                 prf("Open for KW Text Failed.  Try again.");
                 //fclose(kwtext);
                 usrptr->substt=44;
                 prfmsg(ASKIMPF);
                 break;
               }
               dfaSetBlk(kwfile);
               if (dfaStepLO(pzlptr)!=1) prf("Error reading ctlrec");
               else {
                 stpos=atoi(pzlptr->puzlno);
                 stpos++;
                 prf("starting pos=%d",stpos);
                 outprf(usrnum);
                 while (fgets(tempstr,80,kwtext)!=NULL) {
                   prf("Loading:%s",tempstr);
                   outprf(usrnum);
                   stpos++;
                   itoa(stpos,pzlptr->puzlno,10);
                   stzcpy(pzlptr->puzlstr,tempstr,78);
                   iostat=dfaInsertDup(pzlptr);
                   if (iostat!=1) {
                      prf("Database is damaged, Rebuild.");
                      outprf(usrnum);
                      dfaRstBlk();
                      fclose(kwtext);
                      usrptr->substt=1;
                      prfmsg(CHOPTS);
                      break;
                   }
                 }
                 dfaStepLO(pzlptr);
                 stpos--;
                 itoa(stpos,pzlptr->puzlno,10);
                 maxrec=stpos;
                 dfaUpdate(pzlptr);
               }
               dfaRstBlk();
               fclose(kwtext);
             } 
             usrptr->substt=1;
             prf("Import Ends");
             prfmsg(CHOPTS);
             break;
    default: usrptr->substt=1;
  }
  outprf(usrnum);

  rstmbk();
  return(rcode);        /* exit to main */
}

VOID abend(VOID)
{
  setmbk(msgblk);
  if (usrptr->state==modnum) {  /* if was in me */
    if (usrptr->substt>1) savegame();
  }
  rstmbk();
}

VOID delusr(CHAR *usernm)
{
  SCORES tobj;

  dfaSetBlk(plyf);
  if (dfaAcqEQ(&tobj,usernm,0)) dfaDelete();
  dfaRstBlk();
}

VOID chterm(VOID)
{
  //savehi();
  clsmsg(msgblk);
  dfaClose(hiscf);
  dfaClose(plyf);
  dfaClose(kwfile);
}
