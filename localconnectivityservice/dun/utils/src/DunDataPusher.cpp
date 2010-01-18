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
* Description:  Pushes data to existing stream from outside
*
*/


#include "DunDataPusher.h"
#include "DunDownstream.h"
#include "DunDebug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CDunDataPusher* CDunDataPusher::NewL( CDunDownstream& aParent,
                                      MDunCompletionReporter* aStreamCallback )
    {
    CDunDataPusher* self = new (ELeave) CDunDataPusher( aParent,
                                                        aStreamCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CDunDataPusher::~CDunDataPusher()
    {
    FTRACE(FPrint( _L("CDunDataPusher::~CDunDataPusher()" )));
    ResetData();
    FTRACE(FPrint( _L("CDunDataPusher::~CDunDataPusher() complete" )));
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CDunDataPusher::ResetData()
    {
    // APIs affecting this:
    // SendQueuedData()
    Stop();
    // AddToEventQueue()
    iEventQueue.Close();
    // Internal
    Initialize();
    }

// ---------------------------------------------------------------------------
// Sets media to be used for this endpoint
// ---------------------------------------------------------------------------
//
TInt CDunDataPusher::SetMedia( RComm* aComm )
    {
    FTRACE(FPrint( _L("CDunDataPusher::SetMedia() (RComm)" )));
    if ( !aComm )
        {
        FTRACE(FPrint( _L("CDunDataPusher::SetMedia() (RComm) (not initialized!) complete" )));
        return KErrGeneral;
        }
    iComm = aComm;
    FTRACE(FPrint( _L("CDunDataPusher::SetMedia() (RComm) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Sets media to be used for this endpoint
// ---------------------------------------------------------------------------
//
TInt CDunDataPusher::SetMedia( RSocket* aSocket )
    {
    FTRACE(FPrint( _L("CDunDataPusher::SetMedia() (RSocket)" )));
    if ( !aSocket )
        {
        FTRACE(FPrint( _L("CDunDataPusher::SetMedia() (RSocket) (not initialized!) complete" )));
        return KErrGeneral;
        }
    iSocket = aSocket;
    FTRACE(FPrint( _L("CDunDataPusher::SetMedia() (RSocket) complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Adds event notification to queue
// ---------------------------------------------------------------------------
//
TInt CDunDataPusher::AddToEventQueue( const TDesC8 *aPushedData,
                                      MDunCompletionReporter* aCallback )
    {
    FTRACE(FPrint( _L("CDunDataPusher::AddToQueue()" )));
    if ( !aPushedData || aPushedData->Length()<0 )
        {
        FTRACE(FPrint( _L("CDunDataPusher::AddToQueue() (unknown data) complete" )));
        return KErrGeneral;
        }
    // Check if identical pointer to data already exists
    TInt foundIndex = FindEventFromQueue( aPushedData );
    if ( foundIndex >= 0 )
        {
        FTRACE(FPrint( _L("CDunDataPusher::AddToQueue() (already exists) complete" )));
        return KErrAlreadyExists;
        }
    // Unique pointer -> add to event queue
    TDunDataPush dataPush;
    dataPush.iPushedData = aPushedData;
    dataPush.iCallback = aCallback;
    TInt retTemp = iEventQueue.Append( dataPush );
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunDataPusher::AddToQueue() (append failed!) complete" )));
        return retTemp;
        }
    FTRACE(FPrint( _L("CDunDataPusher::AddToQueue() complete (%d)" ), iEventQueue.Count() ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Finds an event from queue
// ---------------------------------------------------------------------------
//
TInt CDunDataPusher::FindEventFromQueue( const TDesC8 *aPushedData )
    {
    FTRACE(FPrint( _L("CDunDataPusher::FindEventFromQueue()" )));
    TInt i;
    TInt count = iEventQueue.Count();
    for ( i=0; i<count; i++ )
        {
        if ( iEventQueue[i].iPushedData == aPushedData )
            {
            FTRACE(FPrint( _L("CDunDataPusher::FindEventFromQueue() complete" )));
            return i;
            }
        }
    FTRACE(FPrint( _L("CDunDataPusher::FindEventFromQueue() (not found) complete" )));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Stops one event in the event queue
// ---------------------------------------------------------------------------
//
TInt CDunDataPusher::StopOneEvent( const TDesC8 *aPushedData )
    {
    FTRACE(FPrint( _L("CDunDataPusher::StopOneEvent()" )));
    if ( !aPushedData )
        {
        FTRACE(FPrint( _L("CDunDataPusher::StopOneEvent() (unknown data) complete" )));
        return KErrGeneral;
        }
    TInt foundIndex = FindEventFromQueue( aPushedData );
    if ( foundIndex >= 0 )
        {
        if ( iEventIndex == foundIndex )
            {
            Stop();
            }
        FTRACE(FPrint( _L("CDunDataPusher::StopOneEvent() complete" )));
        return KErrNone;
        }
    FTRACE(FPrint( _L("CDunDataPusher::StopOneEvent() (not found) complete" )));
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Sends queued data in round robin
// ---------------------------------------------------------------------------
//
TBool CDunDataPusher::SendQueuedData()
    {
    FTRACE(FPrint( _L("CDunDataPusher::SendQueuedData()" )));
    if ( iPushState!=EDunStateIdle || iEventQueue.Count()==0 )
        {
        FTRACE(FPrint( _L("CDunDataPusher::SendQueuedData() (not ready) complete" )));
        return EFalse;
        }
    TInt retTemp = ManageOneEvent();
    if ( retTemp != KErrNone )
        {
        FTRACE(FPrint( _L("CDunDataPusher::SendQueuedData() (ERROR) complete" )));
        return EFalse;
        }
    iPushState = EDunStateDataPushing;
    FTRACE(FPrint( _L("CDunDataPusher::SendQueuedData() complete (%d)" ), iEventQueue.Count() ));
    return ETrue;
    }

// ---------------------------------------------------------------------------
// Stops sending for write endpoint
// ---------------------------------------------------------------------------
//
TInt CDunDataPusher::Stop()
    {
    FTRACE(FPrint( _L("CDunDataPusher::Stop()" )));
    if ( iPushState != EDunStateDataPushing )
        {
        FTRACE(FPrint( _L("CDunDataPusher::Stop() (not ready) complete" )));
        return KErrNotReady;
        }
    // As the EDunStateDataPushing can be on even with multiple requests,
    // cancel the actual operation in DoCancel()
    Cancel();
    iPushState = EDunStateIdle;
    FTRACE(FPrint( _L("CDunDataPusher::Stop() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops sending for write endpoint and clears event queue
// ---------------------------------------------------------------------------
//
TInt CDunDataPusher::StopAndClearQueue()
    {
    FTRACE(FPrint( _L("CDunDataPusher::StopAndClearQueue()" )));
    TInt retVal = Stop();
    iEventQueue.Reset();
    iEventIndex = 0;
    FTRACE(FPrint( _L("CDunDataPusher::StopAndClearQueue() complete" )));
    return retVal;
    }

// ---------------------------------------------------------------------------
// Signals completion status in round robin and clears event queue
// ---------------------------------------------------------------------------
//
TInt CDunDataPusher::SignalCompletionAndClearQueue()
    {
    FTRACE(FPrint( _L("CDunDataPusher::SignalCompletionAndClearQueue()" )));
    // First copy the event queue to temporary notitication queue and
    // reset the real event queue before notifications. This is done because
    // implementor of NotifyDataPushComplete() can call AddToEventQueue()
    // (and KErrAlreadyExists will happen there)
    TInt i;
    TInt retTemp;
    RPointerArray<MDunCompletionReporter> notify;
    TInt count = iEventQueue.Count();
    for ( i=0; i<count; i++ )
        {
        if ( !iEventQueue[i].iCallback )
            {
            continue;
            }
        retTemp = notify.Append( iEventQueue[i].iCallback );
        if ( retTemp != KErrNone )
            {
            FTRACE(FPrint( _L("CDunDataPusher::SignalCompletionAndClearQueue() (append failed!) complete" )));
            return retTemp;
            }
        }
    iEventQueue.Reset();
    iEventIndex = 0;
    // Now notify
    count = notify.Count();
    for ( i=0; i<count; i++ )
        {
        notify[i]->NotifyDataPushComplete( EFalse );
        }
    notify.Close();
    FTRACE(FPrint( _L("CDunDataPusher::SignalCompletionAndClearQueue() complete (%d)" ), count ));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CDunDataPusher::CDunDataPusher
// ---------------------------------------------------------------------------
//
CDunDataPusher::CDunDataPusher( CDunDownstream& aParent,
                                MDunCompletionReporter* aStreamCallback ) :
    CActive( EPriorityHigh ),
    iParent( aParent ),
    iStreamCallback( aStreamCallback )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CDunDataPusher::ConstructL
// ---------------------------------------------------------------------------
//
void CDunDataPusher::ConstructL()
    {
    FTRACE(FPrint( _L("CDunDataPusher::ConstructL()" )));
    if ( !iStreamCallback )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    FTRACE(FPrint( _L("CDunDataPusher::ConstructL() complete" )));
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CDunDataPusher::Initialize()
    {
    // Don't initialize iUtility here (it is set through NewL)
    // Don't initialize iStreamCallback here (it is set through NewL)
    iPushState = EDunStateIdle;
    iEventIndex = 0;
    iEPReady = EFalse;
    iSocket = NULL;
    iComm = NULL;
    }

// ---------------------------------------------------------------------------
// Manages one event's data push
// ---------------------------------------------------------------------------
//
TInt CDunDataPusher::ManageOneEvent()
    {
    FTRACE(FPrint( _L("CDunDataPusher::ManageOneEvent()" )));
    if ( IsActive() )
        {
        FTRACE(FPrint( _L("CDunDataPusher::ManageOneEvent() (not ready) complete" )));
        return KErrNotReady;
        }
    if ( iEventIndex < 0 ||
         iEventIndex >= iEventQueue.Count() )
        {
        FTRACE(FPrint( _L("CDunDataPusher::ManageOneEvent() (buffer mismatch) complete" )));
        return KErrGeneral;
        }
    iStatus = KRequestPending;
    if ( !iEPReady )
        {
        SetActive();
        TRequestStatus* requestStatus = &iStatus;
        User::RequestComplete( requestStatus, KErrNone );
        return KErrNone;
        }
    const TDesC8 *pushedData = iEventQueue[iEventIndex].iPushedData;
    if ( iComm )
        {
        iComm->Write( iStatus, *pushedData );
        FTRACE(FPrint( _L("CDunDataPusher::ManageOneEvent() RComm Write() requested" ) ));
        }
    else if ( iSocket )
        {
        iSocket->Send( *pushedData, 0, iStatus );
        FTRACE(FPrint( _L("CDunDataPusher::ManageOneEvent() RSocket Send() requested" ) ));
        }
    else
        {
        FTRACE(FPrint( _L("CDunDataPusher::ManageOneEvent() (ERROR) complete" )));
        return KErrGeneral;
        }
    SetActive();
    FTRACE(FPrint( _L("CDunDataPusher::ManageOneEvent() complete" )));
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Check whether an error code is severe error or not
// ---------------------------------------------------------------------------
//
TInt CDunDataPusher::ProcessErrorCondition( TInt aError, TBool& aIsError )
    {
    FTRACE(FPrint( _L("CDunDataPusher::ProcessErrorCondition() (Dir=%d)" ), EDunWriterDownstream));
    aIsError = EFalse;
    if ( aError != KErrNone )
        {
        aIsError = ETrue;
        TInt retTemp = iParent.iOkErrorsW.Find( aError );
        if ( retTemp == KErrNotFound )
            {
            FTRACE(FPrint( _L("CDunDataPusher::ProcessErrorCondition() (Dir=%d) (%d=ETrue) complete" ), EDunWriterDownstream, aError));
            return ETrue;
            }
        }
    FTRACE(FPrint( _L("CDunDataPusher::ProcessErrorCondition() (Dir=%d) (%d=EFalse) complete" ), EDunWriterDownstream, aError));
    return EFalse;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when endpoint data write complete
// ---------------------------------------------------------------------------
//
void CDunDataPusher::RunL()
    {
    FTRACE(FPrint( _L("CDunDataPusher::RunL()" )));

    TBool isError;
    TInt retTemp = iStatus.Int();
    TInt stop = ProcessErrorCondition( retTemp, isError );

    if ( !stop )  // no real error detected -> continue
        {
        if ( !isError )
            {
            iEventIndex++;
            }
        if ( iEventIndex < iEventQueue.Count() )
            {
            // More to serve so start again
            ManageOneEvent();
            }
        else
            {
            // Last was served so stop processing and notify
            iPushState = EDunStateIdle;
            iStreamCallback->NotifyDataPushComplete( ETrue );
            }
        }  // if ( !stop )
    else  // stop -> tear down connection
        {
        TDunConnectionReason connReason;
        connReason.iReasonType = EDunReasonTypeRW;
        connReason.iContext = EDunMediaContextLocal;
        connReason.iSignalType = 0;
        connReason.iSignalHigh = EFalse;
        connReason.iDirection = EDunWriterDownstream;
        connReason.iErrorCode = retTemp;
        iParent.iUtility->DoNotifyConnectionNotOk( iComm,
                                                   iSocket,
                                                   connReason,
                                                   iParent.iCallbacksW );
        }  // else

    FTRACE(FPrint( _L("CDunDataPusher::RunL() complete" )));
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CDunDataPusher::DoCancel()
    {
    FTRACE(FPrint( _L("CDunDataPusher::DoCancel()" )));
    if ( iComm )
        {
        iComm->WriteCancel();
        FTRACE(FPrint( _L("CDunDataPusher::DoCancel() (RComm) cancelled" )));
        }
    else if ( iSocket )
        {
        iSocket->CancelWrite();
        FTRACE(FPrint( _L("CDunDataPusher::DoCancel() (RSocket) cancelled" )));
        }
    FTRACE(FPrint( _L("CDunDataPusher::DoCancel() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunEndpointReady.
// Gets called when endpoint is ready
// ---------------------------------------------------------------------------
//
void CDunDataPusher::NotifyEndpointReady()
    {
    FTRACE(FPrint( _L("CDunDataPusher::NotifyEndpointReady()" )));
    iEPReady = ETrue;
    FTRACE(FPrint( _L("CDunDataPusher::NotifyEndpointReady() complete" )));
    }

// ---------------------------------------------------------------------------
// From class MDunEndpointReady.
// Gets called when endpoint is not ready
// ---------------------------------------------------------------------------
//
void CDunDataPusher::NotifyEndpointNotReady()
    {
    FTRACE(FPrint( _L("CDunDataPusher::NotifyEndpointNotReady()" )));
    iEPReady = EFalse;
    FTRACE(FPrint( _L("CDunDataPusher::NotifyEndpointNotReady() complete" )));
    }
