// WL_ACT2.C

#include <stdio.h>
#include <math.h>
#include "wl_def.h"

extern "C"{
//extern fixed slAtan(fixed y, fixed x);
extern fixed MTH_Atan(fixed y, fixed x);
}
#undef atan2
//#define atan2(a,b) slAtan(a,b)
#define atan2(a,b) MTH_Atan(a,b)


void A_StartDeathCam(objtype *ob);

void T_Path(objtype *ob);
void T_Shoot(objtype *ob);
void T_Bite(objtype *ob);
void T_DogChase(objtype *ob);
void T_Chase(objtype *ob);
void T_Projectile(objtype *ob);
void T_Stand(objtype *ob);

void A_Smoke(objtype *ob);

void T_Schabb(objtype *ob);
void T_SchabbThrow(objtype *ob);
void T_Fake(objtype *ob);
void T_FakeFire(objtype *ob);
void T_Ghosts(objtype *ob);

void A_Slurpie(objtype *ob);
void A_HitlerMorph(objtype *ob);
void A_MechaSound(objtype *ob);

void T_UShoot(objtype *ob);

void T_Launch(objtype *ob);
void T_Will(objtype *ob);

void A_Relaunch(objtype *ob);
void A_Victory(objtype *ob);
void A_StartAttack(objtype *ob);
void A_Breathing(objtype *ob);

void T_SpectreWait(objtype *ob);
void A_Dormant(objtype *ob);

void T_Gift(objtype *ob);
void T_GiftThrow (objtype *ob);

void T_Fat(objtype *ob);
void T_FatThrow(objtype *ob);

void T_BJRun(objtype *ob);
void T_BJJump(objtype *ob);
void T_BJDone(objtype *ob);
void T_BJYell(objtype *ob);

void T_DeathCam(objtype *ob);

void T_Player(objtype *ob);
void T_Attack(objtype *ob);

