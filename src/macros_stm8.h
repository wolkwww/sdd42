//==============================================================================
//	����: macros.h
//	����: 2010-11-16
//	���.: 0.2 (�����)
//	���.: 0.2 - 201-11-17
//        �������� ������ Pin_Off()
//==============================================================================
//  2010 (C) ����� ���������� aka ZiB
//	http://ziblog.ru	zib@ziblog.ru
//==============================================================================

#ifndef MACROS_H_
#define MACROS_H_

//-- ������� �������� ----------------------------------------------------------
#define Bit(Idx)                  		(1 << (Idx))
//-- �� ������ ���� ------------------------------------------------------------
#define Bit_Set(Var, BitIdx)        ((Var) |=  Bit(BitIdx))
#define Bit_Clr(Var, BitIdx)        ((Var) &= ~Bit(BitIdx))
#define Bit_Get(Var, BitIdx)		((Var)  &  Bit(BitIdx))
#define Bit_Inv(Var, BitIdx)		((Var) ^=  Bit(BitIdx))
#define Bit_Is_Set(Var, BitIdx)		(Bit_Get(Var, BitIdx) == Bit(BitIdx))
#define Bit_Is_Clr(Var, BitIdx)		(Bit_Get(Var, BitIdx) == 0x00)
#define Set(FlagDef)				Bit_Set(FlagDef)
#define Clr(FlagDef)				Bit_Clr(FlagDef)
#define Get(FlagDef)           		Bit_Get(FlagDef)
#define Inv(FlagDef)				Bit_Inv(FlagDef)
#define Is_Set(FlagDef)        		Bit_Is_Set(FlagDef)
#define Is_Clr(FlagDef)				Bit_Is_Clr(FlagDef)
//-- �� ����� -----------------------------------------------------------------
#define Bits_Set(Var, Mask)        ((Var) |=  (Mask))
#define Bits_Clr(Var, Mask)        ((Var) &= ~(Mask))
#define Bits_Inv(Var, Mask)        ((Var) ^=  (Mask))

//-- �������� �������� ---------------------------------------------------------
#define Array_Length(Value)			(sizeof(Value) / sizeof(Value[0]))

//-- �������������� ����� ------------------------------------------------------
/*
typedef unsigned char				UInt8;
typedef unsigned short			UInt16;
typedef unsigned long				UInt32;
typedef signed char					SInt8;
typedef signed short				SInt16;
typedef signed long					SInt32;
typedef float								Float;
typedef double							Double;
typedef long double					Extended;
*/

