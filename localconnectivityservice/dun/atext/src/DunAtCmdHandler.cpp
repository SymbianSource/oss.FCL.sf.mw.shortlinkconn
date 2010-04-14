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
* Description:  AT command handler and notifier
*
*/

/*
 * Points to consider:
 * - Each of the AT commands sent to ATEXT are converted to upper case form.
 *   Thus the ATEXT plugins don't need to check for case. The conversion to
 *   upper case form stops when carriage return or '=' character is found.
 */

/*
 * The AT command handling is splitted to two parts on high level:
 * 1) Splitter: splitting the sub-commands in a command line to multiple ones
 *    for ATEXT to process.
 * 2) Combiner: combining the replies coming from ATEXT using a filter
 *    (the filter categories are explained in DunAtCmdPusher.cpp)
 */

#include "DunAtCmdHandler.h"
#include "DunAtUrcHandler.h"
#include "DunDownstream.h"
#include "DunDebug.h"

const TInt8 KDunCancel = 24;

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
EXPORT_C CDunAtCmdHandler* CDunAtCmdHandler::NewL(
    MDunAtCmdStatusReporter* aUpstream,
    MDunStreamManipulator* aDownstream,
    const TDesC8* aConnectionName )
    {
    CDunAtCmdHandler* self = new (ELeave) CDunAtCmdHandler(
        aUpstream,
        aDownstream,
        aConnectionName );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunAtCmdHandler::~CDunAtCmdHandler()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::~CDunAtCmdHandler()") ));
    ResetData();
    FTRACE(FPrint( _L("CDunAtCmdHandler::~CDunAtCmdHandler() complete") ));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunAtCmdHandler::ResetData()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ResetData()") ));
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // NewL()
    DeletePluginHandlers();
    delete iNvramListen;
    iNvramListen = NULL;
    delete iModeListen;
    iModeListen = NULL;
    delete iEcomListen;
    iEcomListen = NULL;
    delete iAtSpecialCmdHandler;
    iAtSpecialCmdHandler = NULL;
    if ( iAtCmdExtCommon.Handle() )
        {
        iAtCmdExtCommon.SynchronousClose();
        iAtCmdExtCommon.Close();
        }
    if ( iAtCmdExt.Handle() )
        {
        iAtCmdExt.SynchronousClose();
        iAtCmdExt.Close();
        }
    iSpecials.ResetAndDestroy();
    iSpecials.Close();
    // AddCmdModeCallback()
    iCmdCallbacks.Close();
    // Internal
    Initialize();
    FTRACE(FPrint( _L("CDunAtCmdHandler::ResetData() complete") ));
    }

// ---------------------------------------------------------------------------
// Adds callback for command mode notification
// The callback will be called when command mode starts or ends
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::AddCmdModeCallback( MDunCmdModeMonitor* aCallback )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddCmdModeCallback()" ) ));
    if ( !aCallback )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::AddCmdModeCallback() (aCallback) not initialized!" ) ));
        return KErrGeneral;
        }
    TInt retTemp = iCmdCallbacks.Find( aCallback );
    if ( retTemp != KErrNotFound )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::AddCmdModeCallback() (already exists) complete" ) ));
        return KErrAlreadyExists;
        }
    retTemp = iCmdCallbacks.Append( aCallback );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::AddCmdModeCallback() (append failed!) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddCmdModeCallback() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Parses an AT command
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::ParseCommand( TDesC8& aCommand,
                                              TBool& aPartialInput )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ParseCommand()") ));
    FTRACE(FPrint( _L("CDunAtCmdHandler::ParseCommand() received:") ));
    FTRACE(FPrintRaw(aCommand) );
    iCommand = &aCommand;
    // Manage partial AT command
    TBool needsCarriage = ETrue;
    TBool okToExit = ManagePartialCommand( aCommand, needsCarriage );
    if ( okToExit )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ParseCommand() (ok to exit) complete") ));
        aPartialInput = ETrue;
        return KErrNone;
        }
    if ( iHandleState != EDunStateIdle )
        {
        aPartialInput = EFalse;
        ResetParseBuffers();
        FTRACE(FPrint( _L("CDunAtCmdHandler::ParseCommand() (not ready) complete") ));
        return KErrNotReady;
        }
    TBool pushStarted = HandleASlashCommand();
    if ( pushStarted )
        {
        // Note: return here with "partial input" status to fool CDunUpstream
        // into reissuing the read request. The AT command has not really
        // started yet so this is necessary.
        aPartialInput = ETrue;
        ResetParseBuffers();
        FTRACE(FPrint( _L("CDunAtCmdHandler::ParseCommand() (A/) complete") ));
        return KErrNone;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ParseCommand() received total:") ));
    FTRACE(FPrintRaw(iInputBuffer) );
    iHandleState = EDunStateAtCmdHandling;
    iUpstream->NotifyAtCmdHandlingStart();
    iDecodeInfo.iFirstDecode = ETrue;
    iDecodeInfo.iDecodeIndex = 0;
    HandleNextDecodedCommand();
    FTRACE(FPrint( _L("CDunAtCmdHandler::ParseCommand() (not supported) complete") ));
    aPartialInput = EFalse;
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Manages request to abort command handling
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::ManageAbortRequest()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageAbortRequest()") ));
    // Just forward the request, do no other own processing
    TInt retVal = iCmdPusher->ManageAbortRequest();
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageAbortRequest() complete") ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Sets end of command line marker on for the possible series of AT commands.
// ---------------------------------------------------------------------------
//
EXPORT_C void CDunAtCmdHandler::SetEndOfCmdLine( TBool aClearInput )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::SetEndOfCmdLine()") ));
    ManageEndOfCmdHandling( EFalse, ETrue, aClearInput );
    FTRACE(FPrint( _L("CDunAtCmdHandler::SetEndOfCmdLine() complete") ));
    }