statetype gamestates[MAXSTATES] = {
/* s_none */	{false,0,0,NULL,NULL,s_none},
/* s_boom1 */	{false,SPR_BOOM_1,6,NULL,NULL,s_boom2},
/* s_boom2 */	{false,SPR_BOOM_2,6,NULL,NULL,s_boom3},
/* s_boom3 */	{false,SPR_BOOM_3,6,NULL,NULL, s_none},

/* s_rocket */	{true,SPR_ROCKET_1,3,(void (*)())T_Projectile,(void (*)())A_Smoke,s_rocket},

/* s_smoke1 */	{false,SPR_SMOKE_1,3,NULL,NULL,s_smoke2},
/* s_smoke2 */	{false,SPR_SMOKE_2,3,NULL,NULL,s_smoke3},
/* s_smoke3 */	{false,SPR_SMOKE_3,3,NULL,NULL,s_smoke4},
/* s_smoke4 */	{false,SPR_SMOKE_4,3,NULL,NULL, s_none},
/* guards */
/* s_grdstand */	{true,SPR_GRD_S_1,0,(void (*)())T_Stand,NULL,s_grdstand},

/* s_grdpath1  */	{true,SPR_GRD_W1_1,20,(void (*)())T_Path,NULL,s_grdpath1s},
/* s_grdpath1s  */	{true,SPR_GRD_W1_1,5,NULL,NULL,s_grdpath2},
/* s_grdpath2  */	{true,SPR_GRD_W2_1,15,(void (*)())T_Path,NULL,s_grdpath3},
/* s_grdpath3  */	{true,SPR_GRD_W3_1,20,(void (*)())T_Path,NULL,s_grdpath3s},
/* s_grdpath3s  */	{true,SPR_GRD_W3_1,5,NULL,NULL,s_grdpath4},
/* s_grdpath4  */	{true,SPR_GRD_W4_1,15,(void (*)())T_Path,NULL,s_grdpath1},

/* s_grdpain  */	{2,SPR_GRD_PAIN_1,10,NULL,NULL,s_grdchase1},
/* s_grdpain1  */	{2,SPR_GRD_PAIN_2,10,NULL,NULL,s_grdchase1},

/* s_grdshoot1  */	{false,SPR_GRD_SHOOT1,20,NULL,NULL,s_grdshoot2},
/* s_grdshoot2  */	{false,SPR_GRD_SHOOT2,20,NULL,(void (*)())T_Shoot,s_grdshoot3},
/* s_grdshoot3  */	{false,SPR_GRD_SHOOT3,20,NULL,NULL,s_grdchase1},

/* s_grdchase1  */	{true,SPR_GRD_W1_1,10,(void (*)())T_Chase,NULL,s_grdchase1s},
/* s_grdchase1s  */	{true,SPR_GRD_W1_1,3,NULL,NULL,s_grdchase2},
/* s_grdchase2  */	{true,SPR_GRD_W2_1,8,(void (*)())T_Chase,NULL,s_grdchase3},
/* s_grdchase3  */	{true,SPR_GRD_W3_1,10,(void (*)())T_Chase,NULL,s_grdchase3s},
/* s_grdchase3s  */	{true,SPR_GRD_W3_1,3,NULL,NULL,s_grdchase4},
/* s_grdchase4  */	{true,SPR_GRD_W4_1,8,(void (*)())T_Chase,NULL,s_grdchase1},

/* s_grddie1	 */	{false,SPR_GRD_DIE_1,15,NULL,(void (*)())A_DeathScream,s_grddie2},
/* s_grddie2	 */	{false,SPR_GRD_DIE_2,15,NULL,NULL,s_grddie3},
/* s_grddie3	 */	{false,SPR_GRD_DIE_3,15,NULL,NULL,s_grddie4},
/* s_grddie4	 */	{false,SPR_GRD_DEAD,0,NULL,NULL,s_grddie4},



/* s_dogpath1  */	{true,SPR_DOG_W1_1,20,(void (*)())T_Path,NULL,s_dogpath1s},
/* s_dogpath1s  */	{true,SPR_DOG_W1_1,5,NULL,NULL,s_dogpath2},
/* s_dogpath2  */	{true,SPR_DOG_W2_1,15,(void (*)())T_Path,NULL,s_dogpath3},
/* s_dogpath3  */	{true,SPR_DOG_W3_1,20,(void (*)())T_Path,NULL,s_dogpath3s},
/* s_dogpath3s  */	{true,SPR_DOG_W3_1,5,NULL,NULL,s_dogpath4},
/* s_dogpath4  */	{true,SPR_DOG_W4_1,15,(void (*)())T_Path,NULL,s_dogpath1},

/* s_dogjump1  */	{false,SPR_DOG_JUMP1,10,NULL,NULL,s_dogjump2},
/* s_dogjump2  */	{false,SPR_DOG_JUMP2,10,NULL,(void (*)())T_Bite,s_dogjump3},
/* s_dogjump3  */	{false,SPR_DOG_JUMP3,10,NULL,NULL,s_dogjump4},
/* s_dogjump4  */	{false,SPR_DOG_JUMP1,10,NULL,NULL,s_dogjump5},
/* s_dogjump5  */	{false,SPR_DOG_W1_1,10,NULL,NULL,s_dogchase1},

/* s_dogchase1  */	{true,SPR_DOG_W1_1,10,(void (*)())T_DogChase,NULL,s_dogchase1s},
/* s_dogchase1s  */	{true,SPR_DOG_W1_1,3,NULL,NULL,s_dogchase2},
/* s_dogchase2  */	{true,SPR_DOG_W2_1,8,(void (*)())T_DogChase,NULL,s_dogchase3},
/* s_dogchase3  */	{true,SPR_DOG_W3_1,10,(void (*)())T_DogChase,NULL,s_dogchase3s},
/* s_dogchase3s  */	{true,SPR_DOG_W3_1,3,NULL,NULL,s_dogchase4},
/* s_dogchase4  */	{true,SPR_DOG_W4_1,8,(void (*)())T_DogChase,NULL,s_dogchase1},

/* s_dogdie1	 */	{false,SPR_DOG_DIE_1,15,NULL,(void (*)())A_DeathScream,s_dogdie2},
/* s_dogdie2	 */	{false,SPR_DOG_DIE_2,15,NULL,NULL,s_dogdie3},
/* s_dogdie3	 */	{false,SPR_DOG_DIE_3,15,NULL,NULL,s_dogdead},
/* s_dogdead	 */	{false,SPR_DOG_DEAD,15,NULL,NULL,s_dogdead},

/* s_ofcstand */	{true,SPR_OFC_S_1,0,(void (*)())T_Stand,NULL,s_ofcstand},

/* s_ofcpath1  */	{true,SPR_OFC_W1_1,20,(void (*)())T_Path,NULL,s_ofcpath1s},
/* s_ofcpath1s  */	{true,SPR_OFC_W1_1,5,NULL,NULL,s_ofcpath2},
/* s_ofcpath2  */	{true,SPR_OFC_W2_1,15,(void (*)())T_Path,NULL,s_ofcpath3},
/* s_ofcpath3  */	{true,SPR_OFC_W3_1,20,(void (*)())T_Path,NULL,s_ofcpath3s},
/* s_ofcpath3s  */	{true,SPR_OFC_W3_1,5,NULL,NULL,s_ofcpath4},
/* s_ofcpath4  */	{true,SPR_OFC_W4_1,15,(void (*)())T_Path,NULL,s_ofcpath1},

/* s_ofcpain  */	{2,SPR_OFC_PAIN_1,10,NULL,NULL,s_ofcchase1},
/* s_ofcpain1  */	{2,SPR_OFC_PAIN_2,10,NULL,NULL,s_ofcchase1},

/* s_ofcshoot1  */	{false,SPR_OFC_SHOOT1,6,NULL,NULL,s_ofcshoot2},
/* s_ofcshoot2  */	{false,SPR_OFC_SHOOT2,20,NULL,(void (*)())T_Shoot,s_ofcshoot3},
/* s_ofcshoot3  */	{false,SPR_OFC_SHOOT3,10,NULL,NULL,s_ofcchase1},

/* s_ofcchase1  */	{true,SPR_OFC_W1_1,10,(void (*)())T_Chase,NULL,s_ofcchase1s},
/* s_ofcchase1s  */	{true,SPR_OFC_W1_1,3,NULL,NULL,s_ofcchase2},
/* s_ofcchase2  */	{true,SPR_OFC_W2_1,8,(void (*)())T_Chase,NULL,s_ofcchase3},
/* s_ofcchase3  */	{true,SPR_OFC_W3_1,10,(void (*)())T_Chase,NULL,s_ofcchase3s},
/* s_ofcchase3s  */	{true,SPR_OFC_W3_1,3,NULL,NULL,s_ofcchase4},
/* s_ofcchase4  */	{true,SPR_OFC_W4_1,8,(void (*)())T_Chase,NULL,s_ofcchase1},

/* s_ofcdie1	 */	{false,SPR_OFC_DIE_1,11,NULL,(void (*)())A_DeathScream,s_ofcdie2},
/* s_ofcdie2	 */	{false,SPR_OFC_DIE_2,11,NULL,NULL,s_ofcdie3},
/* s_ofcdie3	 */	{false,SPR_OFC_DIE_3,11,NULL,NULL,s_ofcdie4},
/* s_ofcdie4	 */	{false,SPR_OFC_DIE_4,11,NULL,NULL,s_ofcdie5},
/* s_ofcdie5	 */	{false,SPR_OFC_DEAD,0,NULL,NULL,s_ofcdie5},


/* s_mutstand */	{true,SPR_MUT_S_1,0,(void (*)())T_Stand,NULL,s_mutstand},

/* s_mutpath1  */	{true,SPR_MUT_W1_1,20,(void (*)())T_Path,NULL,s_mutpath1s},
/* s_mutpath1s  */	{true,SPR_MUT_W1_1,5,NULL,NULL,s_mutpath2},
/* s_mutpath2  */	{true,SPR_MUT_W2_1,15,(void (*)())T_Path,NULL,s_mutpath3},
/* s_mutpath3  */	{true,SPR_MUT_W3_1,20,(void (*)())T_Path,NULL,s_mutpath3s},
/* s_mutpath3s  */	{true,SPR_MUT_W3_1,5,NULL,NULL,s_mutpath4},
/* s_mutpath4  */	{true,SPR_MUT_W4_1,15,(void (*)())T_Path,NULL,s_mutpath1},

/* s_mutpain  */	{2,SPR_MUT_PAIN_1,10,NULL,NULL,s_mutchase1},
/* s_mutpain1  */	{2,SPR_MUT_PAIN_2,10,NULL,NULL,s_mutchase1},

/* s_mutshoot1  */	{false,SPR_MUT_SHOOT1,6,NULL,(void (*)())T_Shoot,s_mutshoot2},
/* s_mutshoot2  */	{false,SPR_MUT_SHOOT2,20,NULL,NULL,s_mutshoot3},
/* s_mutshoot3  */	{false,SPR_MUT_SHOOT3,10,NULL,(void (*)())T_Shoot,s_mutshoot4},
/* s_mutshoot4  */	{false,SPR_MUT_SHOOT4,20,NULL,NULL,s_mutchase1},

/* s_mutchase1  */	{true,SPR_MUT_W1_1,10,(void (*)())T_Chase,NULL,s_mutchase1s},
/* s_mutchase1s  */	{true,SPR_MUT_W1_1,3,NULL,NULL,s_mutchase2},
/* s_mutchase2  */	{true,SPR_MUT_W2_1,8,(void (*)())T_Chase,NULL,s_mutchase3},
/* s_mutchase3  */	{true,SPR_MUT_W3_1,10,(void (*)())T_Chase,NULL,s_mutchase3s},
/* s_mutchase3s  */	{true,SPR_MUT_W3_1,3,NULL,NULL,s_mutchase4},
/* s_mutchase4  */	{true,SPR_MUT_W4_1,8,(void (*)())T_Chase,NULL,s_mutchase1},

/* s_mutdie1	 */	{false,SPR_MUT_DIE_1,7,NULL,(void (*)())A_DeathScream,s_mutdie2},
/* s_mutdie2	 */	{false,SPR_MUT_DIE_2,7,NULL,NULL,s_mutdie3},
/* s_mutdie3	 */	{false,SPR_MUT_DIE_3,7,NULL,NULL,s_mutdie4},
/* s_mutdie4	 */	{false,SPR_MUT_DIE_4,7,NULL,NULL,s_mutdie5},
/* s_mutdie5	 */	{false,SPR_MUT_DEAD,0,NULL,NULL,s_mutdie5},


/* s_ssstand */	{true,SPR_SS_S_1,0,(void (*)())T_Stand,NULL,s_ssstand},

/* s_sspath1  */	{true,SPR_SS_W1_1,20,(void (*)())T_Path,NULL,s_sspath1s},
/* s_sspath1s  */	{true,SPR_SS_W1_1,5,NULL,NULL,s_sspath2},
/* s_sspath2  */	{true,SPR_SS_W2_1,15,(void (*)())T_Path,NULL,s_sspath3},
/* s_sspath3  */	{true,SPR_SS_W3_1,20,(void (*)())T_Path,NULL,s_sspath3s},
/* s_sspath3s  */	{true,SPR_SS_W3_1,5,NULL,NULL,s_sspath4},
/* s_sspath4  */	{true,SPR_SS_W4_1,15,(void (*)())T_Path,NULL,s_sspath1},

/* s_sspain 	 */	{2,SPR_SS_PAIN_1,10,NULL,NULL,s_sschase1},
/* s_sspain1  */	{2,SPR_SS_PAIN_2,10,NULL,NULL,s_sschase1},

/* s_ssshoot1  */	{false,SPR_SS_SHOOT1,20,NULL,NULL,s_ssshoot2},
/* s_ssshoot2  */	{false,SPR_SS_SHOOT2,20,NULL,(void (*)())T_Shoot,s_ssshoot3},
/* s_ssshoot3  */	{false,SPR_SS_SHOOT3,10,NULL,NULL,s_ssshoot4},
/* s_ssshoot4  */	{false,SPR_SS_SHOOT2,10,NULL,(void (*)())T_Shoot,s_ssshoot5},
/* s_ssshoot5  */	{false,SPR_SS_SHOOT3,10,NULL,NULL,s_ssshoot6},
/* s_ssshoot6  */	{false,SPR_SS_SHOOT2,10,NULL,(void (*)())T_Shoot,s_ssshoot7},
/* s_ssshoot7   */	{false,SPR_SS_SHOOT3,10,NULL,NULL,s_ssshoot8},
/* s_ssshoot8   */	{false,SPR_SS_SHOOT2,10,NULL,(void (*)())T_Shoot,s_ssshoot9},
/* s_ssshoot9   */	{false,SPR_SS_SHOOT3,10,NULL,NULL,s_sschase1},

/* s_sschase1  */	{true,SPR_SS_W1_1,10,(void (*)())T_Chase,NULL,s_sschase1s},
/* s_sschase1s  */	{true,SPR_SS_W1_1,3,NULL,NULL,s_sschase2},
/* s_sschase2  */	{true,SPR_SS_W2_1,8,(void (*)())T_Chase,NULL,s_sschase3},
/* s_sschase3  */	{true,SPR_SS_W3_1,10,(void (*)())T_Chase,NULL,s_sschase3s},
/* s_sschase3s  */	{true,SPR_SS_W3_1,3,NULL,NULL,s_sschase4},
/* s_sschase4  */	{true,SPR_SS_W4_1,8,(void (*)())T_Chase,NULL,s_sschase1},

/* s_ssdie1	 */	{false,SPR_SS_DIE_1,15,NULL,(void (*)())A_DeathScream,s_ssdie2},
/* s_ssdie2	 */	{false,SPR_SS_DIE_2,15,NULL,NULL,s_ssdie3},
/* s_ssdie3	 */	{false,SPR_SS_DIE_3,15,NULL,NULL,s_ssdie4},
/* s_ssdie4	 */	{false,SPR_SS_DEAD,0,NULL,NULL,s_ssdie4},


#ifdef SPEAR

/* s_hrocket	  */	{true,SPR_HROCKET_1,3,(void (*)())T_Projectile,(void (*)())A_Smoke,s_hrocket},
/* s_hsmoke1	  */	{false,SPR_HSMOKE_1,3,NULL,NULL,s_hsmoke2},
/* s_hsmoke2	  */	{false,SPR_HSMOKE_2,3,NULL,NULL,s_hsmoke3},
/* s_hsmoke3	  */	{false,SPR_HSMOKE_3,3,NULL,NULL,s_hsmoke4},
/* s_hsmoke4	  */	{false,SPR_HSMOKE_4,3,NULL,NULL, s_none},

/* s_hboom1	  */	{false,SPR_HBOOM_1,6,NULL,NULL,s_hboom2},
/* s_hboom2	  */	{false,SPR_HBOOM_2,6,NULL,NULL,s_hboom3},
/* s_hboom3	  */	{false,SPR_HBOOM_3,6,NULL,NULL,s_none},

/* s_transstand */	{false,SPR_TRANS_W1,0,(void (*)())T_Stand,NULL,s_transstand},

/* s_transchase1  */	{false,SPR_TRANS_W1,10,(void (*)())T_Chase,NULL,s_transchase1s},
/* s_transchase1s */	{false,SPR_TRANS_W1,3,NULL,NULL,s_transchase2},
/* s_transchase2  */	{false,SPR_TRANS_W2,8,(void (*)())T_Chase,NULL,s_transchase3},
/* s_transchase3  */	{false,SPR_TRANS_W3,10,(void (*)())T_Chase,NULL,s_transchase3s},
/* s_transchase3s */	{false,SPR_TRANS_W3,3,NULL,NULL,s_transchase4},
/* s_transchase4  */	{false,SPR_TRANS_W4,8,(void (*)())T_Chase,NULL,s_transchase1},

/* s_transdie0 */	{false,SPR_TRANS_W1,1,NULL,(void (*)())A_DeathScream,s_transdie01},
/* s_transdie01 */	{false,SPR_TRANS_W1,1,NULL,NULL,s_transdie1},
/* s_transdie1 */	{false,SPR_TRANS_DIE1,15,NULL,NULL,s_transdie2},
/* s_transdie2 */	{false,SPR_TRANS_DIE2,15,NULL,NULL,s_transdie3},
/* s_transdie3 */	{false,SPR_TRANS_DIE3,15,NULL,NULL,s_transdie4},
/* s_transdie4 */	{false,SPR_TRANS_DEAD,0,NULL,NULL,s_transdie4},

/* s_transshoot1  */	{false,SPR_TRANS_SHOOT1,30,NULL,NULL,s_transshoot2},
/* s_transshoot2  */	{false,SPR_TRANS_SHOOT2,10,NULL,(void (*)())T_Shoot,s_transshoot3},
/* s_transshoot3  */	{false,SPR_TRANS_SHOOT3,10,NULL,(void (*)())T_Shoot,s_transshoot4},
/* s_transshoot4  */	{false,SPR_TRANS_SHOOT2,10,NULL,(void (*)())T_Shoot,s_transshoot5},
/* s_transshoot5  */	{false,SPR_TRANS_SHOOT3,10,NULL,(void (*)())T_Shoot,s_transshoot6},
/* s_transshoot6  */	{false,SPR_TRANS_SHOOT2,10,NULL,(void (*)())T_Shoot,s_transshoot7},
/* s_transshoot7  */	{false,SPR_TRANS_SHOOT3,10,NULL,(void (*)())T_Shoot,s_transshoot8},
/* s_transshoot8  */	{false,SPR_TRANS_SHOOT1,10,NULL,NULL,s_transchase1},


/* s_uberstand */	{false,SPR_UBER_W1,0,(void (*)())T_Stand,NULL,s_uberstand},

/* s_uberchase1  */	{false,SPR_UBER_W1,10,(void (*)())T_Chase,NULL,s_uberchase1s},
/* s_uberchase1s */	{false,SPR_UBER_W1,3,NULL,NULL,s_uberchase2},
/* s_uberchase2  */	{false,SPR_UBER_W2,8,(void (*)())T_Chase,NULL,s_uberchase3},
/* s_uberchase3  */	{false,SPR_UBER_W3,10,(void (*)())T_Chase,NULL,s_uberchase3s},
/* s_uberchase3s */	{false,SPR_UBER_W3,3,NULL,NULL,s_uberchase4},
/* s_uberchase4  */	{false,SPR_UBER_W4,8,(void (*)())T_Chase,NULL,s_uberchase1},

/* s_uberdie0 */	{false,SPR_UBER_W1,1,NULL,(void (*)())A_DeathScream,s_uberdie01},
/* s_uberdie01 */	{false,SPR_UBER_W1,1,NULL,NULL,s_uberdie1},
/* s_uberdie1 */	{false,SPR_UBER_DIE1,15,NULL,NULL,s_uberdie2},
/* s_uberdie2 */	{false,SPR_UBER_DIE2,15,NULL,NULL,s_uberdie3},
/* s_uberdie3 */	{false,SPR_UBER_DIE3,15,NULL,NULL,s_uberdie4},
/* s_uberdie4 */	{false,SPR_UBER_DIE4,15,NULL,NULL,s_uberdie5},
/* s_uberdie5 */	{false,SPR_UBER_DEAD,0,NULL,NULL,s_uberdie5},

/* s_ubershoot1  */	{false,SPR_UBER_SHOOT1,30,NULL,NULL,s_ubershoot2},
/* s_ubershoot2  */	{false,SPR_UBER_SHOOT2,12,NULL,(void (*)())T_UShoot,s_ubershoot3},
/* s_ubershoot3  */	{false,SPR_UBER_SHOOT3,12,NULL,(void (*)())T_UShoot,s_ubershoot4},
/* s_ubershoot4  */	{false,SPR_UBER_SHOOT4,12,NULL,(void (*)())T_UShoot,s_ubershoot5},
/* s_ubershoot5  */	{false,SPR_UBER_SHOOT3,12,NULL,(void (*)())T_UShoot,s_ubershoot6},
/* s_ubershoot6  */	{false,SPR_UBER_SHOOT2,12,NULL,(void (*)())T_UShoot,s_ubershoot7},
/* s_ubershoot7  */	{false,SPR_UBER_SHOOT1,12,NULL,NULL,s_uberchase1},


/* s_willstand */	{false,SPR_WILL_W1,0,(void (*)())T_Stand,NULL,s_willstand},

/* s_willchase1  */	{false,SPR_WILL_W1,10,(void (*)())T_Will,NULL,s_willchase1s},
/* s_willchase1s */	{false,SPR_WILL_W1,3,NULL,NULL,s_willchase2},
/* s_willchase2  */	{false,SPR_WILL_W2,8,(void (*)())T_Will,NULL,s_willchase3},
/* s_willchase3  */	{false,SPR_WILL_W3,10,(void (*)())T_Will,NULL,s_willchase3s},
/* s_willchase3s */	{false,SPR_WILL_W3,3,NULL,NULL,s_willchase4},
/* s_willchase4  */	{false,SPR_WILL_W4,8,(void (*)())T_Will,NULL,s_willchase1},

/* s_willdie1 */	{false,SPR_WILL_W1,1,NULL,(void (*)())A_DeathScream,s_willdie2},
/* s_willdie2 */	{false,SPR_WILL_W1,10,NULL,NULL,s_willdie3},
/* s_willdie3 */	{false,SPR_WILL_DIE1,10,NULL,NULL,s_willdie4},
/* s_willdie4 */	{false,SPR_WILL_DIE2,10,NULL,NULL,s_willdie5},
/* s_willdie5 */	{false,SPR_WILL_DIE3,10,NULL,NULL,s_willdie6},
/* s_willdie6 */	{false,SPR_WILL_DEAD,20,NULL,NULL,s_willdie6},

/* s_willshoot1  */	{false,SPR_WILL_SHOOT1,30,NULL,NULL,s_willshoot2},
/* s_willshoot2  */	{false,SPR_WILL_SHOOT2,10,NULL,(void (*)())T_Launch,s_willshoot3},
/* s_willshoot3  */	{false,SPR_WILL_SHOOT3,10,NULL,(void (*)())T_Shoot,s_willshoot4},
/* s_willshoot4  */	{false,SPR_WILL_SHOOT4,10,NULL,(void (*)())T_Shoot,s_willshoot5},
/* s_willshoot5  */	{false,SPR_WILL_SHOOT3,10,NULL,(void (*)())T_Shoot,s_willshoot6},
/* s_willshoot6  */	{false,SPR_WILL_SHOOT4,10,NULL,(void (*)())T_Shoot,s_willchase1},


/* s_deathstand */	{false,SPR_DEATH_W1,0,(void (*)())T_Stand,NULL,s_deathstand},

/* s_deathchase1  */	{false,SPR_DEATH_W1,10,(void (*)())T_Will,NULL,s_deathchase1s},
/* s_deathchase1s */	{false,SPR_DEATH_W1,3,NULL,NULL,s_deathchase2},
/* s_deathchase2  */	{false,SPR_DEATH_W2,8,(void (*)())T_Will,NULL,s_deathchase3},
/* s_deathchase3  */	{false,SPR_DEATH_W3,10,(void (*)())T_Will,NULL,s_deathchase3s},
/* s_deathchase3s */	{false,SPR_DEATH_W3,3,NULL,NULL,s_deathchase4},
/* s_deathchase4  */	{false,SPR_DEATH_W4,8,(void (*)())T_Will,NULL,s_deathchase1},

/* s_deathdie1 */	{false,SPR_DEATH_W1,1,NULL,(void (*)())A_DeathScream,s_deathdie2},
/* s_deathdie2 */	{false,SPR_DEATH_W1,10,NULL,NULL,s_deathdie3},
/* s_deathdie3 */	{false,SPR_DEATH_DIE1,10,NULL,NULL,s_deathdie4},
/* s_deathdie4 */	{false,SPR_DEATH_DIE2,10,NULL,NULL,s_deathdie5},
/* s_deathdie5 */	{false,SPR_DEATH_DIE3,10,NULL,NULL,s_deathdie6},
/* s_deathdie6 */	{false,SPR_DEATH_DIE4,10,NULL,NULL,s_deathdie7},
/* s_deathdie7 */	{false,SPR_DEATH_DIE5,10,NULL,NULL,s_deathdie8},
/* s_deathdie8 */	{false,SPR_DEATH_DIE6,10,NULL,NULL,s_deathdie9},
/* s_deathdie9 */	{false,SPR_DEATH_DEAD,0,NULL,NULL,s_deathdie9},

/* s_deathshoot1  */	{false,SPR_DEATH_SHOOT1,30,NULL,NULL,s_deathshoot2},
/* s_deathshoot2  */	{false,SPR_DEATH_SHOOT2,10,NULL,(void (*)())T_Launch,s_deathshoot3},
/* s_deathshoot3  */	{false,SPR_DEATH_SHOOT4,10,NULL,(void (*)())T_Shoot,s_deathshoot4},
/* s_deathshoot4  */	{false,SPR_DEATH_SHOOT3,10,NULL,(void (*)())T_Launch,s_deathshoot5},
/* s_deathshoot5  */	{false,SPR_DEATH_SHOOT4,10,NULL,(void (*)())T_Shoot,s_deathchase1},


/* s_angelstand */	{false,SPR_ANGEL_W1,0,(void (*)())T_Stand,NULL,s_angelstand},

/* s_angelchase1  */	{false,SPR_ANGEL_W1,10,(void (*)())T_Will,NULL,s_angelchase1s},
/* s_angelchase1s */	{false,SPR_ANGEL_W1,3,NULL,NULL,s_angelchase2},
/* s_angelchase2  */	{false,SPR_ANGEL_W2,8,(void (*)())T_Will,NULL,s_angelchase3},
/* s_angelchase3  */	{false,SPR_ANGEL_W3,10,(void (*)())T_Will,NULL,s_angelchase3s},
/* s_angelchase3s */	{false,SPR_ANGEL_W3,3,NULL,NULL,s_angelchase4},
/* s_angelchase4  */	{false,SPR_ANGEL_W4,8,(void (*)())T_Will,NULL,s_angelchase1},

/* s_angeldie1 */	{false,SPR_ANGEL_W1,1,NULL,(void (*)())A_DeathScream,s_angeldie11},
/* s_angeldie11 */	{false,SPR_ANGEL_W1,1,NULL,NULL,s_angeldie2},
/* s_angeldie2 */	{false,SPR_ANGEL_DIE1,10,NULL,(void (*)())A_Slurpie,s_angeldie3},
/* s_angeldie3 */	{false,SPR_ANGEL_DIE2,10,NULL,NULL,s_angeldie4},
/* s_angeldie4 */	{false,SPR_ANGEL_DIE3,10,NULL,NULL,s_angeldie5},
/* s_angeldie5 */	{false,SPR_ANGEL_DIE4,10,NULL,NULL,s_angeldie6},
/* s_angeldie6 */	{false,SPR_ANGEL_DIE5,10,NULL,NULL,s_angeldie7},
/* s_angeldie7 */	{false,SPR_ANGEL_DIE6,10,NULL,NULL,s_angeldie8},
/* s_angeldie8 */	{false,SPR_ANGEL_DIE7,10,NULL,NULL,s_angeldie9},
/* s_angeldie9 */	{false,SPR_ANGEL_DEAD,130,NULL,(void (*)())A_Victory,s_angeldie9},

/* s_angelshoot1  */	{false,SPR_ANGEL_SHOOT1,10,NULL,(void (*)())A_StartAttack,s_angelshoot2},
/* s_angelshoot2  */	{false,SPR_ANGEL_SHOOT2,20,NULL,(void (*)())T_Launch,s_angelshoot3},
/* s_angelshoot3  */	{false,SPR_ANGEL_SHOOT1,10,NULL,(void (*)())A_Relaunch,s_angelshoot2},

/* s_angeltired  */	{false,SPR_ANGEL_TIRED1,40,NULL,(void (*)())A_Breathing,s_angeltired2},
/* s_angeltired2 */	{false,SPR_ANGEL_TIRED2,40,NULL,NULL,s_angeltired3},
/* s_angeltired3 */	{false,SPR_ANGEL_TIRED1,40,NULL,(void (*)())A_Breathing,s_angeltired4},
/* s_angeltired4 */	{false,SPR_ANGEL_TIRED2,40,NULL,NULL,s_angeltired5},
/* s_angeltired5 */	{false,SPR_ANGEL_TIRED1,40,NULL,(void (*)())A_Breathing,s_angeltired6},
/* s_angeltired6 */	{false,SPR_ANGEL_TIRED2,40,NULL,NULL,s_angeltired7},
/* s_angeltired7 */	{false,SPR_ANGEL_TIRED1,40,NULL,(void (*)())A_Breathing,s_angelchase1},

/* s_spark1  */	{false,SPR_SPARK1,6,(void (*)())T_Projectile,NULL,s_spark2},
/* s_spark2  */	{false,SPR_SPARK2,6,(void (*)())T_Projectile,NULL,s_spark3},
/* s_spark3  */	{false,SPR_SPARK3,6,(void (*)())T_Projectile,NULL,s_spark4},
/* s_spark4  */	{false,SPR_SPARK4,6,(void (*)())T_Projectile,NULL,s_spark1},


//
// spectre
//


/* s_spectrewait1 */	{false,SPR_SPECTRE_W1,10,(void (*)())T_Stand,NULL,s_spectrewait2},
/* s_spectrewait2 */	{false,SPR_SPECTRE_W2,10,(void (*)())T_Stand,NULL,s_spectrewait3},
/* s_spectrewait3 */	{false,SPR_SPECTRE_W3,10,(void (*)())T_Stand,NULL,s_spectrewait4},
/* s_spectrewait4 */	{false,SPR_SPECTRE_W4,10,(void (*)())T_Stand,NULL,s_spectrewait1},

/* s_spectrechase1 */	{false,SPR_SPECTRE_W1,10,(void (*)())T_Ghosts,NULL,s_spectrechase2},
/* s_spectrechase2 */	{false,SPR_SPECTRE_W2,10,(void (*)())T_Ghosts,NULL,s_spectrechase3},
/* s_spectrechase3 */	{false,SPR_SPECTRE_W3,10,(void (*)())T_Ghosts,NULL,s_spectrechase4},
/* s_spectrechase4 */	{false,SPR_SPECTRE_W4,10,(void (*)())T_Ghosts,NULL,s_spectrechase1},

/* s_spectredie1 */	{false,SPR_SPECTRE_F1,10,NULL,NULL,s_spectredie2},
/* s_spectredie2 */	{false,SPR_SPECTRE_F2,10,NULL,NULL,s_spectredie3},
/* s_spectredie3 */	{false,SPR_SPECTRE_F3,10,NULL,NULL,s_spectredie4},
/* s_spectredie4 */	{false,SPR_SPECTRE_F4,300,NULL,NULL,s_spectrewake},
/* s_spectrewake */	{false,SPR_SPECTRE_F4,10,NULL,(void (*)())A_Dormant,s_spectrewake},

#endif

#ifndef SPEAR

/* s_blinkychase1  */	{false,SPR_BLINKY_W1,10,(void (*)())T_Ghosts,NULL,s_blinkychase2},
/* s_blinkychase2  */	{false,SPR_BLINKY_W2,10,(void (*)())T_Ghosts,NULL,s_blinkychase1},

/* s_inkychase1 	 */	{false,SPR_INKY_W1,10,(void (*)())T_Ghosts,NULL,s_inkychase2},
/* s_inkychase2 	 */	{false,SPR_INKY_W2,10,(void (*)())T_Ghosts,NULL,s_inkychase1},

/* s_pinkychase1  */	{false,SPR_PINKY_W1,10,(void (*)())T_Ghosts,NULL,s_pinkychase2},
/* s_pinkychase2  */	{false,SPR_PINKY_W2,10,(void (*)())T_Ghosts,NULL,s_pinkychase1},

/* s_clydechase1  */	{false,SPR_CLYDE_W1,10,(void (*)())T_Ghosts,NULL,s_clydechase2},
/* s_clydechase2  */	{false,SPR_CLYDE_W2,10,(void (*)())T_Ghosts,NULL,s_clydechase1},

/* s_bossstand */	{false,SPR_BOSS_W1,0,(void (*)())T_Stand,NULL,s_bossstand},

/* s_bosschase1  */	{false,SPR_BOSS_W1,10,(void (*)())T_Chase,NULL,s_bosschase1s},
/* s_bosschase1s */	{false,SPR_BOSS_W1,3,NULL,NULL,s_bosschase2},
/* s_bosschase2  */	{false,SPR_BOSS_W2,8,(void (*)())T_Chase,NULL,s_bosschase3},
/* s_bosschase3  */	{false,SPR_BOSS_W3,10,(void (*)())T_Chase,NULL,s_bosschase3s},
/* s_bosschase3s */	{false,SPR_BOSS_W3,3,NULL,NULL,s_bosschase4},
/* s_bosschase4  */	{false,SPR_BOSS_W4,8,(void (*)())T_Chase,NULL,s_bosschase1},

/* s_bossdie1 */	{false,SPR_BOSS_DIE1,15,NULL,(void (*)())A_DeathScream,s_bossdie2},
/* s_bossdie2 */	{false,SPR_BOSS_DIE2,15,NULL,NULL,s_bossdie3},
/* s_bossdie3 */	{false,SPR_BOSS_DIE3,15,NULL,NULL,s_bossdie4},
/* s_bossdie4 */	{false,SPR_BOSS_DEAD,0,NULL,NULL,s_bossdie4},

/* s_bossshoot1  */	{false,SPR_BOSS_SHOOT1,30,NULL,NULL,s_bossshoot2},
/* s_bossshoot2  */	{false,SPR_BOSS_SHOOT2,10,NULL,(void (*)())T_Shoot,s_bossshoot3},
/* s_bossshoot3  */	{false,SPR_BOSS_SHOOT3,10,NULL,(void (*)())T_Shoot,s_bossshoot4},
/* s_bossshoot4  */	{false,SPR_BOSS_SHOOT2,10,NULL,(void (*)())T_Shoot,s_bossshoot5},
/* s_bossshoot5  */	{false,SPR_BOSS_SHOOT3,10,NULL,(void (*)())T_Shoot,s_bossshoot6},
/* s_bossshoot6  */	{false,SPR_BOSS_SHOOT2,10,NULL,(void (*)())T_Shoot,s_bossshoot7},
/* s_bossshoot7  */	{false,SPR_BOSS_SHOOT3,10,NULL,(void (*)())T_Shoot,s_bossshoot8},
/* s_bossshoot8  */	{false,SPR_BOSS_SHOOT1,10,NULL,NULL,s_bosschase1},



/* s_gretelstand */	{false,SPR_GRETEL_W1,0,(void (*)())T_Stand,NULL,s_gretelstand},

/* s_gretelchase1  */	{false,SPR_GRETEL_W1,10,(void (*)())T_Chase,NULL,s_gretelchase1s},
/* s_gretelchase1s */	{false,SPR_GRETEL_W1,3,NULL,NULL,s_gretelchase2},
/* s_gretelchase2  */	{false,SPR_GRETEL_W2,8,(void (*)())T_Chase,NULL,s_gretelchase3},
/* s_gretelchase3  */	{false,SPR_GRETEL_W3,10,(void (*)())T_Chase,NULL,s_gretelchase3s},
/* s_gretelchase3s */	{false,SPR_GRETEL_W3,3,NULL,NULL,s_gretelchase4},
/* s_gretelchase4  */	{false,SPR_GRETEL_W4,8,(void (*)())T_Chase,NULL,s_gretelchase1},

/* s_greteldie1 */	{false,SPR_GRETEL_DIE1,15,NULL,(void (*)())A_DeathScream,s_greteldie2},
/* s_greteldie2 */	{false,SPR_GRETEL_DIE2,15,NULL,NULL,s_greteldie3},
/* s_greteldie3 */	{false,SPR_GRETEL_DIE3,15,NULL,NULL,s_greteldie4},
/* s_greteldie4 */	{false,SPR_GRETEL_DEAD,0,NULL,NULL,s_greteldie4},

/* s_gretelshoot1  */	{false,SPR_GRETEL_SHOOT1,30,NULL,NULL,s_gretelshoot2},
/* s_gretelshoot2  */	{false,SPR_GRETEL_SHOOT2,10,NULL,(void (*)())T_Shoot,s_gretelshoot3},
/* s_gretelshoot3  */	{false,SPR_GRETEL_SHOOT3,10,NULL,(void (*)())T_Shoot,s_gretelshoot4},
/* s_gretelshoot4  */	{false,SPR_GRETEL_SHOOT2,10,NULL,(void (*)())T_Shoot,s_gretelshoot5},
/* s_gretelshoot5  */	{false,SPR_GRETEL_SHOOT3,10,NULL,(void (*)())T_Shoot,s_gretelshoot6},
/* s_gretelshoot6  */	{false,SPR_GRETEL_SHOOT2,10,NULL,(void (*)())T_Shoot,s_gretelshoot7},
/* s_gretelshoot7  */	{false,SPR_GRETEL_SHOOT3,10,NULL,(void (*)())T_Shoot,s_gretelshoot8},
/* s_gretelshoot8  */	{false,SPR_GRETEL_SHOOT1,10,NULL,NULL,s_gretelchase1},

/* s_schabbstand */	{false,SPR_SCHABB_W1,0,(void (*)())T_Stand,NULL,s_schabbstand},

/* s_schabbchase1  */	{false,SPR_SCHABB_W1,10,(void (*)())T_Schabb,NULL,s_schabbchase1s},
/* s_schabbchase1s */	{false,SPR_SCHABB_W1,3,NULL,NULL,s_schabbchase2},
/* s_schabbchase2  */	{false,SPR_SCHABB_W2,8,(void (*)())T_Schabb,NULL,s_schabbchase3},
/* s_schabbchase3  */	{false,SPR_SCHABB_W3,10,(void (*)())T_Schabb,NULL,s_schabbchase3s},
/* s_schabbchase3s */	{false,SPR_SCHABB_W3,3,NULL,NULL,s_schabbchase4},
/* s_schabbchase4  */	{false,SPR_SCHABB_W4,8,(void (*)())T_Schabb,NULL,s_schabbchase1},

/* s_schabbdeathcam */	{false,SPR_SCHABB_W1,1,NULL,NULL,s_schabbdie1},

/* s_schabbdie1 */	{false,SPR_SCHABB_W1,10,NULL,(void (*)())A_DeathScream,s_schabbdie2},
/* s_schabbdie2 */	{false,SPR_SCHABB_W1,10,NULL,NULL,s_schabbdie3},
/* s_schabbdie3 */	{false,SPR_SCHABB_DIE1,10,NULL,NULL,s_schabbdie4},
/* s_schabbdie4 */	{false,SPR_SCHABB_DIE2,10,NULL,NULL,s_schabbdie5},
/* s_schabbdie5 */	{false,SPR_SCHABB_DIE3,10,NULL,NULL,s_schabbdie6},
/* s_schabbdie6 */	{false,SPR_SCHABB_DEAD,20,NULL,(void (*)())A_StartDeathCam,s_schabbdie6},

/* s_schabbshoot1  */	{false,SPR_SCHABB_SHOOT1,30,NULL,NULL,s_schabbshoot2},
/* s_schabbshoot2  */	{false,SPR_SCHABB_SHOOT2,10,NULL,(void (*)())T_SchabbThrow,s_schabbchase1},

/* s_needle1  */	{false,SPR_HYPO1,6,(void (*)())T_Projectile,NULL,s_needle2},
/* s_needle2  */	{false,SPR_HYPO2,6,(void (*)())T_Projectile,NULL,s_needle3},
/* s_needle3  */	{false,SPR_HYPO3,6,(void (*)())T_Projectile,NULL,s_needle4},
/* s_needle4  */	{false,SPR_HYPO4,6,(void (*)())T_Projectile,NULL,s_needle1},



/* s_giftstand */	{false,SPR_GIFT_W1,0,(void (*)())T_Stand,NULL,s_giftstand},

/* s_giftchase1  */	{false,SPR_GIFT_W1,10,(void (*)())T_Gift,NULL,s_giftchase1s},
/* s_giftchase1s */	{false,SPR_GIFT_W1,3,NULL,NULL,s_giftchase2},
/* s_giftchase2  */	{false,SPR_GIFT_W2,8,(void (*)())T_Gift,NULL,s_giftchase3},
/* s_giftchase3  */	{false,SPR_GIFT_W3,10,(void (*)())T_Gift,NULL,s_giftchase3s},
/* s_giftchase3s */	{false,SPR_GIFT_W3,3,NULL,NULL,s_giftchase4},
/* s_giftchase4  */	{false,SPR_GIFT_W4,8,(void (*)())T_Gift,NULL,s_giftchase1},

/* s_giftdeathcam */	{false,SPR_GIFT_W1,1,NULL,NULL,s_giftdie1},

/* s_giftdie1 */	{false,SPR_GIFT_W1,1,NULL,(void (*)())A_DeathScream,s_giftdie2},
/* s_giftdie2 */	{false,SPR_GIFT_W1,10,NULL,NULL,s_giftdie3},
/* s_giftdie3 */	{false,SPR_GIFT_DIE1,10,NULL,NULL,s_giftdie4},
/* s_giftdie4 */	{false,SPR_GIFT_DIE2,10,NULL,NULL,s_giftdie5},
/* s_giftdie5 */	{false,SPR_GIFT_DIE3,10,NULL,NULL,s_giftdie6},
/* s_giftdie6 */	{false,SPR_GIFT_DEAD,20,NULL,(void (*)())A_StartDeathCam,s_giftdie6},

/* s_giftshoot1  */	{false,SPR_GIFT_SHOOT1,30,NULL,NULL,s_giftshoot2},
/* s_giftshoot2  */	{false,SPR_GIFT_SHOOT2,10,NULL,(void (*)())T_GiftThrow,s_giftchase1},



/* s_fatstand */	{false,SPR_FAT_W1,0,(void (*)())T_Stand,NULL,s_fatstand},

/* s_fatchase1  */	{false,SPR_FAT_W1,10,(void (*)())T_Fat,NULL,s_fatchase1s},
/* s_fatchase1s */	{false,SPR_FAT_W1,3,NULL,NULL,s_fatchase2},
/* s_fatchase2  */	{false,SPR_FAT_W2,8,(void (*)())T_Fat,NULL,s_fatchase3},
/* s_fatchase3  */	{false,SPR_FAT_W3,10,(void (*)())T_Fat,NULL,s_fatchase3s},
/* s_fatchase3s */	{false,SPR_FAT_W3,3,NULL,NULL,s_fatchase4},
/* s_fatchase4  */	{false,SPR_FAT_W4,8,(void (*)())T_Fat,NULL,s_fatchase1},

/* s_fatdeathcam */	{false,SPR_FAT_W1,1,NULL,NULL,s_fatdie1},

/* s_fatdie1 */	{false,SPR_FAT_W1,1,NULL,(void (*)())A_DeathScream,s_fatdie2},
/* s_fatdie2 */	{false,SPR_FAT_W1,10,NULL,NULL,s_fatdie3},
/* s_fatdie3 */	{false,SPR_FAT_DIE1,10,NULL,NULL,s_fatdie4},
/* s_fatdie4 */	{false,SPR_FAT_DIE2,10,NULL,NULL,s_fatdie5},
/* s_fatdie5 */	{false,SPR_FAT_DIE3,10,NULL,NULL,s_fatdie6},
/* s_fatdie6 */	{false,SPR_FAT_DEAD,20,NULL,(void (*)())A_StartDeathCam,s_fatdie6},

/* s_fatshoot1  */	{false,SPR_FAT_SHOOT1,30,NULL,NULL,s_fatshoot2},
/* s_fatshoot2  */	{false,SPR_FAT_SHOOT2,10,NULL,(void (*)())T_GiftThrow,s_fatshoot3},
/* s_fatshoot3  */	{false,SPR_FAT_SHOOT3,10,NULL,(void (*)())T_Shoot,s_fatshoot4},
/* s_fatshoot4  */	{false,SPR_FAT_SHOOT4,10,NULL,(void (*)())T_Shoot,s_fatshoot5},
/* s_fatshoot5  */	{false,SPR_FAT_SHOOT3,10,NULL,(void (*)())T_Shoot,s_fatshoot6},
/* s_fatshoot6  */	{false,SPR_FAT_SHOOT4,10,NULL,(void (*)())T_Shoot,s_fatchase1},


/* s_fakestand */	{false,SPR_FAKE_W1,0,(void (*)())T_Stand,NULL,s_fakestand},

/* s_fakechase1  */	{false,SPR_FAKE_W1,10,(void (*)())T_Fake,NULL,s_fakechase1s},
/* s_fakechase1s */	{false,SPR_FAKE_W1,3,NULL,NULL,s_fakechase2},
/* s_fakechase2  */	{false,SPR_FAKE_W2,8,(void (*)())T_Fake,NULL,s_fakechase3},
/* s_fakechase3  */	{false,SPR_FAKE_W3,10,(void (*)())T_Fake,NULL,s_fakechase3s},
/* s_fakechase3s */	{false,SPR_FAKE_W3,3,NULL,NULL,s_fakechase4},
/* s_fakechase4  */	{false,SPR_FAKE_W4,8,(void (*)())T_Fake,NULL,s_fakechase1},

/* s_fakedie1 */	{false,SPR_FAKE_DIE1,10,NULL,(void (*)())A_DeathScream,s_fakedie2},
/* s_fakedie2 */	{false,SPR_FAKE_DIE2,10,NULL,NULL,s_fakedie3},
/* s_fakedie3 */	{false,SPR_FAKE_DIE3,10,NULL,NULL,s_fakedie4},
/* s_fakedie4 */	{false,SPR_FAKE_DIE4,10,NULL,NULL,s_fakedie5},
/* s_fakedie5 */	{false,SPR_FAKE_DIE5,10,NULL,NULL,s_fakedie6},
/* s_fakedie6 */	{false,SPR_FAKE_DEAD,0,NULL,NULL,s_fakedie6},

/* s_fakeshoot1  */	{false,SPR_FAKE_SHOOT,8,NULL,(void (*)())T_FakeFire,s_fakeshoot2},
/* s_fakeshoot2  */	{false,SPR_FAKE_SHOOT,8,NULL,(void (*)())T_FakeFire,s_fakeshoot3},
/* s_fakeshoot3  */	{false,SPR_FAKE_SHOOT,8,NULL,(void (*)())T_FakeFire,s_fakeshoot4},
/* s_fakeshoot4  */	{false,SPR_FAKE_SHOOT,8,NULL,(void (*)())T_FakeFire,s_fakeshoot5},
/* s_fakeshoot5  */	{false,SPR_FAKE_SHOOT,8,NULL,(void (*)())T_FakeFire,s_fakeshoot6},
/* s_fakeshoot6  */	{false,SPR_FAKE_SHOOT,8,NULL,(void (*)())T_FakeFire,s_fakeshoot7},
/* s_fakeshoot7  */	{false,SPR_FAKE_SHOOT,8,NULL,(void (*)())T_FakeFire,s_fakeshoot8},
/* s_fakeshoot8  */	{false,SPR_FAKE_SHOOT,8,NULL,(void (*)())T_FakeFire,s_fakeshoot9},
/* s_fakeshoot9  */	{false,SPR_FAKE_SHOOT,8,NULL,NULL,s_fakechase1},

/* s_fire1  */	{false,SPR_FIRE1,6,NULL,(void (*)())T_Projectile,s_fire2},
/* s_fire2  */	{false,SPR_FIRE2,6,NULL,(void (*)())T_Projectile,s_fire1},


/* s_mechastand */	{false,SPR_MECHA_W1,0,(void (*)())T_Stand,NULL,s_mechastand},

/* s_mechachase1  */	{false,SPR_MECHA_W1,10,(void (*)())T_Chase,(void (*)())A_MechaSound,s_mechachase1s},
/* s_mechachase1s */	{false,SPR_MECHA_W1,6,NULL,NULL,s_mechachase2},
/* s_mechachase2  */	{false,SPR_MECHA_W2,8,(void (*)())T_Chase,NULL,s_mechachase3},
/* s_mechachase3  */	{false,SPR_MECHA_W3,10,(void (*)())T_Chase,(void (*)())A_MechaSound,s_mechachase3s},
/* s_mechachase3s */	{false,SPR_MECHA_W3,6,NULL,NULL,s_mechachase4},
/* s_mechachase4  */	{false,SPR_MECHA_W4,8,(void (*)())T_Chase,NULL,s_mechachase1},

/* s_mechadie1 */	{false,SPR_MECHA_DIE1,10,NULL,(void (*)())A_DeathScream,s_mechadie2},
/* s_mechadie2 */	{false,SPR_MECHA_DIE2,10,NULL,NULL,s_mechadie3},
/* s_mechadie3 */	{false,SPR_MECHA_DIE3,10,NULL,(void (*)())A_HitlerMorph,s_mechadie4},
/* s_mechadie4 */	{false,SPR_MECHA_DEAD,0,NULL,NULL,s_mechadie4},

/* s_mechashoot1  */	{false,SPR_MECHA_SHOOT1,30,NULL,NULL,s_mechashoot2},
/* s_mechashoot2  */	{false,SPR_MECHA_SHOOT2,10,NULL,(void (*)())T_Shoot,s_mechashoot3},
/* s_mechashoot3  */	{false,SPR_MECHA_SHOOT3,10,NULL,(void (*)())T_Shoot,s_mechashoot4},
/* s_mechashoot4  */	{false,SPR_MECHA_SHOOT2,10,NULL,(void (*)())T_Shoot,s_mechashoot5},
/* s_mechashoot5  */	{false,SPR_MECHA_SHOOT3,10,NULL,(void (*)())T_Shoot,s_mechashoot6},
/* s_mechashoot6  */	{false,SPR_MECHA_SHOOT2,10,NULL,(void (*)())T_Shoot,s_mechachase1},


/* s_hitlerchase1  */	{false,SPR_HITLER_W1,6,(void (*)())T_Chase,NULL,s_hitlerchase1s},
/* s_hitlerchase1s */	{false,SPR_HITLER_W1,4,NULL,NULL,s_hitlerchase2},
/* s_hitlerchase2  */	{false,SPR_HITLER_W2,2,(void (*)())T_Chase,NULL,s_hitlerchase3},
/* s_hitlerchase3  */	{false,SPR_HITLER_W3,6,(void (*)())T_Chase,NULL,s_hitlerchase3s},
/* s_hitlerchase3s */	{false,SPR_HITLER_W3,4,NULL,NULL,s_hitlerchase4},
/* s_hitlerchase4  */	{false,SPR_HITLER_W4,2,(void (*)())T_Chase,NULL,s_hitlerchase1},

/* s_hitlerdeathcam */	{false,SPR_HITLER_W1,10,NULL,NULL,s_hitlerdie1},

/* s_hitlerdie1 */	{false,SPR_HITLER_W1,1,NULL,(void (*)())A_DeathScream,s_hitlerdie2},
/* s_hitlerdie2 */	{false,SPR_HITLER_W1,10,NULL,NULL,s_hitlerdie3},
/* s_hitlerdie3 */	{false,SPR_HITLER_DIE1,10,NULL,(void (*)())A_Slurpie,s_hitlerdie4},
/* s_hitlerdie4 */	{false,SPR_HITLER_DIE2,10,NULL,NULL,s_hitlerdie5},
/* s_hitlerdie5 */	{false,SPR_HITLER_DIE3,10,NULL,NULL,s_hitlerdie6},
/* s_hitlerdie6 */	{false,SPR_HITLER_DIE4,10,NULL,NULL,s_hitlerdie7},
/* s_hitlerdie7 */	{false,SPR_HITLER_DIE5,10,NULL,NULL,s_hitlerdie8},
/* s_hitlerdie8 */	{false,SPR_HITLER_DIE6,10,NULL,NULL,s_hitlerdie9},
/* s_hitlerdie9 */	{false,SPR_HITLER_DIE7,10,NULL,NULL,s_hitlerdie10},
/* s_hitlerdie10 */	{false,SPR_HITLER_DEAD,20,NULL,(void (*)())A_StartDeathCam,s_hitlerdie10},

/* s_hitlershoot1  */	{false,SPR_HITLER_SHOOT1,30,NULL,NULL,s_hitlershoot2},
/* s_hitlershoot2  */	{false,SPR_HITLER_SHOOT2,10,NULL,(void (*)())T_Shoot,s_hitlershoot3},
/* s_hitlershoot3  */	{false,SPR_HITLER_SHOOT3,10,NULL,(void (*)())T_Shoot,s_hitlershoot4},
/* s_hitlershoot4  */	{false,SPR_HITLER_SHOOT2,10,NULL,(void (*)())T_Shoot,s_hitlershoot5},
/* s_hitlershoot5  */	{false,SPR_HITLER_SHOOT3,10,NULL,(void (*)())T_Shoot,s_hitlershoot6},
/* s_hitlershoot6  */	{false,SPR_HITLER_SHOOT2,10,NULL,(void (*)())T_Shoot,s_hitlerchase1},



/* s_bjrun1  */	{false,SPR_BJ_W1,12,(void (*)())T_BJRun,NULL,s_bjrun1s},
/* s_bjrun1s */	{false,SPR_BJ_W1,3, NULL,NULL,s_bjrun2},
/* s_bjrun2  */	{false,SPR_BJ_W2,8,(void (*)())T_BJRun,NULL,s_bjrun3},
/* s_bjrun3  */	{false,SPR_BJ_W3,12,(void (*)())T_BJRun,NULL,s_bjrun3s},
/* s_bjrun3s */	{false,SPR_BJ_W3,3, NULL,NULL,s_bjrun4},
/* s_bjrun4  */	{false,SPR_BJ_W4,8,(void (*)())T_BJRun,NULL,s_bjrun1},


/* s_bjjump1 */	{false,SPR_BJ_JUMP1,14,(void (*)())T_BJJump,NULL,s_bjjump2},
/* s_bjjump2 */	{false,SPR_BJ_JUMP2,14,(void (*)())T_BJJump,(void (*)())T_BJYell,s_bjjump3},
/* s_bjjump3 */	{false,SPR_BJ_JUMP3,14,(void (*)())T_BJJump,NULL,s_bjjump4},
/* s_bjjump4 */	{false,SPR_BJ_JUMP4,300,NULL,(void (*)())T_BJDone,s_bjjump4},


/* s_deathcam */	{false,0,0,NULL,NULL, s_none},


#endif


/* s_player */	{false,0,0,(void (*)())T_Player,NULL, s_none},
/* s_attack */	{false,0,0,(void (*)())T_Attack,NULL, s_none}
};


