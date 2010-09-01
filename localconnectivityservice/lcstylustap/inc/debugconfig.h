/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Project configure file.
*
*/


#ifndef ___DEBUGCONFIG_H__
#define ___DEBUGCONFIG_H__


#ifdef _DEBUG
#define PRJ_ENABLE_TRACE
#endif


//#define PRJ_FILE_TRACE

/**
 * build the project for module test purpose if this is defined
 */
//#define PRJ_MODULETEST_BUILD

/**
 * build the project using stubs to replace the dependencies if this is defined
 */
//#define PRJ_USE_STUB


/**
 * Custom logging variations.
 */
#ifdef PRJ_FILE_TRACE
_LIT(KLogFile,"lcstylustap.txt");
_LIT(KLogDir,"lcstylustap");
#endif

#ifdef PRJ_ENABLE_TRACE
_LIT(KTracePrefix16, "[lcstylustap] ");
_LIT8(KTracePrefix8, "[lcstylustap] ");
_LIT8(KFuncFormat8, "><%S");
_LIT8(KFuncThisFormat8, "><%S, [0x%08X]");
_LIT8(KFuncEntryFormat8, ">%S");
_LIT8(KFuncEntryThisFormat8, ">%S, [0x%08X]");
_LIT8(KFuncExitFormat8, "<%S");

_LIT(KPanicCategory, "LC lcstylustap");    
_LIT8(KPanicPrefix8, "PANIC code ");
_LIT8(KLeavePrefix8, "LEAVE code ");
#endif

const TInt KMaxLogLineLength = 512;

#define KPRINTERROR        0x00000001 // Tracing level: error
#define KPRINTINFO        0x00000002 // Tracing level: function trace
#define KPRINTSTATE        0x00000004 // Tracing level: state machine info
#define KPRINTWARNING   0x00000008 // Tracing level: warning

const TInt KTraceMask = KPRINTERROR | KPRINTINFO | KPRINTSTATE | KPRINTWARNING;

#endif // ___DEBUGCONFIG_H__