// ---------------------------------------------------------------------------
// Stops sending of AT command from parse buffer
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::Stop()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::Stop()") ));
    // Only stop iCmdPusher here, not iUrcHandlers!
    if ( iHandleState != EDunStateAtCmdHandling )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    iCmdPusher->Stop();
    iHandleState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunAtCmdHandler::Stop() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Starts URC message handling
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::StartUrc()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::StartUrc()") ));
    TInt i;
    TInt count = iUrcHandlers.Count();
    for ( i=0; i<count; i++ )
        {
        TInt retTemp = iUrcHandlers[i]->IssueRequest();
        if ( retTemp!=KErrNone && retTemp!=KErrNotReady )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::StartUrc() (ERROR) complete") ));
            return retTemp;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::StartUrc() complete") ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops URC message handling
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CDunAtCmdHandler::StopUrc()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::StopUrc()") ));
    TInt i;
    TInt retVal = KErrNone;
    TInt count = iUrcHandlers.Count();
    for ( i=0; i<count; i++ )
        {
        retVal = iUrcHandlers[i]->Stop();
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::StopUrc() complete") ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// CDunAtCmdHandler::CDunAtCmdHandler
// ---------------------------------------------------------------------------
//
CDunAtCmdHandler::CDunAtCmdHandler( MDunAtCmdStatusReporter* aUpstream,
                                    MDunStreamManipulator* aDownstream,
                                    const TDesC8* aConnectionName ) :
    iUpstream( aUpstream ),
    iDownstream( aDownstream ),
    iConnectionName( aConnectionName )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunAtCmdHandler::ConstructL
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::ConstructL()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ConstructL()") ));
    if ( !iUpstream || !iDownstream || !iConnectionName )
        {
        User::Leave( KErrGeneral );
        }
    // Connect to AT command extension (must succeed)
    TInt retTemp = KErrNone;
    CleanupClosePushL( iAtCmdExt );
    retTemp = iAtCmdExt.Connect( EDunATExtension, *iConnectionName );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ConstructL() connect (%d)"), retTemp));
        User::Leave( retTemp );
        }
    CleanupClosePushL( iAtCmdExtCommon );
    retTemp = iAtCmdExtCommon.Connect( *iConnectionName );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ConstructL() common connect (%d)"), retTemp));
        User::Leave( retTemp );
        }
    // Create the array of special commands
    CreateSpecialCommandsL();
    // Create the plugin handlers
    CreatePluginHandlersL();
    // Create the listeners
    CDunAtEcomListen* ecomListen = CDunAtEcomListen::NewLC( &iAtCmdExt, this );
    CDunAtModeListen* modeListen = CDunAtModeListen::NewLC( &iAtCmdExtCommon,
                                                           this );
    CDunAtNvramListen* nvramListen = CDunAtNvramListen::NewLC( &iAtCmdExt,
                                                               &iAtCmdExtCommon );
    // Set the default modes (+report) and characters
    GetAndSetDefaultSettingsL();
    // Start listening
    ecomListen->IssueRequest();
    modeListen->IssueRequest();
    nvramListen->IssueRequest();
    CleanupStack::Pop( nvramListen );
    CleanupStack::Pop( modeListen );
    CleanupStack::Pop( ecomListen );
    CleanupStack::Pop( &iAtCmdExtCommon );
    CleanupStack::Pop( &iAtCmdExt );
    iEcomListen = ecomListen;
    iModeListen = modeListen;
    iNvramListen = nvramListen;
    
    iAtSpecialCmdHandler = CDunAtSpecialCmdHandler::NewL();
    FTRACE(FPrint( _L("CDunAtCmdHandler::ConstructL() complete") ));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::Initialize()
    {
    // Don't initialize iUpstream here (it is set through NewL)
    // Don't initialize iDownstream here (it is set through NewL)
    // Don't initialize iConnectionName here (it is set through NewL)
    iHandleState = EDunStateIdle;
    iCarriageReturn = 0;
    iLineFeed = 0;
    iBackspace = 0;
    iCommand = NULL;
    iDecodeInfo.iFirstDecode = ETrue;
    iDecodeInfo.iDecodeIndex = KErrNotFound;
    iDecodeInfo.iPrevChar = 0;
    iDecodeInfo.iPrevExists = EFalse;
    iCmdPusher = NULL;
    iEcomListen = NULL;
    iModeListen = NULL;
    iNvramListen = NULL;
    iDataMode = EFalse;
    iEchoOn = EFalse;
    iQuietOn = EFalse;
    iVerboseOn = EFalse;
    iEndIndex = KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Creates plugin handlers for this class
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::CreatePluginHandlersL()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::CreatePluginHandlersL()") ));
    if ( !iAtCmdExt.Handle() )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::CreatePluginHandlersL() complete") ));
        User::Leave( KErrGeneral );
        }
    // First create the command reply pusher
    CDunAtCmdPusher* cmdPusher = CDunAtCmdPusher::NewLC( &iAtCmdExt,
                                                         this,
                                                         iDownstream,
                                                         &iOkBuffer );
    // Next create the URC handlers
    TInt i;
    TInt numOfPlugins = iAtCmdExt.NumberOfPlugins();
    for ( i=0; i<numOfPlugins; i++ )
        {
        AddOneUrcHandlerL();
        }
    CleanupStack::Pop( cmdPusher );
    iCmdPusher = cmdPusher;
    FTRACE(FPrint( _L("CDunAtCmdHandler::CreatePluginHandlersL() complete") ));
    }

// ---------------------------------------------------------------------------
// Creates an array of special commands
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::CreateSpecialCommandsL()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::CreateSpecialCommandsL()") ));
    TInt retTemp = KErrNone;
    TBool firstSearch = ETrue;
    for ( ;; )
        {
        retTemp = iAtCmdExt.GetNextSpecialCommand( iInputBuffer, firstSearch );
        if ( retTemp != KErrNone )
            {
            break;
            }
        HBufC8* specialCmd = HBufC8::NewMaxLC( iInputBuffer.Length() );
        TPtr8 specialCmdPtr = specialCmd->Des();
        specialCmdPtr.Copy( iInputBuffer );
        specialCmdPtr.UpperCase();
        iSpecials.AppendL( specialCmd );
        CleanupStack::Pop( specialCmd );
        }
    iInputBuffer.Zero();
    FTRACE(FPrint( _L("CDunAtCmdHandler::CreateSpecialCommandsL() complete") ));
    }