/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define PROJECTILESIZE	0xc000l

#define BJRUNSPEED	2048
#define BJJUMPSPEED	680

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

static const int starthitpoints[4][NUMENEMIES] =
	 //
	 // BABY MODE
	 //
	 {
	 {25,	// guards
	  50,	// officer
	  100,	// SS
	  1,	// dogs
	  850,	// Hans
	  850,	// Schabbs
	  200,	// fake hitler
	  800,	// mecha hitler
	  45,	// mutants
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts

	  850,	// Gretel
	  850,	// Gift
	  850,	// Fat
	  5,	// en_spectre,
	  1450,	// en_angel,
	  850,	// en_trans,
	  1050,	// en_uber,
	  950,	// en_will,
	  1250	// en_death
	  },
	 //
	 // DON'T HURT ME MODE
	 //
	 {25,	// guards
	  50,	// officer
	  100,	// SS
	  1,	// dogs
	  950,	// Hans
	  950,	// Schabbs
	  300,	// fake hitler
	  950,	// mecha hitler
	  55,	// mutants
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts

	  950,	// Gretel
	  950,	// Gift
	  950,	// Fat
	  10,	// en_spectre,
	  1550,	// en_angel,
	  950,	// en_trans,
	  1150,	// en_uber,
	  1050,	// en_will,
	  1350	// en_death
	  },
	 //
	 // BRING 'EM ON MODE
	 //
	 {25,	// guards
	  50,	// officer
	  100,	// SS
	  1,	// dogs

	  1050,	// Hans
	  1550,	// Schabbs
	  400,	// fake hitler
	  1050,	// mecha hitler

	  55,	// mutants
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts

	  1050,	// Gretel
	  1050,	// Gift
	  1050,	// Fat
	  15,	// en_spectre,
	  1650,	// en_angel,
	  1050,	// en_trans,
	  1250,	// en_uber,
	  1150,	// en_will,
	  1450	// en_death
	  },
	 //
	 // DEATH INCARNATE MODE
	 //
	 {25,	// guards
	  50,	// officer
	  100,	// SS
	  1,	// dogs

	  1200,	// Hans
	  2400,	// Schabbs
	  500,	// fake hitler
	  1200,	// mecha hitler

	  65,	// mutants
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts
	  25,	// ghosts

	  1200,	// Gretel
	  1200,	// Gift
	  1200,	// Fat
	  25,	// en_spectre,
	  2000,	// en_angel,
	  1200,	// en_trans,
	  1400,	// en_uber,
	  1300,	// en_will,
	  1600	// en_death
}};

