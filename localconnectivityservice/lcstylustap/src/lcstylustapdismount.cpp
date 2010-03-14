/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  ECOM  implementation for touch indicator
*
*/

#include "debug.h"
#include "lcstylustapdismount.h"

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CLcStylusTapDismount::~CLcStylusTapDismount()
    {
    TRACE_FUNC
    Cancel(); 
    delete iDismountTimer;    
    iRFs.Close();    
    }

// ---------------------------------------------------------------------------
// Symbian two phase constructor.
// ---------------------------------------------------------------------------
//
CLcStylusTapDismount* CLcStylusTapDismount::NewL()
    {
    TRACE_FUNC    
    CLcStylusTapDismount* self = CLcStylusTapDismount::NewLC();
    CleanupStack::Pop(self);    
    return self;
    }

// ---------------------------------------------------------------------------
// Symbian two phase constructor. Object pushed to cleanup stack 
// ---------------------------------------------------------------------------
//
CLcStylusTapDismount* CLcStylusTapDismount::NewLC()
    {
    TRACE_FUNC
    
    CLcStylusTapDismount* self = new (ELeave) CLcStylusTapDismount();
    CleanupStack::PushL(self);
    self->ConstructL();
    
    return self;

    }

// ---------------------------------------------------------------------------
// Returning of previous notifier and starting of new one 
// ---------------------------------------------------------------------------
//
void CLcStylusTapDismount::RunL()
    {
    TRACE_FUNC    
    
    delete iDismountTimer;    
    iDismountTimer = NULL;    
    
    if ( iDriveIndex < KMaxDrives )
        {
        DoDismount();
        }
    }

// ---------------------------------------------------------------------------
// Cancel pending notifier and those in queue 
// ---------------------------------------------------------------------------
//
void CLcStylusTapDismount::DoCancel()
    {
    TRACE_FUNC
    iRFs.NotifyDismountCancel(iStatus);
    }

// ---------------------------------------------------------------------------
// C++ constructor 
// ---------------------------------------------------------------------------
//
CLcStylusTapDismount::CLcStylusTapDismount()
    : CActive(EPriorityStandard)
    {
    TRACE_FUNC    
    CActiveScheduler::Add(this);    
    }

// ---------------------------------------------------------------------------
// 2nd-phase constructor 
// ---------------------------------------------------------------------------
//
void CLcStylusTapDismount::ConstructL()
    {
    TRACE_FUNC    
    User::LeaveIfError( iRFs.Connect());
    }

// ---------------------------------------------------------------------------
// Dismount drive 
// ---------------------------------------------------------------------------
//
void CLcStylusTapDismount::DisMountUsbDrives()
    {    
    TRACE_FUNC
    Cancel();
    iDriveIndex = 0;
    iRFs.DriveList( iDriveList );
    DoDismount();
    }

// ---------------------------------------------------------------------------
// Dismount next drive 
// ---------------------------------------------------------------------------
//
void CLcStylusTapDismount::DoDismount()
    {
    TRACE_FUNC        
    TDriveInfo info;
    TInt err = KErrNone;
    for ( ; iDriveIndex < KMaxDrives; iDriveIndex++ )
        {
        if ( iDriveList[iDriveIndex] )
            {
            err = iRFs.Drive( info , iDriveIndex );            
            if ( info.iConnectionBusType == EConnectionBusUsb &&                 
                 info.iDriveAtt & KDriveAttExternal && 
                 err == KErrNone  )
                {
                TRACE_INFO(_L("CLcStylusTapDismount::DoDismount Dismount notify request "));    
                iRFs.NotifyDismount( iDriveIndex, iStatus, EFsDismountNotifyClients );                
                TRAP_IGNORE( iDismountTimer = CForceDismountTimer::NewL(this) );
                SetActive();
                break;
                }                     
            }
        }
    }

// ---------------------------------------------------------------------------
// Dismount next drive 
// ---------------------------------------------------------------------------
//
void CLcStylusTapDismount::TimerExpired()
    {
    TRACE_FUNC    
    
    Cancel();
    delete iDismountTimer;
    iDismountTimer = NULL;    
    iRFs.NotifyDismount( iDriveIndex, iStatus, EFsDismountForceDismount );                
    SetActive();
    
    }    
// End of File