// ---------------------------------------------------------------------------
// Recreates special command data.
// This is done when a plugin is installed or uninstalled.
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::RecreateSpecialCommands()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::RecreateSpecialCommands()") ));
    iSpecials.ResetAndDestroy();
    TRAPD( retTrap, CreateSpecialCommandsL() );
    FTRACE(FPrint( _L("CDunAtCmdHandler::RecreateSpecialCommands() complete") ));
    return retTrap;
    }

// ---------------------------------------------------------------------------
// Gets default settings from RATExtCommon and sets them to RATExt
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::GetAndSetDefaultSettingsL()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetAndSetDefaultSettingsL()") ));
    // Note: Let's assume command mode is off by default
    TUint modeSet = GetCurrentModeL( KModeEcho | KModeQuiet | KModeVerbose );
    iEchoOn    = ( modeSet & KEchoModeBase    ) ? ETrue : EFalse;
    iQuietOn   = ( modeSet & KQuietModeBase   ) ? ETrue : EFalse;
    iVerboseOn = ( modeSet & KVerboseModeBase ) ? ETrue : EFalse;
    iCarriageReturn = GetCurrentModeL( KModeCarriage );
    iLineFeed = GetCurrentModeL( KModeLineFeed );
    iBackspace = GetCurrentModeL( KModeBackspace );
    iAtCmdExt.ReportQuietModeChange( iQuietOn );
    iAtCmdExt.ReportVerboseModeChange( iVerboseOn );
    iAtCmdExt.ReportCharacterChange( ECharTypeCarriage, iCarriageReturn );
    iAtCmdExt.ReportCharacterChange( ECharTypeLineFeed, iLineFeed );
    iAtCmdExt.ReportCharacterChange( ECharTypeBackspace, iBackspace );
    RegenerateReplyStrings();
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetAndSetDefaultSettingsL() settings: E=%d, Q=%d, V=%d"), iEchoOn, iQuietOn, iVerboseOn ));
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetAndSetDefaultSettingsL() settings: CR=%u, LF=%u, BS=%u"), iCarriageReturn, iLineFeed, iBackspace ));
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetAndSetDefaultSettingsL() complete") ));
    }

// ---------------------------------------------------------------------------
// Regenerates the reply strings based on settings
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::RegenerateReplyStrings()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateReplyStrings()") ));
    TBool retVal = EFalse;
    retVal |= RegenerateOkReply();
    retVal |= RegenerateErrorReply();
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateReplyStrings() complete") ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Regenerates the ok reply based on settings
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::RegenerateOkReply()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateOkReply()") ));
    iOkBuffer.Zero();
    if ( iQuietOn )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateOkReply() (quiet) complete") ));
        return ETrue;
        }
    if ( iVerboseOn )
        {
        _LIT8( KVerboseOk, "OK" );
        iOkBuffer.Append( iCarriageReturn );
        iOkBuffer.Append( iLineFeed );
        iOkBuffer.Append( KVerboseOk );
        iOkBuffer.Append( iCarriageReturn );
        iOkBuffer.Append( iLineFeed );
        }
    else
        {
        _LIT8( KNumericOk, "0" );
        iOkBuffer.Append( KNumericOk );
        iOkBuffer.Append( iCarriageReturn );
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateOkReply() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Regenerates the error reply based on settings
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::RegenerateErrorReply()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateErrorReply()") ));
    iErrorBuffer.Zero();
    if ( iQuietOn )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateErrorReply() (quiet) complete") ));
        return ETrue;
        }
    if ( iVerboseOn )
        {
        _LIT8( KVerboseError, "ERROR" );
        iErrorBuffer.Append( iCarriageReturn );
        iErrorBuffer.Append( iLineFeed );
        iErrorBuffer.Append( KVerboseError );
        iErrorBuffer.Append( iCarriageReturn );
        iErrorBuffer.Append( iLineFeed );
        }
    else
        {
        _LIT8( KNumericError, "4" );
        iErrorBuffer.Append( KNumericError );
        iErrorBuffer.Append( iCarriageReturn );
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::RegenerateErrorReply() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Gets current mode
// ---------------------------------------------------------------------------
//
TUint CDunAtCmdHandler::GetCurrentModeL( TUint aMask )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetCurrentModeL()") ));
    TUint maskCheck = aMask & ( ~KSupportedModes );
    if ( maskCheck != 0 )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::GetCurrentModeL() (not supported) complete") ));
        User::Leave( KErrNotSupported );
        }
    TUint newMode = 0;
    TInt retTemp = iAtCmdExtCommon.GetMode( aMask, newMode );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::GetCurrentModeL() (ERROR) complete") ));
        User::Leave( retTemp );
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::GetCurrentModeL() complete") ));
    return newMode & (KModeChanged-1);
    }

// ---------------------------------------------------------------------------
// Instantiates one URC message handling class instance and adds it to the URC
// message handler array
// ---------------------------------------------------------------------------
//
CDunAtUrcHandler* CDunAtCmdHandler::AddOneUrcHandlerL()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddOneUrcHandlerL()") ));
    CDunAtUrcHandler* urcHandler = CDunAtUrcHandler::NewLC( &iAtCmdExt,
                                                            iDownstream );
    iUrcHandlers.AppendL( urcHandler );
    CleanupStack::Pop( urcHandler );
    FTRACE(FPrint( _L("CDunAtCmdHandler::AddOneUrcHandlerL() complete") ));
    return urcHandler;
    }

// ---------------------------------------------------------------------------
// Deletes all instantiated URC message handlers
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::DeletePluginHandlers()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::DeletePluginHandlers()") ));
    delete iCmdPusher;
    iCmdPusher = NULL;
    TInt i;
    TInt count = iUrcHandlers.Count();
    for ( i=0; i<count; i++ )
        {
        delete iUrcHandlers[i];
        iUrcHandlers[i] = NULL;
        }
    iUrcHandlers.Reset();
    iUrcHandlers.Close();
    FTRACE(FPrint( _L("CDunAtCmdHandler::DeletePluginHandlers() complete") ));
    }

