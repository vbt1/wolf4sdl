
   .section SEGA_P
   .align   4

!   .global  _memcpy
!   .global  _memcpyb
   .global  _memcpyw
   .global  _memcpyl
   .global  _memcpywh
   .global  _memclrwh
   .global  _MTH_Mul2
   .global  _memset4_fast

_MTH_Mul2:
	DMULS.L R4,R5
 	STS	MACH,R3
 	STS	MACL,R0
	RTS		! return
 	XTRCT	R3,R0	! xtract for Fixed32


! dst 	   = r4
! src 	   = r5
! size     = r6
!_memcpy:
!_memcpyb:
!   tst    r6,r6
!   bt     lendb

!lmemcpy:
!   mov.b  @r5+,r0
!   mov.b  r0,@r4
!   dt     r6
!   add    #1,r4
!   bf     lmemcpy
!lendb:
!   rts
!   nop

_memcpyw:
   shlr   r6
   tst    r6,r6
   bt     lendw

lmemcpyw:
   mov.w  @r5+,r0
   mov.w  r0,@r4
   dt     r6
   add    #2,r4
   bf     lmemcpyw

lendw:
   rts
   nop

_memcpyl:
   shlr2   r6
   tst    r6,r6
   bt     lendl

lmemcpyl:
   mov.l  @r5+,r0
   mov.l  r0,@r4
   dt     r6
   add    #4,r4
   bf     lmemcpyl

lendl:
   rts
   nop

_memcpywh:
   mov.l @r15,r1 ! on met 192 (le pas)
!   nop
!   bt _memcpywh
!   mov.l @(4,r15),r2  ! on met 1
!   mov.l @(8,r15),r3  ! on met 0
!   mov.l @(12,r15),r0 ! on met 0
   mov.l  r8,@-r15

!   tst    r0,r0
!   bf     lcwh_3     ! or copymode

lcwh_1:
   mov    r6,r8  ! on copie r6=320 dans r8

lcwh_2:
   mov.b  @r5,r0 ! copie la valeur de l'adresse dans r5(adresse source) dans r0
   mov.b  r0,@r4  ! copie r0 dans l'adresse de destination(r4)
   add    #1,r4 ! on se déplace de 1 dans l'adresse destination
!   add    r2,r5 ! on se déplace de 1 dans l'adresse source (param r2 inutile)
   add    #1,r5 ! on se déplace de 1 dans l'adresse source (param r2 inutile)
   dt     r8    ! on décrémente r8 (320)
!   dt     r6    ! on décrémente r6 (320)
   bf     lcwh_2 ! on boucle 320x

   add    r1,r4 ! on ajoute le pas 192 a l'adresse de destination(r4)
!   add    #192,r4 ! on ajoute le pas 192 a l'adresse de destination(r4)
!   add    r3,r5 ! on ajoute 0 à l'adresse source
   dt     r7    ! on décrémente r7 (240)
   bf     lcwh_1 ! on boucle 240x
   bra    lcwh_end
   nop

 ! void memsetl_fast(uint32_t *ptr, uint32_t value, uint32_t len);
! ptr must be 32-bit aligned. value must be non-zero. len is in bytes and must
! be a multiple of 4. None of these are checked in the function itself
! (although, it will throw an exception if the ptr isn't aligned, since the CPU
! won't do that access).

_memset4_fast:
    add     r4, r6
    add     #4, r4
.loop2:
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    mov.l   r5, @-r6
    cmp/eq  r4, r6
    bf/s    .loop2
    mov.l   r5, @-r6

    rts

    nop
  

!lcwh_3:
!   mov    r6,r8

!lcwh_4:
!   mov.b  @r5,r0
!   tst    r0,r0
!   bt     lcwh_nopix
!   mov.b  r0,@r4

!lcwh_nopix:
!   add    #1,r4
!   add    r2,r5
!   dt     r8
!   bf     lcwh_4

!   add    r1,r4
!   add    r3,r5
!   dt     r7
!   bf     lcwh_3

lcwh_end:
   rts
   mov.l  @r15+,r8

_memclrwh:        ! terulet torlo: memclrwh(byte* buf, long width, long height, long step)
                  ! step = map width - brush width
   xor    r2,r2

lswh_1:
   mov    r5,r1

lswh_2:
   mov.b  r2,@r4
   add    #1,r4
   dt     r1
   bf     lswh_2

   add    r7,r4
   dt     r6
   bf     lswh_1
   rts
   nop
/*
   .global  _getvcount
_getvcount:
   mov.l  lvdp2addr,r1
   mov.w  @(2,r1),r0
   mov.w  @(10,r1),r0
   rts
   nop

   .align 4
lvdp2addr:
   .long  0x25f80000

   .align 4
   .global  _testhi
_testhi:
   mov.l  hiaddr,r1
   mov    #120,r0
lidoit:
   mov.l  @r1,r2
   mov.l  r2,@r1
   add    #4,r1
   dt     r0
   bf     lidoit
   rts
   nop

   .align 4
hiaddr:  .long 0x26010000

   .global  _testlo
_testlo:
   mov.l  loaddr,r1
   mov    #120,r0
lodoit:
   mov.l  @r1,r2
   mov.l  r2,@r1
   add    #4,r1
   dt     r0
   bf     lodoit
   rts
   nop

   .align 4
loaddr:  .long 0x20210000

   .global  _testslow
_testslow:
   mov.l  slowaddr,r1
   mov    #120,r0
slodoit:
   mov.l  @r1,r2
   mov.l  r2,@r1
   add    #4,r1
   dt     r0
   bf     slodoit
   rts
   nop

   .align 4
slowaddr:  .long 0x24010000
*/
   .end