short atan2fix(fixed x, fixed y)
{
    boolean negative;
    long long quot;
    fixed tang;
    int offset;
    int res;
    if (x < 0) {
	x = -x;
	negative = true;
	offset = 180;
    } else {
	negative = false;
	offset = 0;
    }
    if (y < 0) {
	y = -y;
	negative = !negative;
	if (negative)
	    offset = 360;
    }
    if (x == 0)
      return negative ? 270 : 90;
    if (y == 0)
      return offset;
    quot = ((long long)y << 32) / x;
    tang = (fixed)quot;
    if (quot != tang) {
	/* Overflow.  */
	res = 90;
    } else {
	int low = 0;
	int high = FINEANGLES / 4 - 1;

	while (low + 1 < high) {
	    res = (low + high) >> 1;
	    if (finetangent[res] < tang)
		high = res;
	    else
		low = res;
	}
	res = res / (FINEANGLES / ANGLES);
    }
    if (negative)
	res = -res;
    return res + offset;
}

/*
=================
=
= A_Smoke
=
=================
*/

void A_Smoke(objtype *ob)
{
	GetNewActor();
#ifdef SPEAR
	if (ob->obclass == hrocketobj)
		newobj->state = s_hsmoke1;
	else
#endif
		newobj->state = s_smoke1;
	newobj->ticcount = 6;

	newobj->tilex = ob->tilex;
	newobj->tiley = ob->tiley;
	newobj->x = ob->x;
	newobj->y = ob->y;
	newobj->obclass = inertobj;
	newobj->active = ac_yes;

	newobj->flags = FL_NEVERMARK;
}