// ---------------------------------------------------------------------------
// Manages partial AT command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ManagePartialCommand( TDesC8& aCommand,
                                              TBool& aNeedsCarriage )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand()") ));
    aNeedsCarriage = ETrue;
    // Check length of command
    if ( aCommand.Length() == 0 )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() (length zero) complete") ));
        return ETrue;
        }
    // Check one character (or unit) based input data
    if ( aCommand.Length() == KDunChSetMaxCharLen )
        {
        EchoCommand( aCommand );
        // Handle backspace and cancel characters
        TBool found = HandleSpecialCharacters( aCommand );
        if ( found )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() (special) complete") ));
            return ETrue;
            }
        }
    TBool endFound = EFalse;
    TBool overflow = AppendCommandToInputBuffer( aCommand, endFound );
    if ( overflow )
        {
        // Overflow occurred so return ETrue (consumed) to skip all the rest
        // characters until carriage return is found
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() (overflow) complete") ));
        return ETrue;
        }
    // If something went wrong, do nothing (return consumed)
    if ( iInputBuffer.Length() <= 0 )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() (length) complete") ));
        return ETrue;
        }
    // If "A/", no carriage return is needed, check that now
    if ( IsASlashCommand() )
        {
        aNeedsCarriage = EFalse;
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() (A/) complete") ));
        return EFalse;
        }
    // For other commands and if no end, just return with consumed
    if ( !endFound )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() (void) complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManagePartialCommand() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Echoes a command if echo is on
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::EchoCommand( TDesC8& aDes )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::EchoCommand()") ));
    if ( aDes.Length() > KDunChSetMaxCharLen )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::EchoCommand() (wrong length) complete") ));
        return EFalse;
        }
    if ( iEchoOn )
        {
        iEchoBuffer.Copy( aDes );
        iDownstream->NotifyDataPushRequest( &iEchoBuffer, NULL );
        FTRACE(FPrint( _L("CDunAtCmdHandler::EchoCommand() complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::EchoCommand() (not started) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles backspace and cancel characters
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::HandleSpecialCharacters( TDesC8& aCommand )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialCharacters()") ));
    if ( aCommand.Length() != KDunChSetMaxCharLen )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialCharacters() (wrong length) complete") ));
        return EFalse;
        }
    if ( aCommand[0] == iBackspace )
        {
        TInt bufferLength = iInputBuffer.Length();
        if ( bufferLength > 0 )
            {
            iInputBuffer.SetLength( bufferLength-1 );
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialCharacters() (backspace) complete") ));
        return ETrue;
        }
    if ( aCommand[0] == KDunCancel )
        {
        ResetParseBuffers();  // More processing here?
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialCharacters() (cancel) complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleSpecialCharacters() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Appends command to parse buffer
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::AppendCommandToInputBuffer( TDesC8& aCommand,
                                                    TBool& aEndFound )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::AppendCommandToInputBuffer()") ));
    aEndFound = EFalse;
    TInt cmdBufIndex = 0;
    TInt cmdBufLim = aCommand.Length();
    while ( cmdBufIndex < cmdBufLim )
        {
        if ( iInputBuffer.Length() == iInputBuffer.MaxLength() )
            {
            // 1) If output is full and end found from input
            //    -> reset buffers and overflow found
            // 2) If output is full and end not found from input
            //    -> don't reset buffers and overflow found
            TInt foundIndex = FindEndOfCommand( aCommand );
            if ( foundIndex >= 0 )
                {
                aEndFound = ETrue;
                ResetParseBuffers();
                FTRACE(FPrint( _L("CDunAtCmdHandler::AppendCommandToInputBuffer() (reset) complete") ));
                }
            FTRACE(FPrint( _L("CDunAtCmdHandler::AppendCommandToInputBuffer() (overflow) complete") ));
            return ETrue;
            }
        TChar character = aCommand[cmdBufIndex];
        if ( IsEndOfCommand(character) )
            {
            aEndFound = ETrue;
            iEndIndex = cmdBufIndex;
            break;
            }
        iInputBuffer.Append( aCommand[cmdBufIndex] );
        cmdBufIndex++;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::AppendCommandToInputBuffer() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles next decoded command from input buffer
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::HandleNextDecodedCommand()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleNextDecodedCommand()") ));
    if ( iHandleState != EDunStateAtCmdHandling )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleNextDecodedCommand() (not ready) complete") ));
        return ETrue;
        }
    TBool extracted = ExtractNextDecodedCommand();
    if ( !extracted )
        {
        ManageEndOfCmdHandling( ETrue, ETrue, ETrue );
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleNextDecodedCommand() (last) complete") ));
        return ETrue;
        }
    // Next convert the decoded AT command to uppercase
    // Don't check for case status -> let mixed cases pass
    iParseInfo.iSendBuffer.Copy( iDecodeInfo.iDecodeBuffer );
    TInt maxLength = iParseInfo.iSendBuffer.MaxLength();
    TPtr8 upperDes( &iParseInfo.iSendBuffer[0], iParseInfo.iLimit, maxLength );
    upperDes.UpperCase();
    // Next always send the command to ATEXT
    iCmdPusher->IssueRequest( iParseInfo.iSendBuffer );
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleNextDecodedCommand() complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Manages end of AT command handling
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::ManageEndOfCmdHandling( TBool aNotifyExternal,
                                               TBool aNotifyLocal,
                                               TBool aClearInput )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEndOfCmdHandling()") ));
    if ( iInputBuffer.Length() > 0 )
        {
        iLastBuffer.Copy( iInputBuffer );
        }
    ResetParseBuffers( aClearInput );
    iHandleState = EDunStateIdle;
    if ( aNotifyLocal )
        {
        iCmdPusher->SetEndOfCmdLine();
        }
    if ( !aNotifyExternal )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEndOfCmdHandling() (no external) complete") ));
        return;
        }
    // Note: here we need to avoid internal recursion when parsing the
    // multiple IsEndOfCommand() and IsDelimiterCharacter() markers inside the
    // same upstream block.
    // Skip all the extra markers except the one we already know to exist.
    TInt i;
    TInt startVal = iEndIndex + 1;
    TInt foundIndex = KErrNotFound;
    TInt count = iCommand->Length();
    for ( i=startVal; i<count; i++ )
        {
        TChar character = (*iCommand)[i];
        if ( !(IsEndOfCommand(character)||IsDelimiterCharacter(character)) )
            {
            foundIndex = i;
            break;
            }
        }
    iUpstream->NotifyAtCmdHandlingEnd( foundIndex );
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEndOfCmdHandling() complete") ));
    }

// ---------------------------------------------------------------------------
// Extracts next decoded command from input buffer to decode buffer
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ExtractNextDecodedCommand( TBool aPeek )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractNextDecodedCommand()") ));
    iParseInfo.iLimit = KErrNotFound;
    TDunDecodeInfo oldInfo = iDecodeInfo;
    iDecodeInfo.iDecodeBuffer.Zero();
    // Find start of decode command from input buffer
    TBool extendedCmd = EFalse;
    TInt startIndex = iDecodeInfo.iDecodeIndex;
    startIndex = FindStartOfDecodedCommand( iInputBuffer,
                                            startIndex,
                                            extendedCmd );
    if ( startIndex < 0 )
        {
        RestoreOldDecodeInfo( aPeek, oldInfo );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractNextDecodedCommand() (no start) complete") ));
        return EFalse;
        }
    // Find end of decode command from input buffer
    TBool extendedEnd = EFalse;
    TBool oneCharCmd = EFalse;
    TBool specialCmd = EFalse;
    TInt endIndex = KErrNotFound;
    if ( extendedCmd )
        {
        if( iAtSpecialCmdHandler->IsCompleteSubCommand(iInputBuffer, startIndex, endIndex) == EFalse )
            {
            extendedEnd = CheckExtendedCommand( startIndex, endIndex );
            }
        }
    else
        {
        specialCmd = CheckSpecialCommand( startIndex, endIndex );
        if ( !specialCmd )
            {
            CheckBasicCommand( startIndex, endIndex, oneCharCmd );
            }
        }
    if ( endIndex < startIndex )
        {
        RestoreOldDecodeInfo( aPeek, oldInfo );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractNextDecodedCommand() (no end) complete") ));
        return EFalse;
        }
    TInt cmdLength = endIndex - startIndex + 1;
    // If the limit was not already set then do it now
    if ( iParseInfo.iLimit < 0 )
        {
        iParseInfo.iLimit = cmdLength;
        }
    // Next create a new command
    if ( !iDecodeInfo.iFirstDecode && !oneCharCmd && !specialCmd )
        {
        _LIT( KAtMsg, "AT" );
        iDecodeInfo.iDecodeBuffer.Append( KAtMsg );
        iParseInfo.iLimit += 2;  // Length of "AT"
        // Note: The length of iDecodeBuffer is not exceeded here because "AT"
        // is added only for the second commands after that.
        }
    TPtrC8 decodedCmd = iInputBuffer.Mid( startIndex, cmdLength );
    iDecodeInfo.iDecodeBuffer.Append( decodedCmd );
    // Set index for next round
    iDecodeInfo.iFirstDecode = EFalse;
    iDecodeInfo.iDecodeIndex = endIndex + 1;
    if ( extendedEnd )  // skip the extra ';'
        {
        iDecodeInfo.iDecodeIndex++;
        }
    RestoreOldDecodeInfo( aPeek, oldInfo );
    FTRACE(FPrint( _L("CDunAtCmdHandler::ExtractNextDecodedCommand() complete") ));
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Restores old decode info. For ExtractNextDecodedCommand() when aPeeks is
// ETrue.
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::RestoreOldDecodeInfo( TBool aPeek,
                                             TDunDecodeInfo& aOldInfo )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::RestoreOldDecodeInfo()") ));
    if ( aPeek )
        {
        iDecodeInfo = aOldInfo;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::RestoreOldDecodeInfo() complete") ));
    }

// ---------------------------------------------------------------------------
// Finds end of an AT command
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::FindEndOfCommand( TDesC8& aDes, TInt aStartIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindEndOfCommand()") ));
    TInt i;
    TInt length = aDes.Length();
    for ( i=aStartIndex; i<length; i++ )
        {
        TChar character = aDes[i];
        if ( IsEndOfCommand(character) )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::FindEndOfCommand() complete (%d)"), i ));
            return i;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindEndOfCommand() (not found) complete") ));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Tests for end of AT command character
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsEndOfCommand( TChar& aCharacter )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsEndOfCommand()") ));
    if ( aCharacter==iCarriageReturn || aCharacter==iLineFeed )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsEndOfCommand() (found) complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsEndOfCommand() (not found) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Finds start of a decoded AT command
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::FindStartOfDecodedCommand( TDesC8& aDes,
                                                  TInt aStartIndex,
                                                  TBool& aExtended )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindStartOfDecodedCommand()") ));
    aExtended = EFalse;
    TInt i;
    TInt count = aDes.Length();
    for ( i=aStartIndex; i<count; i++ )
        {
        TChar character = aDes[i];
        if ( IsDelimiterCharacter(character) && !IsOneCharacterCommand(i) )
            {
            continue;
            }
        if ( !iDecodeInfo.iFirstDecode && IsExtendedCharacter(character) )
            {
            aExtended = ETrue;
            }
        else if ( iDecodeInfo.iFirstDecode && i+2<count )
            {
            // i+2 is the position of '+' in "AT+" and other similar sets
            character = aDes[i+2];
            if ( IsExtendedCharacter(character) )
                {
                aExtended = ETrue;
                }
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::FindStartOfDecodedCommand() complete (%d)"), i ));
        return i;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::FindStartOfDecodedCommand() (not found) complete") ));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Checks if character is delimiter character
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsDelimiterCharacter( TChar aCharacter )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsDelimiterCharacter()") ));
    if ( aCharacter.IsSpace() || aCharacter==';' || aCharacter==0x00 )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsDelimiterCharacter() complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsDelimiterCharacter() (not delimiter) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Checks if character is of extended group
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsExtendedCharacter( TChar aCharacter )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsExtendedCharacter()") ));
    if ( aCharacter=='+'  || aCharacter=='&' || aCharacter=='%' ||
         aCharacter=='\\' || aCharacter=='*' || aCharacter=='#' ||
         aCharacter=='$'  || aCharacter=='^' )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsExtendedCharacter() complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsExtendedCharacter() (not extended) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Checks extended command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::CheckExtendedCommand( TInt aStartIndex, TInt& aEndIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::CheckExtendedCommand()") ));
    iDecodeInfo.iPrevExists = EFalse;
    TBool inQuotes = EFalse;
    TBool endFound = EFalse;
    TInt length = iInputBuffer.Length();
    for ( aEndIndex=aStartIndex; aEndIndex<length; aEndIndex++ )
        {
        TChar character = iInputBuffer[aEndIndex];
        if ( character == '"' )
            {
            if ( iDecodeInfo.iPrevExists && iParseInfo.iLimit<0 )
                {
                iParseInfo.iLimit = aEndIndex - aStartIndex;
                }
            inQuotes ^= ETrue;  // EFalse to ETrue or ETrue to EFalse
            iDecodeInfo.iPrevExists = ETrue;
            iDecodeInfo.iPrevChar = character;
            continue;
            }
        if ( inQuotes )
            {
            continue;
            }
        // The next ones are those that are not in quotes
        if ( character == '=' && iParseInfo.iLimit<0 )
            {
            iParseInfo.iLimit = aEndIndex - aStartIndex;
            }
        if ( IsDelimiterCharacter(character) )
            {
            endFound = ETrue;
            break;
            }
        if( IsExtendedCharacter(character) && (aEndIndex != aStartIndex) && iDecodeInfo.iPrevExists )
            {
            if( iDecodeInfo.iPrevChar.IsAlphaDigit() )
                {
                aEndIndex--;
                // End found but return EFalse in order to calling function can proceed correct way,
                // no extended end.
                return EFalse;
                }
            }
        iDecodeInfo.iPrevExists = ETrue;
        iDecodeInfo.iPrevChar = character;
        }
    aEndIndex--;
    FTRACE(FPrint( _L("CDunAtCmdHandler::CheckExtendedCommand() complete") ));
    return endFound;
    }

// ---------------------------------------------------------------------------
// Checks special command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::CheckSpecialCommand( TInt aStartIndex,
                                             TInt& aEndIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::CheckSpecialCommand()") ));
    TBuf8<KDunInputBufLength> upperBuf;
    TInt newLength = iInputBuffer.Length() - aStartIndex;
    upperBuf.Copy( &iInputBuffer[aStartIndex], newLength );
    upperBuf.UpperCase();
    TInt i;
    TInt count = iSpecials.Count();
    for ( i=0; i<count; i++ )
        {
        HBufC8* specialCmd = iSpecials[i];
        TInt specialLength = specialCmd->Length();
        if ( newLength < specialLength )
            {
            continue;
            }
        TInt origLength = newLength;
        if ( newLength > specialLength )
            {
            upperBuf.SetLength( specialLength );
            }
        TInt cmpResult = upperBuf.Compare( *specialCmd );
        upperBuf.SetLength( origLength );
        if ( cmpResult == 0 )
            {
            iParseInfo.iLimit = specialLength;
            aEndIndex = (origLength-1) + aStartIndex;
            FTRACE(FPrint( _L("CDunAtCmdHandler::CheckSpecialCommand() complete") ));
            return ETrue;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::CheckSpecialCommand() (not found) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Checks extended command
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::CheckBasicCommand( TInt aStartIndex,
                                          TInt& aEndIndex,
                                          TBool& aOneCharCmd )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::CheckBasicCommand()") ));
    aEndIndex = aStartIndex;
    aOneCharCmd = EFalse;
    TBool inQuotes = EFalse;
    TInt length = iInputBuffer.Length();
    iDecodeInfo.iPrevExists = EFalse;
    if ( aStartIndex < length )
        {
        TChar character = iInputBuffer[aStartIndex];
        if ( IsOneCharacterCommand(aStartIndex) )
            {
            aOneCharCmd = ETrue;
            // Without "endIndex++" here
            FTRACE(FPrint( _L("CDunAtCmdHandler::CheckBasicCommand() (X) complete") ));
            return KErrNone;
            }
        }
    for ( ; aEndIndex<length; aEndIndex++ )
        {
        TChar character = iInputBuffer[aEndIndex];
        if ( character == '"' )
            {
            if ( iDecodeInfo.iPrevExists && iParseInfo.iLimit<0 )
                {
                iParseInfo.iLimit = aEndIndex - aStartIndex;
                }
            inQuotes ^= ETrue;  // EFalse to ETrue or ETrue to EFalse
            iDecodeInfo.iPrevExists = ETrue;
            iDecodeInfo.iPrevChar = character;
            continue;
            }
        if ( inQuotes )
            {
            continue;
            }
        if ( character == '?' )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::CheckBasicCommand() (?) complete") ));
            return KErrNone;
            }
        if ( character.IsSpace() || IsExtendedCharacter(character) )
            {
            aEndIndex--;
            FTRACE(FPrint( _L("CDunAtCmdHandler::CheckBasicCommand() (S or extended) complete") ));
            return KErrNone;
            }
        if ( !iDecodeInfo.iPrevExists )
            {
            iDecodeInfo.iPrevExists = ETrue;
            iDecodeInfo.iPrevChar = character;
            continue;
            }
        if ( IsOneCharacterCommand(aEndIndex) )
            {
            aEndIndex--;
            FTRACE(FPrint( _L("CDunAtCmdHandler::CheckBasicCommand() (one char) complete") ));
            return KErrNone;
            }
        if ( character.IsAlpha() && !iDecodeInfo.iPrevChar.IsAlpha() )
            {
            aEndIndex--;
            FTRACE(FPrint( _L("CDunAtCmdHandler::CheckBasicCommand() (boundary) complete") ));
            return KErrNone;
            }
        iDecodeInfo.iPrevExists = ETrue;
        iDecodeInfo.iPrevChar = character;
        }
    aEndIndex--;
    FTRACE(FPrint( _L("CDunAtCmdHandler::CheckBasicCommand() (not found) complete") ));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Check if any one character command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsOneCharacterCommand( TInt aIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsOneCharacterCommand()") ));
    if ( aIndex<0 || aIndex>=iInputBuffer.Length() )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsOneCharacterCommand() (index out of bounds) complete") ));
        return EFalse;
        }
    TChar character = iInputBuffer[aIndex];
    if ( character==',' || character==';' || character=='@'  || character=='!' ||
         IsOneCharacterACommand(aIndex) )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsOneCharacterCommand() complete") ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsOneCharacterCommand() (not found) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Check if one character "A" command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsOneCharacterACommand( TInt aIndex )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsOneCharacterACommand()") ));
    if ( aIndex<0 || aIndex>=iInputBuffer.Length() )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsOneCharacterACommand() (index out of bounds) complete") ));
        return EFalse;
        }
    if ( iInputBuffer[aIndex]!='a' && iInputBuffer[aIndex]!='A' )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsOneCharacterACommand() (not found) complete") ));
        return EFalse;
        }
    TBool prevAlpha = EFalse;
    TBool nextAlpha = EFalse;
    TBool nextSlash = EFalse;
    TInt length = iInputBuffer.Length();
    if ( iDecodeInfo.iPrevExists && iDecodeInfo.iPrevChar.IsAlpha() )
        {
        prevAlpha = ETrue;
        }
    if ( aIndex+1 < length )
        {
        TChar nextChar = iInputBuffer[aIndex+1];
        if ( nextChar.IsAlpha() )
            {
            nextAlpha = ETrue;
            }
        if ( nextChar == '/' )
            {
            nextSlash = ETrue;
            }
        }
    if ( prevAlpha || nextAlpha || nextSlash )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::IsOneCharacterACommand() (not found) complete") ));
        return EFalse;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsOneCharacterACommand() (complete) complete") ));
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Check if "A/" command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::IsASlashCommand()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsASlashCommand()") ));
    if ( iInputBuffer.Length() == 2 )
        {
        if ( iInputBuffer[1] == '/' &&
            (iInputBuffer[0] == 'A' || iInputBuffer[0] == 'a') )
            {
            FTRACE(FPrint( _L("CDunAtCmdHandler::IsASlashCommand() (found) complete") ));
            return ETrue;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::IsASlashCommand() (not found) complete") ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles "A/" command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::HandleASlashCommand()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleASlashCommand()") ));
    // If not "A/" command, return
    if ( !IsASlashCommand() )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleASlashCommand() (no push) complete") ));
        return EFalse;
        }
    // If "A/" command and last buffer exist, set the last buffer as the current buffer
    if ( iLastBuffer.Length() > 0 )
        {
        iInputBuffer.Copy( iLastBuffer );
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleASlashCommand() (copy) complete") ));
        return EFalse;
        }
    // Last buffer not set so return "ERROR" if quiet mode not on
    if ( iQuietOn )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::HandleASlashCommand() (quiet) complete") ));
        return EFalse;
        }
    iDownstream->NotifyDataPushRequest( &iErrorBuffer, NULL );
    FTRACE(FPrint( _L("CDunAtCmdHandler::HandleASlashCommand() complete") ));
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Resets parse buffers
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::ResetParseBuffers( TBool aClearInput )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ResetParseBuffers()") ));
    if ( aClearInput )
        {
        iInputBuffer.Zero();
        }
    iDecodeInfo.iFirstDecode = ETrue;
    iDecodeInfo.iDecodeIndex = 0;
    iDecodeInfo.iPrevExists = EFalse;
    iDecodeInfo.iDecodeBuffer.Zero();
    FTRACE(FPrint( _L("CDunAtCmdHandler::ResetParseBuffers() complete") ));
    }

