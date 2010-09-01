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
* Description:  LC stylustap force unmount timeouttimer
*/

#include "debug.h"
#include "forcedismounttimer.h"


const TInt KForceDismountTimeOut   = 6000000; // 6 seconds

// ======== MEMBER FUNCTIONS ========
// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CForceDismountTimer* CForceDismountTimer::NewL( MTimerNotifier* aTimeOutNotify)
    {
    TRACE_FUNC    
    CForceDismountTimer* self = CForceDismountTimer::NewLC( aTimeOutNotify );
    CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//    
CForceDismountTimer* CForceDismountTimer::NewLC( MTimerNotifier* aTimeOutNotify )
    {
    TRACE_FUNC    
    CForceDismountTimer* self = new (ELeave) CForceDismountTimer( aTimeOutNotify );
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// CTimeOutTimer()
// ---------------------------------------------------------------------------
//
CForceDismountTimer::CForceDismountTimer( MTimerNotifier* aTimeOutNotify):
    CTimer(EPriorityStandard), 
    iNotify(aTimeOutNotify)    
    {
    TRACE_FUNC    
    }    

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CForceDismountTimer::~CForceDismountTimer()
    {
    TRACE_FUNC    
    Cancel();
    }

// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CForceDismountTimer::ConstructL()
    {
    TRACE_FUNC    
    if ( !iNotify )    
        {
        User::Leave(KErrArgument);    
        }
    CTimer::ConstructL();
    CActiveScheduler::Add(this);
    After( KForceDismountTimeOut );
    }

// ---------------------------------------------------------------------------
// From class CActive
// RunL()
// ---------------------------------------------------------------------------
//
void CForceDismountTimer::RunL()
    {
    TRACE_FUNC    
    // Timer request has completed, so notify the timer's owner
    iNotify->TimerExpired();
    }