/*
===================
=
= ProjectileTryMove
=
= returns true if move ok
===================
*/

#define PROJSIZE	0x2000

boolean ProjectileTryMove(objtype *ob)
{
	int xl,yl,xh,yh,x,y;

	xl = (ob->x-PROJSIZE) >>TILESHIFT;
	yl = (ob->y-PROJSIZE) >>TILESHIFT;

	xh = (ob->x+PROJSIZE) >>TILESHIFT;
	yh = (ob->y+PROJSIZE) >>TILESHIFT;

/* check for solid walls */
	for (y=yl;y<=yh;y++) {
		for (x=xl;x<=xh;x++) {
//		if (actorat[x][y] && !(actorat[x][y] & 0x8000))
		if (solid_actor_at(x, y))
				return false;
		}
	}

	return true;
}



/*
=================
=
= T_Projectile
=
=================
*/

void T_Projectile(objtype *ob)
{
	long	deltax,deltay;
	int		damage;
	long	speed;

	speed = (long)ob->speed*tics;

	deltax = FixedMul(speed,costable[ob->angle]);
	deltay = -FixedMul(speed,sintable[ob->angle]);

	if (deltax>0x10000l)
		deltax = 0x10000l;
	if (deltay>0x10000l)
		deltay = 0x10000l;

	ob->x += deltax;
	ob->y += deltay;

	deltax = labs(ob->x - player->x);
	deltay = labs(ob->y - player->y);

	if (!ProjectileTryMove(ob))
	{
		if (ob->obclass == rocketobj)
		{
			SD_PlaySound(MISSILEHITSND);
			ob->state = s_boom1;
		}
#ifdef SPEAR
		else if (ob->obclass == hrocketobj)
		{
			SD_PlaySound(MISSILEHITSND);
			ob->state = s_hboom1;
		}
#endif
		else
			ob->state = s_none;		// mark for removal

		return;
	}

	if (deltax < PROJECTILESIZE && deltay < PROJECTILESIZE)
	{	// hit the player
		switch (ob->obclass)
		{
		case needleobj:
			damage = (US_RndT() >>3) + 20;
			break;
		case rocketobj:
		case hrocketobj:
		case sparkobj:
			damage = (US_RndT() >>3) + 30;
			break;
		case fireobj:
			damage = (US_RndT() >>3);
			break;
		default:
			return;
		}

		TakeDamage(damage, ob);
		ob->state = s_none;		// mark for removal
		return;
	}

	ob->tilex = ob->x >> TILESHIFT;
	ob->tiley = ob->y >> TILESHIFT;

}