// ---------------------------------------------------------------------------
// Manages command mode change
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ManageCommandModeChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCommandModeChange()" ) ));
    if ( aMode & KCommandModeChanged )
        {
        if ( aMode & KModeCommand )  // command mode ON
            {
            ReportCommandModeChange( ETrue );
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCommandModeChange() command mode changed ON" ) ));
            }
        else  // command mode OFF
            {
            ReportCommandModeChange( EFalse );
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCommandModeChange() command mode changed OFF" ) ));
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCommandModeChange() (change) complete" ) ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCommandModeChange()" ) ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Reports command mode start/end change
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::ReportCommandModeChange( TBool aStart )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ReportCommandModeChange()" ) ));
    TInt i;
    TInt count = iCmdCallbacks.Count();
    if ( aStart )
        {
        if ( iDataMode )
            {
            for ( i=0; i<count; i++ )
                {
                iCmdCallbacks[i]->NotifyCommandModeStart();
                }
            iDataMode = EFalse;
            }
        }
    else  // end
        {
        if ( !iDataMode )
            {
            for ( i=0; i<count; i++ )
                {
                iCmdCallbacks[i]->NotifyCommandModeEnd();
                }
            iDataMode = ETrue;
            }
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ReportCommandModeChange() complete" ) ));
    }

// ---------------------------------------------------------------------------
// Manages echo mode change
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ManageEchoModeChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange()" ) ));
    if ( aMode & KEchoModeChanged )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() checking echo mode..." ) ));
        if ( aMode & KModeEcho )  // echo mode ON
            {
            iEchoOn = ETrue;
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() echo mode changed ON" ) ));
            }
        else  // echo mode OFF
            {
            iEchoOn = EFalse;
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() echo mode changed OFF" ) ));
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() (change) complete" ) ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() complete" ) ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Manages quiet mode change
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ManageQuietModeChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageQuietModeChange()" ) ));
    if ( aMode & KQuietModeChanged )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageEchoModeChange() checking quiet mode..." ) ));
        if ( aMode & KModeQuiet )  // quiet mode ON
            {
            iAtCmdExt.ReportQuietModeChange( ETrue );
            iQuietOn = ETrue;
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageQuietModeChange() quiet mode changed ON" ) ));
            }
        else  // quiet mode OFF
            {
            iAtCmdExt.ReportQuietModeChange( EFalse );
            iQuietOn = EFalse;
            FTRACE(FPrint( _L("CDunAtCmdHandler::ManageQuietModeChange() quiet mode changed OFF" ) ));
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageQuietModeChange() (change) complete" ) ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageQuietModeChange() complete" ) ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Manages quiet mode change
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::ManageVerboseModeChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageVerboseModeChange()" ) ));
    if ( aMode & KVerboseModeChanged )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageVerboseModeChange() checking verbose mode..." ) ));
        if ( aMode & KModeVerbose )  // verbose mode ON
            {
            iAtCmdExt.ReportVerboseModeChange( ETrue );
            iVerboseOn = ETrue;
            FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyVerboseStatusChange() verbose mode changed ON" ) ));
            }
        else  // verbose mode OFF
            {
            iAtCmdExt.ReportVerboseModeChange( EFalse );
            iVerboseOn = EFalse;
            FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyVerboseStatusChange() verbose mode changed OFF" ) ));
            }
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageVerboseModeChange() (change) complete" ) ));
        return ETrue;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageVerboseModeChange() complete" ) ));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Manages character change
// ---------------------------------------------------------------------------
//
void CDunAtCmdHandler::ManageCharacterChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCharacterChange()" ) ));
    if ( aMode & KCarriageChanged )
        {
        iCarriageReturn = aMode & (KModeChanged-1);
        iAtCmdExt.ReportCharacterChange( ECharTypeCarriage, iCarriageReturn );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCharacterChange() carriage return changed" ) ));
        }
    else if ( aMode & KLineFeedChanged )
        {
        iLineFeed = aMode & (KModeChanged-1);
        iAtCmdExt.ReportCharacterChange( ECharTypeLineFeed, iLineFeed );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCharacterChange() line feed changed" ) ));
        }
    else if ( aMode & KBackspaceChanged )
        {
        iBackspace = aMode & (KModeChanged-1);
        iAtCmdExt.ReportCharacterChange( ECharTypeBackspace, iBackspace );
        FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCharacterChange() backspace changed" ) ));
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::ManageCharacterChange() complete" ) ));
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdPusher.
// Notifies about end of AT command processing. This is after all reply data
// for an AT command is multiplexed to the downstream.
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::NotifyEndOfProcessing( TInt /*aError*/ )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEndOfProcessing()" ) ));
    HandleNextDecodedCommand();
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEndOfProcessing() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdPusher.
// Notifies about request to stop AT command handling for the rest of the
// command line data
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::NotifyEndOfCmdLineProcessing()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEndOfCmdLineProcessing()" ) ));
    TInt retVal = Stop();
    ManageEndOfCmdHandling( ETrue, EFalse, ETrue );
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyEndOfCmdLineProcessing() complete" ) ));
    return retVal;
    }

// ---------------------------------------------------------------------------
// From class MDunAtCmdPusher.
// Notifies about request to peek for the next command
// ---------------------------------------------------------------------------
//
TBool CDunAtCmdHandler::NotifyNextCommandPeekRequest()
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyNextCommandPeekRequest()") ));
    TBool extracted = ExtractNextDecodedCommand( ETrue );
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyNextCommandPeekRequest() complete") ));
    return extracted;
    }

