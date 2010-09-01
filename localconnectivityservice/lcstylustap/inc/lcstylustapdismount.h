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
* Description:  Header file for Stylus Tap indicator
*
*/


#ifndef C_LCSTYLUSTAPDISMOUNT_H
#define C_LCSTYLUSTAPDISMOUNT_H

#include <e32base.h> // CActive
#include <f32file.h> 
#include "forcedismounttimer.h"


/**
 *  Active class dismount notifications notifiers 
 *
 */
NONSHARABLE_CLASS( CLcStylusTapDismount ) : public CActive, 
                                            public MTimerNotifier
    {
public:
    virtual ~CLcStylusTapDismount();
    static CLcStylusTapDismount* NewL();
    static CLcStylusTapDismount* NewLC();
    
public:
    /**
     * Send dismount notifications for all usb drives.
     */
    void DisMountUsbDrives();
    
protected:
    /**
     * Constructor
     */
    CLcStylusTapDismount();
    
    /**
     * ConstructL
     */
    void ConstructL();
    
private:
    /**
     *  Send dismount nontication
     */
    void DoDismount();

private: //from CActive    
    // CActive implementation
    /**
     * RunL
     */
    void RunL();
    
    /**
     * DoCancel
     */
    void DoCancel();
    
private: // from MTimerNotifier

    /**
     * Force dismount timer callback
     */     
    void TimerExpired();    
        
private:
    /**
     * Drive index
     */
    TInt iDriveIndex;
    /**
     * RFs session
     */
    RFs  iRFs;
    
    /**
     * List of drives
     */
    TDriveList iDriveList;    
    
    /**
     * Force dismount timer
     * Own
     */
    CForceDismountTimer* iDismountTimer;
    };



#endif      // __LCSTYLUSTAP_H__