//------------------------------------------------------------------------------
/// ��������������� ��� ������ � ������� �����-������
/// ������:
/// Pin_TEST    A, 3, High, NoPullUp, NoOpenDrain, NoSpeedLimit, NoIT
//------------------------------------------------------------------------------
// ����������� (����/�����)
#define Pin_Cfg_In(Ltr, Idx)  			{ Bit_Clr(P##Ltr##_DDR, Idx); }
#define Pin_Cfg_Out(Ltr, Idx) 			{ Bit_Set(P##Ltr##_DDR, Idx); }
// ���������� "�������������" ��������
#define Pin_Cfg_PullUp(Ltr, Idx) 		{ Bit_Set(P##Ltr##_CR1, Idx); }
#define Pin_Cfg_NoPullUp(Ltr, Idx) 		{ Bit_Clr(P##Ltr##_CR1, Idx); }
// ����������
#define Pin_Cfg_IT(Ltr, Idx)   			{ Bit_Set(P##Ltr##_CR2, Idx); }
#define Pin_Cfg_NoIT(Ltr, Idx) 			{ Bit_Clr(P##Ltr##_CR2, Idx); }
// ����������� �� �������� (��� ������)
#define Pin_Cfg_SpeedLimit(Ltr, Idx)	{ Bit_Set(P##Ltr##_CR2, Idx); }
#define Pin_Cfg_NoSpeedLimit(Ltr, Idx)	{ Bit_Clr(P##Ltr##_CR2, Idx); }
// �������� P-Buffer ()
#define Pin_Cfg_OpenDrain(Ltr, Idx)		{ Bit_Clr(P##Ltr##_CR1, Idx); }
#define Pin_Cfg_NoOpenDrain(Ltr, Idx)	{ Bit_Set(P##Ltr##_CR1, Idx); }
// ��������� ��������� ������
#define Pin_Cfg_Level_High(Ltr, Idx)	{ Bit_Set(P##Ltr##_ODR, Idx); }
#define Pin_Cfg_Level_Low(Ltr, Idx)		{ Bit_Clr(P##Ltr##_ODR, Idx); }
// ������� ����� �����-������ � ��������� ���������� �������
#define Pin_Set_On_High(Ltr, Idx)   	Pin_Cfg_Level_High(Ltr, Idx)
#define Pin_Set_On_Low(Ltr, Idx)    	Pin_Cfg_Level_Low(Ltr, Idx)
#define Pin_Set_On(Ltr, Idx, Level, PullUpRes, Driver, Speed, Interrupt) { Pin_Set_On_##Level(Ltr, Idx); }
// ������� ����� �����-������ � ��������� ����������� ����
#define Pin_Set_Off_High(Ltr, Idx)  	Pin_Cfg_Level_Low(Ltr, Idx)
#define Pin_Set_Off_Low(Ltr, Idx)   	Pin_Cfg_Level_High(Ltr, Idx)
#define Pin_Set_Off(Ltr, Idx, Level, PullUpRes, Driver, Speed, Interrupt) { Pin_Set_Off_##Level(Ltr, Idx); }
// ��������� ����������� ������ ���� �����-������
#define Pin_Set_Inv(Ltr, Idx, Level, PullUpRes, Driver, Speed, Interrupt) { Bit_Inv(P##Ltr##_ODR, Idx); }
// ������ ��������� ����� �����-������
#define Pin_Get_Sig_Low(Ltr, Idx)    (Bit_Is_Clr(Var, BitIdx))
#define Pin_Get_Sig_High(Ltr, Idx)   (Bit_Is_Set(Var, BitIdx))
#define Pin_Get_Sig(Ltr, Idx, Level, PullUpRes, Driver, Speed, Interrupt) Pin_Get_Sig_##Level(Ltr, Idx)
// ��������� ����� �����-������ �� ����
#define Pin_Set_In(Ltr, Idx, Level, PullUpRes, Driver, Speed, Interrupt) { Pin_Cfg_##Interrupt(Ltr, Idx); Pin_Cfg_In(Ltr, Idx); Pin_Cfg_##PullUpRes(Ltr, Idx); }
// ��������� ����� �����-������ �� �����
#define Pin_Set_Out(Ltr, Idx, Level, PullUpRes, Driver, Speed, Interrupt) { Pin_Cfg_##Driver(Ltr, Idx); Pin_Cfg_Out(Ltr, Idx); Pin_Cfg_##Speed(Ltr, Idx); }
// ������� ����� �����-������ � ������ ���������
#define Pin_Set_HiZ(Ltr, Idx, Level, PullUpRes, Driver, Speed, Interrupt) { Pin_##Interrupt(Ltr, Idx); Pin_Cfg_In(Ltr, Idx); Pin_##PullUpRes(Ltr, Idx); }

// �������� �������
#define Pin_In(PortDef)   		Pin_Set_In(PortDef)
#define Pin_Out(PortDef)   		Pin_Set_Out(PortDef)
#define Pin_HiZ(PortDef)      Pin_Set_HiZ(PortDef)
#define Pin_Off(PortDef)      Pin_Set_Off(PortDef)
#define Pin_On(PortDef)      	Pin_Set_On(PortDef)
#define Pin_Inv(PortDef)     	Pin_Set_Inv(PortDef)
#define Pin_Sig(PortDef)   		Pin_Get_Sig(PortDef)

#endif	// MACROS_H_