// ---------------------------------------------------------------------------
// From class MDunAtEcomListen.
// Notifies about new plugin installation
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::NotifyPluginInstallation( TUid& /*aPluginUid*/ )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginInstallation()" ) ));
    CDunAtUrcHandler* urcHandler = NULL;
    TRAPD( retTrap, urcHandler=AddOneUrcHandlerL() );
    if ( retTrap != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginInstallation() (trapped!) complete" ) ));
        return retTrap;
        }
    TInt retTemp = urcHandler->IssueRequest();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginInstallation() (issuerequest) complete" ) ));
        return retTemp;
        }
    TUid ownerUid = urcHandler->OwnerUid();
    iAtCmdExt.ReportListenerUpdateReady( ownerUid, EEcomTypeInstall );
    // As a last step recreate the special command data
    retTemp = RecreateSpecialCommands();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginInstallation() (recreate) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginInstallation() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunAtEcomListen.
// Notifies about existing plugin uninstallation
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::NotifyPluginUninstallation( TUid& aPluginUid )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginUninstallation()" ) ));
    TInt i;
    TInt count = iUrcHandlers.Count();
    for ( i=count-1; i>=0; i-- )
        {
        TUid ownerUid = iUrcHandlers[i]->OwnerUid();
        if ( ownerUid == aPluginUid )
            {
            delete iUrcHandlers[i];
            iUrcHandlers.Remove( i );
            iAtCmdExt.ReportListenerUpdateReady( ownerUid,
                                                 EEcomTypeUninstall );
            }
        }
    // As a last step recreate the special command data
    TInt retTemp = RecreateSpecialCommands();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginUninstallation() (recreate) complete" ) ));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyPluginUninstallation() complete" ) ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MDunAtModeListen.