/*
=============================================================================

							GUARD

=============================================================================
*/


/*
===============
=
= SpawnStand
=
===============
*/

void SpawnStand (enemy_t which, int tilex, int tiley, int dir)
{
	word *map, tile = 0;

	switch (which)
	{
	case en_guard:
		SpawnNewObj (tilex,tiley,s_grdstand);
		newobj->speed = SPDPATROL;
		//if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_officer:
		SpawnNewObj (tilex,tiley,s_ofcstand);
		newobj->speed = SPDPATROL;
		//if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_mutant:
		SpawnNewObj (tilex,tiley,s_mutstand);
		newobj->speed = SPDPATROL;
		//if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_ss:
		SpawnNewObj (tilex,tiley,s_ssstand);
		newobj->speed = SPDPATROL;
		//if (!loadedgame)
		  gamestate.killtotal++;
		break;
	default:
		break;
	}


	map = mapsegs[0]+farmapylookup[tiley]+tilex;
	if (*map == AMBUSHTILE)
	{
		tilemap[tilex][tiley] = 0;

		if (*(map+1) >= AREATILE)
			tile = *(map+1);
		if (*(map-mapwidth) >= AREATILE)
			tile = *(map-mapwidth);
		if (*(map+mapwidth) >= AREATILE)
			tile = *(map+mapwidth);
		if ( *(map-1) >= AREATILE)
			tile = *(map-1);

		*map = tile;
		newobj->areanumber = tile-AREATILE;

		newobj->flags |= FL_AMBUSH;
	}

	newobj->obclass = (classtype)(guardobj+which);
	newobj->hitpoints = starthitpoints[gamestate.difficulty][which];
	newobj->dir = (dirtype)(dir*2);
	newobj->flags |= FL_SHOOTABLE;
}



/*
===============
=
= SpawnDeadGuard
=
===============
*/

void SpawnDeadGuard (int tilex, int tiley)
{
	SpawnNewObj (tilex,tiley,s_grddie4);
	newobj->obclass = inertobj;
//    newobj->flags = FL_NEVERMARK;	
}



/*
===============
=
= SpawnPatrol
=
===============
*/

void SpawnPatrol (enemy_t which, int tilex, int tiley, int dir)
{
	switch (which)
	{
	case en_guard:
		SpawnNewObj (tilex,tiley,s_grdpath1);
		newobj->speed = SPDPATROL;
		//if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_officer:
		SpawnNewObj (tilex,tiley,s_ofcpath1);
		newobj->speed = SPDPATROL;
		//if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_ss:
		SpawnNewObj (tilex,tiley,s_sspath1);
		newobj->speed = SPDPATROL;
		//if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_mutant:
		SpawnNewObj (tilex,tiley,s_mutpath1);
		newobj->speed = SPDPATROL;
		//if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_dog:
		SpawnNewObj (tilex,tiley,s_dogpath1);
		newobj->speed = SPDDOG;
		//if (!loadedgame)
		  gamestate.killtotal++;
		break;
	default:
		break;
	}

	newobj->obclass = (classtype)(guardobj+which);
	newobj->dir = (dirtype)(dir*2);
	newobj->hitpoints = starthitpoints[gamestate.difficulty][which];
	newobj->distance = TILEGLOBAL;
	newobj->flags |= FL_SHOOTABLE;
	newobj->active = ac_yes;

//	actorat[newobj->tilex][newobj->tiley] = 0;	// don't use original spot
	clear_actor(newobj->tilex, newobj->tiley); // don't use original spot
//	actorat[newobj->tilex][newobj->tiley] = NULL;
	
	switch (dir)
	{
	case 0:
		newobj->tilex++;
		break;
	case 1:
		newobj->tiley--;
		break;
	case 2:
		newobj->tilex--;
		break;
	case 3:
		newobj->tiley++;
		break;
	}

//	actorat[newobj->tilex][newobj->tiley] = newobj->id | 0x8000;
	move_actor(newobj);
}



/*
==================
=
= A_DeathScream
=
==================
*/

void A_DeathScream (objtype *ob)
{
#ifndef SPEAR
	if (mapon==9 && !US_RndT())
#else
	if ((mapon==18 || mapon==19) && !US_RndT())
#endif
	{
	 switch(ob->obclass)
	 {
	  case mutantobj:
	  case guardobj:
	  case officerobj:
	  case ssobj:
	  case dogobj:
		SD_PlaySound(DEATHSCREAM6SND);
		return;
	  default:
	  	break;
	 }
	}

	switch (ob->obclass)
	{
	case mutantobj:
		SD_PlaySound(AHHHGSND);
		break;

	case guardobj:
		{
#ifndef UPLOAD
		 int sounds[8]={ DEATHSCREAM1SND, DEATHSCREAM2SND,
				 DEATHSCREAM3SND, DEATHSCREAM4SND,
				 DEATHSCREAM5SND, DEATHSCREAM7SND,
				 DEATHSCREAM8SND, DEATHSCREAM9SND
		 };
		 SD_PlaySound(sounds[US_RndT()%8]);
#else
		 int sounds[3]={ DEATHSCREAM1SND,
		   		 DEATHSCREAM2SND,
				 DEATHSCREAM3SND
		 };
		 SD_PlaySound(sounds[US_RndT()%3]);
#endif
		}
		break;
	case officerobj:
		SD_PlaySound(NEINSOVASSND);
		break;
	case ssobj:
		SD_PlaySound(LEBENSND);
		break;
	case dogobj:
		SD_PlaySound(DOGDEATHSND);
		break;
#ifndef SPEAR
	case bossobj:
		SD_PlaySound(MUTTISND);
		break;
	case schabbobj:
		SD_PlaySound(MEINGOTTSND);
		break;
	case fakeobj:
		SD_PlaySound(HITLERHASND);
		break;
	case mechahitlerobj:
		SD_PlaySound(SCHEISTSND);
		break;
	case realhitlerobj:
		SD_PlaySound(EVASND);
		break;
	case gretelobj:
		SD_PlaySound(MEINSND);
		break;
	case giftobj:
		SD_PlaySound(DONNERSND);
		break;
	case fatobj:
		SD_PlaySound(ROSESND);
		break;
#else
	case spectreobj:
		SD_PlaySound(GHOSTFADESND);
		break;
	case angelobj:
		SD_PlaySound(ANGELDEATHSND);
		break;
	case transobj:
		SD_PlaySound(TRANSDEATHSND);
		break;
	case uberobj:
		SD_PlaySound(UBERDEATHSND);
		break;
	case willobj:
		SD_PlaySound(WILHELMDEATHSND);
		break;
	case deathobj:
		SD_PlaySound(KNIGHTDEATHSND);
		break;
#endif
	default:
		break;
	}
}

/*
============================================================================

								PATH

============================================================================
*/


/*
===============
=
= SelectPathDir
=
===============
*/

void SelectPathDir(objtype *ob)
{
	unsigned spot;

	spot = *(mapsegs[1]+farmapylookup[ob->tiley]+ob->tilex)-ICONARROWS;

	if (spot<8)
	{
	// new direction
		ob->dir = (dirtype)spot;
	}

	ob->distance = TILEGLOBAL;

	if (!TryWalk (ob))
		ob->dir = nodir;
}


/*
===============
=
= T_Path
=
===============
*/

void T_Path (objtype *ob)
{
	long 	move;

	if (SightPlayer (ob))
		return;

	if (ob->dir == nodir)
	{
		SelectPathDir (ob);
		if (ob->dir == nodir)
			return;					// all movement is blocked
	}


	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		if (ob->tilex>MAPSIZE || ob->tiley>MAPSIZE)
		{
			char    str[80];
			sprintf(str,"T_Path hit a wall at %u,%u, dir %u"
			,ob->tilex,ob->tiley,ob->dir);
			Quit(str);
		}



		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;
		move -= ob->distance;

		SelectPathDir (ob);

		if (ob->dir == nodir)
			return;			/* all movement is blocked */
	}
}


/*
=============================================================================

								FIGHT

=============================================================================
*/


/*
===============
=
= T_Shoot
=
= Try to damage the player, based on skill level and player's speed
=
===============
*/

void T_Shoot (objtype *ob)
{
	int	dx,dy,dist;
	int	hitchance,damage;

	hitchance = 128;

	if (!areabyplayer[ob->areanumber])
		return;

	if (!CheckLine (ob))			// player is behind a wall
	  return;

	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx:dy;

	if (ob->obclass == ssobj || ob->obclass == bossobj)
		dist = dist*2/3;					// ss are better shots

	if (thrustspeed >= RUNSPEED)
	{
		if (ob->flags&FL_VISABLE)
			hitchance = 160-dist*16;		// player can see to dodge
		else
			hitchance = 160-dist*8;
	}
	else
	{
		if (ob->flags&FL_VISABLE)
			hitchance = 256-dist*16;		// player can see to dodge
		else
			hitchance = 256-dist*8;
	}

// see if the shot was a hit

	if (US_RndT()<hitchance)
	{
		if (dist<2)
			damage = US_RndT()>>2;
		else if (dist<4)
			damage = US_RndT()>>3;
		else
			damage = US_RndT()>>4;

		TakeDamage (damage,ob);
	}

	switch(ob->obclass)
	{
	 case ssobj:
	   SD_PlaySound(SSFIRESND);
	   break;
#ifndef SPEAR
	 case giftobj:
	 case fatobj:
	   SD_PlaySound(MISSILEFIRESND);
	   break;
	 case mechahitlerobj:
	 case realhitlerobj:
	 case bossobj:
	   SD_PlaySound(BOSSFIRESND);
	   break;
	 case schabbobj:
	   SD_PlaySound(SCHABBSTHROWSND);
	   break;
	 case fakeobj:
	   SD_PlaySound(FLAMETHROWERSND);
	   break;
#endif
	 default:
	   SD_PlaySound(NAZIFIRESND);
	}

}


