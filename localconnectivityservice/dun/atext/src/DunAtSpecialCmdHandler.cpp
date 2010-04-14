/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Special AT command handler
*
*/

#include "DunAtSpecialCmdHandler.h"
#include "DunDebug.h"

// AT command(s) below is part of the AT&FE0Q0V1&C1&D2+IFC=3,1. command which
// is sent by MAC
_LIT8(KSpecialATCmd1, "AT&F");
// Number of special commands
const TInt KDefaultGranularity = 1;

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunAtSpecialCmdHandler* CDunAtSpecialCmdHandler::NewL()
    {
    CDunAtSpecialCmdHandler* self = new (ELeave) CDunAtSpecialCmdHandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CDunAtSpecialCmdHandler::CDunAtSpecialCmdHandler
// ---------------------------------------------------------------------------
//
CDunAtSpecialCmdHandler::CDunAtSpecialCmdHandler()
    {    
    }

// ---------------------------------------------------------------------------
// CDunAtSpecialCmdHandler::ConstructL
// ---------------------------------------------------------------------------
//
void CDunAtSpecialCmdHandler::ConstructL()
    {
    iSpecialCmds = new (ELeave) CDesC8ArrayFlat(KDefaultGranularity);
    // Add here all special commands which need to be handled    
    iSpecialCmds->AppendL(KSpecialATCmd1);
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunAtSpecialCmdHandler::~CDunAtSpecialCmdHandler()
    {
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::~CDunAtSpecialCmdHandler()") ));
    delete iSpecialCmds;
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::~CDunAtSpecialCmdHandler() complete") ));
    }

// ---------------------------------------------------------------------------
// Checks if the command has to be treated special way
// For example in case of MAC, it sends command AT&FE0Q0V1&C1&D2+IFC=3,1.
// meaning there is no delimiters in the command.
// In case of MAC we try to search AT&F (sub command) string from the beginning
// of the command. 
// Search is done character by character basis.
// ---------------------------------------------------------------------------
//
TBool CDunAtSpecialCmdHandler::IsCompleteSubCommand (TChar aCharacter)
    {
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand()") ));
    iBuffer.Append(aCharacter);
    TBool completeSubCmd = EFalse;
    
    if( !IsDataReadyForComparison( iBuffer.Length()) )
        {
        // No need to do comparison because we don't have correct amount of data
        FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand(), no need to compare") ));
        return completeSubCmd;
        }   
    
    TInt count = iSpecialCmds->Count();
    for( TInt i = 0 ; i < count ; i++ )
        {
        if( iSpecialCmds->MdcaPoint(i).Compare(iBuffer) == 0 )
            {
            FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand(), match found, cmd index %d"), i ));
            // Reset internal buffer for next comparison.
            iBuffer.FillZ();
            iBuffer.Zero();            
            completeSubCmd = ETrue;
            break;
            }
        }
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand() complete") ));
    return completeSubCmd;
    }

// ---------------------------------------------------------------------------
// Checks if the command has to be treated special way
// For example in case of MAC, it sends command AT&FE0Q0V1&C1&D2+IFC=3,1.
// meaning there is no delimiters in the command.
// In case of MAC we try to search AT&F (sub command) string from the beginning
// of the command. 
// Search is done string basis.
// ---------------------------------------------------------------------------
//
TBool CDunAtSpecialCmdHandler::IsCompleteSubCommand(TDesC8& aDes, TInt aStartIndex, TInt& aEndIndex)
    {
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand()") ));
    TBool completeSubCmd = EFalse;
    if( aDes.Length() <  MinimumLength() || aStartIndex != 0 )
        {
        // No need to do comparison because we don't have correct amount of data or
        // we are not at the beginning of the input buffer (non decoded buffer)
        FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand(), no need to compare") ));
        return completeSubCmd;
        }   
        
    TInt count = iSpecialCmds->Count();
    for( TInt i = 0 ; i < count ; i++ )
        {
        TInt length = iSpecialCmds->MdcaPoint(i).Length();
        TPtrC8 cmd = aDes.Mid(0, length);
        if( iSpecialCmds->MdcaPoint(i).Compare(cmd) == 0 )
            {
            FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand(), match found, cmd index %d"), i ));
            aEndIndex = length - 1;
            completeSubCmd = ETrue;
            break;
            }
        }
    FTRACE(FPrint( _L("CDunAtSpecialCmdHandler::IsCompleteSubCommand() complete") ));
    return completeSubCmd;
    }

// ---------------------------------------------------------------------------
// Defines when comparison is excecuted, checks if the data lengths are equal.
// ---------------------------------------------------------------------------
//
TBool CDunAtSpecialCmdHandler::IsDataReadyForComparison(TInt aLength)
    {
    TInt count = iSpecialCmds->Count();
    for( TInt i = 0 ; i < count ; i++ )
        {
        if( iSpecialCmds->MdcaPoint(i).Length() == aLength )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Defines minimum length of the special commands.
// ---------------------------------------------------------------------------
//
TInt CDunAtSpecialCmdHandler::MinimumLength()
    {
    TInt length = iSpecialCmds->MdcaPoint(0).Length();
    TInt count = iSpecialCmds->Count();
    for( TInt i = 1 ; i < count ; i++ )
        {
        if( iSpecialCmds->MdcaPoint(i).Length() < length )
            {
            length = iSpecialCmds->MdcaPoint(i).Length();
            break;
            }
        }
    return length;
    }