// Gets called on mode status change
// ---------------------------------------------------------------------------
//
TInt CDunAtCmdHandler::NotifyModeStatusChange( TUint aMode )
    {
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange()") ));
    TBool commandModeSet = ManageCommandModeChange( aMode );
    TBool echoModeSet = ManageEchoModeChange( aMode );
    TBool quietModeSet = ManageQuietModeChange( aMode );
    TBool verboseModeSet = ManageVerboseModeChange( aMode );
    if ( quietModeSet || verboseModeSet )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() new settings: E=%d, Q=%d, V=%d"), iEchoOn, iQuietOn, iVerboseOn ));
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() (regenerate) mode set" ) ));
        RegenerateReplyStrings();
        return KErrNone;
        }
    // Keep the following after "quietModeSet || verboseModeSet" in order to
    // regenerate the reply also if two modes change at the same time
    if ( commandModeSet || echoModeSet )
        {
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() new settings: E=%d, Q=%d, V=%d"), iEchoOn, iQuietOn, iVerboseOn ));
        FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() mode set" ) ));
        return KErrNone;
        }
    ManageCharacterChange( aMode );
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() new settings: CR=%u, LF=%u, BS=%u"), iCarriageReturn, iLineFeed, iBackspace ));
    RegenerateReplyStrings();
    FTRACE(FPrint( _L("CDunAtCmdHandler::NotifyModeStatusChange() complete") ));
    return KErrNone;
    }