/*
===============
=
= T_Bite
=
===============
*/

void T_Bite (objtype *ob)
{
	long	dx,dy;

	SD_PlaySound(DOGATTACKSND);	// JAB

	dx = player->x - ob->x;
	if (dx<0)
		dx = -dx;
	dx -= TILEGLOBAL;
	if (dx <= MINACTORDIST)
	{
		dy = player->y - ob->y;
		if (dy<0)
			dy = -dy;
		dy -= TILEGLOBAL;
		if (dy <= MINACTORDIST)
		{
		   if (US_RndT()<180)
		   {
			   TakeDamage (US_RndT()>>4,ob);
			   return;
		   }
		}
	}

	return;
}

/*
=============================================================================

						 SPEAR ACTORS

=============================================================================
*/

#ifdef SPEAR

/*
===============
=
= SpawnTrans
=
===============
*/

void SpawnTrans (int tilex, int tiley)
{
	if (SoundBlasterPresent && DigiMode != sds_Off)
		gamestates[s_transdie01].tictime = 105;

	SpawnNewObj(tilex,tiley,s_transstand);
	newobj->obclass = transobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_trans];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
		gamestate.killtotal++;
}

//
// uber
//

/*
===============
=
= SpawnUber
=
===============
*/

void SpawnUber (int tilex, int tiley)
{
	if (SoundBlasterPresent && DigiMode != sds_Off)
		gamestates[s_uberdie01].tictime = 70;

	SpawnNewObj (tilex,tiley,s_uberstand);
	newobj->obclass = uberobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_uber];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
		gamestate.killtotal++;
}

/*
===============
=
= T_UShoot
=
===============
*/

void T_UShoot(objtype *ob)
{
	int dx, dy, dist;

	T_Shoot(ob);

	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;
	if (dist <= 1)
		TakeDamage (10,ob);
}

/*
===============
=
= SpawnWill
=
===============
*/

void SpawnWill(int tilex, int tiley)
{
	if (SoundBlasterPresent && DigiMode != sds_Off)
		gamestates[s_willdie2].tictime = 70;

	SpawnNewObj (tilex,tiley,s_willstand);
	newobj->obclass = willobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_will];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
		gamestate.killtotal++;
}

/*
================
=
= T_Will
=
================
*/

void T_Will(objtype *ob)
{
	long move;
	int	dx,dy,dist;
	boolean	dodge;

	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if (CheckLine(ob))						// got a shot at player?
	{
		if ( US_RndT() < (tics<<3) )
		{
		//
		// go into attack frame
		//
			if (ob->obclass == willobj)
				NewState (ob,s_willshoot1);
			else if (ob->obclass == angelobj)
				NewState (ob,s_angelshoot1);
			else
				NewState (ob,s_deathshoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dist <4)
			SelectRunDir (ob);
		else if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}



/*
===============
=
= SpawnDeath
=
===============
*/

void SpawnDeath(int tilex, int tiley)
{
	unsigned *map,tile;

	if (SoundBlasterPresent && DigiMode != sds_Off)
	    gamestates[s_deathdie2].tictime = 105;

	SpawnNewObj (tilex,tiley,s_deathstand);
	newobj->obclass = deathobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_death];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
	  gamestate.killtotal++;
}

/*
===============
=
= T_Launch
=
===============
*/

void T_Launch(objtype *ob)
{
	long deltax, deltay;
	int iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	iangle = atan2fix (deltay,deltax);

	GetNewActor();
	newobj->state = s_rocket;
	newobj->ticcount = 1;

	newobj->tilex = ob->tilex;
	newobj->tiley = ob->tiley;
	newobj->x = ob->x;
	newobj->y = ob->y;
	newobj->obclass = rocketobj;
	switch(ob->obclass)
	{
	case deathobj:
		newobj->state = s_hrocket;
		newobj->obclass = hrocketobj;
		SD_PlaySound(KNIGHTMISSILESND);
		break;
	case angelobj:
		newobj->state = s_spark1;
		newobj->obclass = sparkobj;
		SD_PlaySound(ANGELFIRESND);
		break;
	default:
		SD_PlaySound(MISSILEFIRESND);
	}

	newobj->dir = nodir;
	newobj->angle = iangle;
	newobj->speed = 0x2000l;
	newobj->flags = FL_NONMARK;
	newobj->active = true;
}

void A_Slurpie(objtype *ob)
{
	SD_PlaySound(SLURPIESND);
}

void A_Breathing(objtype *ob)
{
	SD_PlaySound(ANGELTIREDSND);
}

/*
===============
=
= SpawnAngel
=
===============
*/

void SpawnAngel(int tilex, int tiley)
{
	if (SoundBlasterPresent && DigiMode != sds_Off)
		gamestates[s_angeldie11].tictime = 105;

	SpawnNewObj (tilex,tiley,s_angelstand);
	newobj->obclass = angelobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_angel];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
		gamestate.killtotal++;
}

/*
=================
=
= A_Victory
=
=================
*/

void A_Victory(objtype *ob)
{
	playstate = ex_victorious;
}

/*
=================
=
= A_StartAttack
=
=================
*/

void A_StartAttack(objtype *ob)
{
	ob->temp1 = 0;
}

/*
=================
=
= A_Relaunch
=
=================
*/

void A_Relaunch(objtype *ob)
{
	if (++ob->temp1 == 3)
	{
		NewState(ob, s_angeltired);
		return;
	}

	if (US_RndT()&1)
	{
		NewState(ob, s_angelchase1);
		return;
	}
}

/*
===============
=
= SpawnSpectre
=
===============
*/

void SpawnSpectre(int tilex, int tiley)
{
	SpawnNewObj (tilex,tiley,s_spectrewait1);
	newobj->obclass = spectreobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_spectre];
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH; // |FL_NEVERMARK|FL_NONMARK;
	//if (!loadedgame)
		gamestate.killtotal++;
}

/*
===============
=
= A_Dormant
=
===============
*/

void A_Dormant(objtype *ob)
{
	long deltax, deltay;
	int xl,xh,yl,yh;
	int x,y;
	unsigned tile;

	deltax = ob->x - player->x;
	if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
		goto moveok;
	deltay = ob->y - player->y;
	if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
		goto moveok;

	return;
moveok:

	xl = (ob->x-MINDIST) >> TILESHIFT;
	xh = (ob->x+MINDIST) >> TILESHIFT;
	yl = (ob->y-MINDIST) >> TILESHIFT;
	yh = (ob->y+MINDIST) >> TILESHIFT;

	for (y = yl; y <= yh; y++)
		for (x = xl; x <= xh; x++)
		{
			if (!obj_actor_at(x, y))
				continue;
			tile = get_actor_at(x, y);
			if (objlist[tile].flags & FL_SHOOTABLE)
				return;			
		}

	ob->flags |= FL_AMBUSH | FL_SHOOTABLE;
	ob->flags &= ~FL_ATTACKMODE;
	ob->dir = nodir;
	NewState(ob, s_spectrewait1);
}


#endif

/*
=============================================================================

						 SCHABBS / GIFT / FAT

=============================================================================
*/

#ifndef SPEAR

/*
===============
=
= SpawnBoss
=
===============
*/

void SpawnBoss (int tilex, int tiley)
{
	SpawnNewObj (tilex,tiley,s_bossstand);
	newobj->speed = SPDPATROL;

	newobj->obclass = bossobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_boss];
	newobj->dir = south;
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
	  gamestate.killtotal++;
}

/*
===============
=
= SpawnGretel
=
===============
*/

void SpawnGretel (int tilex, int tiley)
{
	SpawnNewObj (tilex,tiley,s_gretelstand);
	newobj->speed = SPDPATROL;

	newobj->obclass = gretelobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_gretel];
	newobj->dir = north;
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
	  gamestate.killtotal++;
}

/*
===============
=
= SpawnGhosts
=
===============
*/

void SpawnGhosts (int which, int tilex, int tiley)
{
	switch(which)
	{
	 case en_blinky:
	   SpawnNewObj (tilex,tiley,s_blinkychase1);
	   break;
	 case en_clyde:
	   SpawnNewObj (tilex,tiley,s_clydechase1);
	   break;
	 case en_pinky:
	   SpawnNewObj (tilex,tiley,s_pinkychase1);
	   break;
	 case en_inky:
	   SpawnNewObj (tilex,tiley,s_inkychase1);
	   break;
	}

	newobj->obclass = ghostobj;
	newobj->speed = SPDDOG;

	newobj->dir = east;
	newobj->flags |= FL_AMBUSH;
	//if (!loadedgame)
	  gamestate.killtotal++;
}

/*
===============
=
= SpawnSchabbs
=
===============
*/

void SpawnSchabbs(int tilex, int tiley)
{
	if (DigiMode != sds_Off) /* TODO: what? */
		gamestates[s_schabbdie2].tictime = 140;
	else
		gamestates[s_schabbdie2].tictime = 5;

	SpawnNewObj(tilex, tiley, s_schabbstand);
	newobj->speed = SPDPATROL;

	newobj->obclass = schabbobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_schabbs];
	newobj->dir = south;
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
	  gamestate.killtotal++;
}


/*
===============
=
= SpawnGift
=
===============
*/

void SpawnGift (int tilex, int tiley)
{
	if (DigiMode != sds_Off)
		gamestates[s_giftdie2].tictime = 140;
	else
		gamestates[s_giftdie2].tictime = 5;

	SpawnNewObj (tilex,tiley,s_giftstand);
	newobj->speed = SPDPATROL;

	newobj->obclass = giftobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_gift];
	newobj->dir = north;
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
	  gamestate.killtotal++;
}


/*
===============
=
= SpawnFat
=
===============
*/

void SpawnFat (int tilex, int tiley)
{
	if (DigiMode != sds_Off)
		gamestates[s_fatdie2].tictime = 140;
	else
		gamestates[s_fatdie2].tictime = 5;

	SpawnNewObj (tilex,tiley,s_fatstand);
	newobj->speed = SPDPATROL;

	newobj->obclass = fatobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_fat];
	newobj->dir = south;
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
	  gamestate.killtotal++;
}


/*
=================
=
= T_SchabbThrow
=
=================
*/

void T_SchabbThrow (objtype *ob)
{
	long	deltax,deltay;
	int		iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	iangle = atan2fix (deltay,deltax);

	GetNewActor ();
	newobj->state = s_needle1;
	newobj->ticcount = 1;

	newobj->tilex = ob->tilex;
	newobj->tiley = ob->tiley;
	newobj->x = ob->x;
	newobj->y = ob->y;
	newobj->obclass = needleobj;
	newobj->dir = nodir;
	newobj->angle = iangle;
	newobj->speed = 0x2000l;

	newobj->flags = FL_NONMARK;
	newobj->active = ac_yes;

	SD_PlaySound(SCHABBSTHROWSND);
}

/*
=================
=
= T_GiftThrow
=
=================
*/

void T_GiftThrow(objtype *ob)
{
	long deltax,deltay;
	int iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	iangle = atan2fix (deltay,deltax);

	GetNewActor ();
	newobj->state = s_rocket;
	newobj->ticcount = 1;

	newobj->tilex = ob->tilex;
	newobj->tiley = ob->tiley;
	newobj->x = ob->x;
	newobj->y = ob->y;
	newobj->obclass = rocketobj;
	newobj->dir = nodir;
	newobj->angle = iangle;
	newobj->speed = 0x2000l;
	newobj->flags = FL_NONMARK;
	newobj->active = ac_yes;

	SD_PlaySound(MISSILEFIRESND);
}



/*
=================
=
= T_Schabb
=
=================
*/

