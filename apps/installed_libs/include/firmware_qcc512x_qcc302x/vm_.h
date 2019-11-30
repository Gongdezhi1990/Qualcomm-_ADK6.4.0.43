#ifndef __VM__H__
#define __VM__H__

/*! file  @brief Definations used by VM traps */
/*! 
IRK data type.
Typed packed IRK(Identity Resolving Key) of the device. Local IRK is packed 
in uint16 IRK[].IRK packing is same as DM_SM_KEY_ID_T.
Used as param type in VmGetLocalIrk().
*/
typedef struct { uint16 irk[8]; } packed_irk;
/*! 
Root key data type.
Typed packed Root Key of the device. ER (Encryption Root) and IR (Identity Root)
root keys are packed in uint16 arrays. Used as param type in VmUpdateRootKeys().
*/
typedef struct { uint16 er[8]; uint16 ir[8]; }packed_root_keys;
      
#endif
