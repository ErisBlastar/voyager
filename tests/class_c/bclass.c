/*
 * This file was generated by the NOM IDL compiler for Voyager - DO NOT EDIT!
 *
 *
 * And remember, phase 3 is near...
 */
/*
 * Built from idl/bclass.idl
 */
#ifndef NOM_BClass_IMPLEMENTATION_FILE
#define NOM_BClass_IMPLEMENTATION_FILE
#endif

#ifdef __OS2__
# define INCL_DOS
# include <os2.h>
#endif /* __OS2__ */

#include <nom.h>
#include <nomtk.h>

#include "bclass.ih"

NOM_Scope gulong NOMLINK impl_BClass_tstQueryBClassUlongVar1(BClass* nomSelf,
    CORBA_Environment *ev)
{
  BClassData* nomThis = BClassGetData(nomSelf);
  gulong nomRetval;

  return _ulVar1;
}

NOM_Scope gulong NOMLINK impl_BClass_tstQueryBClassUlongVar2(BClass* nomSelf,
    CORBA_Environment *ev)
{
  BClassData* nomThis = BClassGetData(nomSelf);

  return _ulVar2;
}

NOM_Scope void NOMLINK impl_BClass_tstSetBClassUlongVar1(BClass* nomSelf,
    const gulong ulNew,
    CORBA_Environment *ev)
{
  BClassData* nomThis = BClassGetData(nomSelf);

  g_message("In %s, setting ulVar1 to 0x%lx", __FUNCTION__, ulNew);
  _ulVar1=ulNew;
}

NOM_Scope void NOMLINK impl_BClass_tstSetBClassUlongVar2(BClass* nomSelf,
    const gulong ulNew,
    CORBA_Environment *ev)
{
  BClassData* nomThis = BClassGetData(nomSelf);

  g_message("In %s, setting ulVar2 to 0x%lx", __FUNCTION__, ulNew);
  _ulVar2=ulNew;
}