void T_Schabb (objtype *ob)
{
	long move;
	int	dx,dy,dist;
	boolean	dodge;

	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if (CheckLine(ob))						// got a shot at player?
	{

		if ( US_RndT() < (tics<<3) )
		{
		//
		// go into attack frame
		//
			NewState (ob,s_schabbshoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dist <4)
			SelectRunDir (ob);
		else if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}




/*
=================
=
= T_Gift
=
=================
*/

void T_Gift (objtype *ob)
{
	long move;
	int	dx,dy,dist;
	boolean	dodge;

	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if (CheckLine(ob))						// got a shot at player?
	{

		if ( US_RndT() < (tics<<3) )
		{
		//
		// go into attack frame
		//
			NewState (ob,s_giftshoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dist <4)
			SelectRunDir (ob);
		else if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}




/*
=================
=
= T_Fat
=
=================
*/

void T_Fat (objtype *ob)
{
	long move;
	int	dx,dy,dist;
	boolean	dodge;

	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if (CheckLine(ob))						// got a shot at player?
	{

		if ( US_RndT() < (tics<<3) )
		{
		//
		// go into attack frame
		//
			NewState (ob,s_fatshoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dist <4)
			SelectRunDir (ob);
		else if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}



/*
=============================================================================

							HITLERS

=============================================================================
*/


/*
===============
=
= SpawnFakeHitler
=
===============
*/

void SpawnFakeHitler(int tilex, int tiley)
{
	if (DigiMode != sds_Off)
		gamestates[s_hitlerdie2].tictime = 140;
	else
		gamestates[s_hitlerdie2].tictime = 5;

	SpawnNewObj(tilex, tiley, s_fakestand);
	newobj->speed = SPDPATROL;

	newobj->obclass = fakeobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_fake];
	newobj->dir = north;
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
	  gamestate.killtotal++;
}


/*
===============
=
= SpawnHitler
=
===============
*/

void SpawnHitler(int tilex, int tiley)
{
	if (DigiMode != sds_Off)
		gamestates[s_hitlerdie2].tictime = 140;
	else
		gamestates[s_hitlerdie2].tictime = 5;


	SpawnNewObj (tilex,tiley,s_mechastand);
	newobj->speed = SPDPATROL;

	newobj->obclass = mechahitlerobj;
	newobj->hitpoints = starthitpoints[gamestate.difficulty][en_hitler];
	newobj->dir = south;
	newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;
	//if (!loadedgame)
	  gamestate.killtotal++;
}


/*
===============
=
= A_HitlerMorph
=
===============
*/

void A_HitlerMorph (objtype *ob)
{
	const word hitpoints[4]={500,700,800,900};

	SpawnNewObj (ob->tilex,ob->tiley,s_hitlerchase1);
	newobj->speed = SPDPATROL*5;

	newobj->x = ob->x;
	newobj->y = ob->y;

	newobj->distance = ob->distance;
	newobj->dir = ob->dir;
	newobj->flags = ob->flags | FL_SHOOTABLE;

	newobj->obclass = realhitlerobj;
	newobj->hitpoints = hitpoints[gamestate.difficulty];
}


////////////////////////////////////////////////////////
//
// A_MechaSound
// A_Slurpie
//
////////////////////////////////////////////////////////
void A_MechaSound(objtype *ob)
{
	if (areabyplayer[ob->areanumber])
		SD_PlaySound(MECHSTEPSND);
}


void A_Slurpie(objtype *ob)
{
	SD_PlaySound(SLURPIESND);
}

/*
=================
=
= T_FakeFire
=
=================
*/

void T_FakeFire (objtype *ob)
{
	long	deltax,deltay;
	int		iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	iangle = atan2fix (deltay,deltax);

	GetNewActor ();
	newobj->state = s_fire1;
	newobj->ticcount = 1;

	newobj->tilex = ob->tilex;
	newobj->tiley = ob->tiley;
	newobj->x = ob->x;
	newobj->y = ob->y;
	newobj->dir = nodir;
	newobj->angle = iangle;
	newobj->obclass = fireobj;
	newobj->speed = 0x1200l;
	newobj->flags = FL_NEVERMARK;
	newobj->active = ac_yes;

	SD_PlaySound(FLAMETHROWERSND);
}

/*
=================
=
= T_Fake
=
=================
*/

void T_Fake (objtype *ob)
{
	long move;

	if (CheckLine(ob))			// got a shot at player?
	{
		if ( US_RndT() < (tics<<1) )
		{
		//
		// go into attack frame
		//
			NewState (ob,s_fakeshoot1);
			return;
		}
	}

	if (ob->dir == nodir)
	{
		SelectDodgeDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		SelectDodgeDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}

#endif
/*
============================================================================

							STAND

============================================================================
*/


/*
===============
=
= T_Stand
=
===============
*/

void T_Stand(objtype *ob)
{
	SightPlayer (ob);
}


/*
============================================================================

								CHASE

============================================================================
*/

/*
=================
=
= T_Chase
=
=================
*/

void T_Chase (objtype *ob)
{
	long move;
	int	dx,dy,dist,chance;
	boolean	dodge;

	if (gamestate.victoryflag)
		return;

	dodge = false;
	if (CheckLine(ob))	// got a shot at player?
	{
		dx = abs(ob->tilex - player->tilex);
		dy = abs(ob->tiley - player->tiley);
		dist = dx>dy ? dx : dy;
		if (!dist || (dist==1 && ob->distance<0x4000) )
			chance = 300;
		else
			chance = (tics<<4)/dist;

		if ( US_RndT()<chance)
		{
		//
		// go into attack frame
		//
			switch (ob->obclass)
			{
			case guardobj:
				NewState (ob,s_grdshoot1);
				break;
			case officerobj:
				NewState (ob,s_ofcshoot1);
				break;
			case mutantobj:
				NewState (ob,s_mutshoot1);
				break;
			case ssobj:
				NewState (ob,s_ssshoot1);
				break;
#ifndef SPEAR
			case bossobj:
				NewState (ob,s_bossshoot1);
				break;
			case gretelobj:
				NewState (ob,s_gretelshoot1);
				break;
			case mechahitlerobj:
				NewState (ob,s_mechashoot1);
				break;
			case realhitlerobj:
				NewState (ob,s_hitlershoot1);
				break;
#else
			case angelobj:
				NewState (ob,s_angelshoot1);
				break;
			case transobj:
				NewState (ob,s_transshoot1);
				break;
			case uberobj:
				NewState (ob,s_ubershoot1);
				break;
			case willobj:
				NewState (ob,s_willshoot1);
				break;
			case deathobj:
				NewState (ob,s_deathshoot1);
				break;
#endif
			default:
				break;
			}
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
		//
		// waiting for a door to open
		//
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}


/*
=================
=
= T_Ghosts
=
=================
*/

void T_Ghosts (objtype *ob)
{
	long move;


	if (ob->dir == nodir)
	{
		SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}

/*
=================
=
= T_DogChase
=
=================
*/

void T_DogChase (objtype *ob)
{
	long 	move;
	long	dx,dy;


	if (ob->dir == nodir)
	{
		SelectDodgeDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
	//
	// check for byte range
	//
		dx = player->x - ob->x;
		if (dx<0)
			dx = -dx;
		dx -= move;
		if (dx <= MINACTORDIST)
		{
			dy = player->y - ob->y;
			if (dy<0)
				dy = -dy;
			dy -= move;
			if (dy <= MINACTORDIST)
			{
				NewState (ob,s_dogjump1);
				return;
			}
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		//
		// reached goal tile, so select another one
		//

		//
		// fix position to account for round off during moving
		//
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		SelectDodgeDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}




#ifndef SPEAR
/*
============================================================================

							BJ VICTORY

============================================================================
*/



/*
===============
=
= SpawnBJVictory
=
===============
*/

void SpawnBJVictory()
{
	SpawnNewObj(player->tilex,player->tiley+1, s_bjrun1);
	newobj->x = player->x;
	newobj->y = player->y;
	newobj->obclass = bjobj;
	newobj->dir = north;
	newobj->temp1 = 6;			// tiles to run forward
}



/*
===============
=
= T_BJRun
=
===============
*/

void T_BJRun(objtype *ob)
{
	long move;

	move = BJRUNSPEED*tics;

	while (move)
	{
		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;
		move -= ob->distance;

		SelectPathDir (ob);

		if (!(--ob->temp1))
		{
			NewState(ob,s_bjjump1);
			return;
		}
	}
}


/*
===============
=
= T_BJJump
=
===============
*/

void T_BJJump(objtype *ob)
{
	long 	move;

	move = BJJUMPSPEED*tics;
	MoveObj(ob,move);
}


/*
===============
=
= T_BJYell
=
===============
*/

void T_BJYell(objtype *ob)
{
    SD_PlaySound(YEAHSND);  // JAB
}


/*
===============
=
= T_BJDone
=
===============
*/

void T_BJDone(objtype *ob)
{
	playstate = ex_victorious;				// exit castle tile
}



//===========================================================================


/*
===============
=
= CheckPosition
=
===============
*/

boolean	CheckPosition(objtype *ob)
{
	int x, y, xl, yl, xh, yh;

	xl = (ob->x-PLAYERSIZE) >>TILESHIFT;
	yl = (ob->y-PLAYERSIZE) >>TILESHIFT;

	xh = (ob->x+PLAYERSIZE) >>TILESHIFT;
	yh = (ob->y+PLAYERSIZE) >>TILESHIFT;

	//
	// check for solid walls
	//
	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			if (actorat[x][y] && !(actorat[x][y] & 0x8000))
//			if (solid_actor_at(x, y))
				return false;
		}

	return true;
}


/*
===============
=
= A_StartDeathCam
=
===============
*/

void A_StartDeathCam(objtype *ob)
{
	long	dx,dy;
	long    xmove,ymove;
	long	dist;
//	memset((byte *)curSurface->pixels, 0, screenWidth*screenHeight);		
	FinishPaletteShifts();

	VW_WaitVBL(100);

	if (gamestate.victoryflag)
	{
		playstate = ex_victorious;				// exit castle tile
		return;
	}

    gamestate.victoryflag = true;
//    unsigned fadeheight = viewsize != 21 ? screenHeight-scaleFactor*STATUSLINES : screenHeight;
//			VL_ClearScreen(bordercol);


slTVOff();			
	memset((byte *)curSurface->pixels, bordercol, screenWidth*screenHeight);
	
//    VL_BarScaledCoord (0, 0, screenWidth, fadeheight, bordercol);
//    FizzleFade(curSurface, screen, 0, 0, screenWidth, fadeheight, 70, false);

slTVOn();			

//			VL_ClearScreen(bordercol);
//while(1);
    if (bordercol != VIEWCOLOR)
    {
        CA_CacheGrChunk (STARTFONT+1);
        fontnumber = 1;
        SETFONTCOLOR(15,bordercol);
        PrintX = 68; PrintY = 45;
        US_Print (STR_SEEAGAIN);
        UNCACHEGRCHUNK(STARTFONT+1);
    }
    else
    {
        CacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);
#ifdef JAPAN
#ifndef JAPDEMO
        CA_CacheScreen(C_LETSSEEPIC);
#endif
#else
        Write(2,7,STR_SEEAGAIN);
#endif
    }
	
#ifndef USE_SPRITES
	VW_UpdateScreen();
#endif
	IN_UserInput(300);

/* line angle up exactly */
	NewState (player,s_deathcam);

	player->x = gamestate.killx;
	player->y = gamestate.killy;

	dx = ob->x - player->x;
	dy = player->y - ob->y;

	player->angle = atan2fix(dy, dx);

/* try to position as close as possible without being in a wall */
	dist = 0x14000l;
	do
	{
		xmove = FixedMul(dist, costable[player->angle]);
		ymove = -FixedMul(dist, sintable[player->angle]);

		player->x = ob->x - xmove;
		player->y = ob->y - ymove;
		dist += 0x1000;

	} while (!CheckPosition(player));

	plux = player->x >> UNSIGNEDSHIFT;			// scale to fit in unsigned
	pluy = player->y >> UNSIGNEDSHIFT;
	player->tilex = player->x >> TILESHIFT;		// scale to tile values
	player->tiley = player->y >> TILESHIFT;

//
// go back to the game
//

DrawPlayScreen(); // vbt ajout
DrawStatusBar(); // vbt ajout

//	DrawPlayBorder();
    fizzlein = true;
	
#ifndef SPEAR
	switch (ob->obclass)
	{
	case schabbobj:
		NewState(ob, s_schabbdeathcam);
		break;
	case realhitlerobj:
		NewState(ob, s_hitlerdeathcam);
		break;
	case giftobj:
		NewState(ob, s_giftdeathcam);
		break;
	case fatobj:
		NewState(ob, s_fatdeathcam);
		break;
	default:
		break;
#endif		
	}
}

#endif

